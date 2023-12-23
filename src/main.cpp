

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <LittleFS.h>

#include <TimeLib.h> 


#include "ConfigFile.h"
#include "SettingsFile.h"
#include "MQTTHandler.h"
#include "HTTPHandler.h"
#include "IOHandler.h"
#include "HardwareConfig.h"

#include <user_interface.h>

std::shared_ptr<SettingsFile> settingsFile;
std::shared_ptr<ConfigFile> configFile;
std::shared_ptr<MQTTHandler> mqttHandler;

std::shared_ptr<IOHandler> ioHandler;
std::shared_ptr<HTTPHandler> httpHandler;

#include <RemoteDebug.h>

RemoteDebug Debug;
uint32_t start_delay;

WiFiUDP udpClient;

bool unconfigured = false;
Ticker flashTimer;
WiFiEventHandler wifiConnectHandler;
WiFiEventHandler wifiDisconnectHandler;
Ticker wifiReconnectTimer;
bool redLedOn = false;

#define FLASH_LED_CONNECTING_DELAY 250
#define FLASH_LED_UNCONFIGURED_DELAY 2000


void log_printf(PGM_P fmt_P, ...){
  va_list args;
  va_start(args, fmt_P);

  Debug.printf_P(fmt_P, args);
  Debug.println();
  va_end(args);

}

void toggleLED(){
  digitalWrite(LED_RED, redLedOn ? HIGH : LOW);
  redLedOn = !redLedOn;
}

void doSmartWifiConfig(){
  WiFi.beginSmartConfig();
  Serial.println(F("Doing SmartConfig"));
  unconfigured = true;
  flashTimer.detach();
  flashTimer.attach_ms_scheduled(FLASH_LED_UNCONFIGURED_DELAY, [](){ toggleLED(); });

  // forever loop: exit only when SmartConfig packets have been received
  while (true)
  {
    delay(500);
    Serial.print(".");
    if (WiFi.smartConfigDone())
    {
      configFile->hostname = "garage-door-unconfigured";
      configFile->update_interval = "60";
      Serial.println(F("SmartConfig successfully configured"));
      break; // exit from loop
    }
  }
}


void  connectToWifi(){
  Serial.println(F("\n\nConnecting to Wifi"));
  WiFi.begin(configFile->wifi_ap.c_str(), configFile->wifi_password.c_str());
  flashTimer.detach();
  flashTimer.attach_ms_scheduled(unconfigured ? FLASH_LED_UNCONFIGURED_DELAY : FLASH_LED_CONNECTING_DELAY, [](){ toggleLED(); });
}

void onWifiConnect(const WiFiEventStationModeGotIP& event){
  Serial.print(F("Connected, IP address: "));
  Serial.println(WiFi.localIP());

  Debug.begin(configFile->hostname);
  Debug.setResetCmdEnabled(true);
  Debug.setSerialEnabled(true); 

  MDNS.begin(configFile->hostname.c_str());

  mqttHandler.reset();
  mqttHandler = std::make_shared<MQTTHandler>(settingsFile, configFile);
  mqttHandler->connectToMQTT();

  ioHandler.reset();
  ioHandler = std::make_shared<IOHandler>(mqttHandler,settingsFile, configFile);
  ioHandler->init();
  ioHandler->ledRed(true);
  ioHandler->ledGreen(true);
  ioHandler->setupTimers();

  httpHandler.reset();
  httpHandler = std::make_shared<HTTPHandler>(ioHandler, settingsFile, configFile);
  httpHandler->setupServer();
  httpHandler->setupOTA();

  ioHandler->ledRed(false);
  ioHandler->ledGreen(false);


  flashTimer.detach();

}

void onWifiDisconnect(const WiFiEventStationModeDisconnected& event){
  flashTimer.detach();
  digitalWrite(LED_RED, LOW);

  Serial.println("Disconnected from Wi-Fi.");

  MDNS.close();
  Debug.stop();
  // NTP.stop();
  mqttHandler.reset();

  if(event.reason == WIFI_DISCONNECT_REASON_NO_AP_FOUND || event.reason == WIFI_DISCONNECT_REASON_AUTH_FAIL){
    doSmartWifiConfig();
  }else{
    wifiReconnectTimer.once(2, connectToWifi);
  }
}


void setup() {
  pinMode(RELAY, OUTPUT);
  digitalWrite(RELAY,LOW);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  LittleFS.begin();

  Serial.begin(115200);
  for(int i = 0; i < 10; i++){
    Serial.println();
  }
  settingsFile = std::make_shared<SettingsFile>();
  configFile = std::make_shared<ConfigFile>();


  WiFi.mode(WIFI_STA);
  wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
  wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);

  if(configFile->wifi_ap.isEmpty()){
    Serial.println(F("Failed to read config.json"));

    doSmartWifiConfig();
  }else{
   connectToWifi();  
  }

 
}

void loop() {
  Debug.handle();
  if(httpHandler && mqttHandler && ioHandler)
  {
    httpHandler->update();
    mqttHandler->update();
    ioHandler->update();
  }
  if(system_get_free_heap_size() < 1000){
   ESP.restart();
  }

}