

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

void log_printf(PGM_P fmt_P, ...){
  va_list args;
  va_start(args, fmt_P);

  Debug.printf_P(fmt_P, args);
  Debug.println();
  va_end(args);

}


void setup() {
  pinMode(RELAY, OUTPUT);
  digitalWrite(RELAY,LOW);
  LittleFS.begin();

  Serial.begin(115200);
  for(int i = 0; i < 10; i++){
    Serial.println();
  }
  settingsFile = std::make_shared<SettingsFile>();
  configFile = std::make_shared<ConfigFile>();

  // trackMem("setup start");

  Serial.print(F("Connecting to Wifi"));
  WiFi.hostname(configFile->hostname);
  WiFi.setAutoReconnect(true);
  WiFi.mode(WIFI_STA);
  WiFi.begin(configFile->wifi_ap.c_str(), configFile->wifi_password.c_str());
  
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.print(F("Connected, IP address: "));
  Serial.println(WiFi.localIP());

  MDNS.begin(configFile->hostname.c_str());
  Debug.begin(configFile->hostname);
  Debug.setResetCmdEnabled(true);
  Debug.setSerialEnabled(true); 
	// NTP.begin(configFile->ntp_server); 

  mqttHandler = std::make_shared<MQTTHandler>(settingsFile, configFile);
  mqttHandler->connectToMQTT();
  
  ioHandler = std::make_shared<IOHandler>(mqttHandler,settingsFile, configFile);
  ioHandler->ledRed(true);
  ioHandler->ledGreen(true);

  httpHandler = std::make_shared<HTTPHandler>(ioHandler, settingsFile, configFile);
  httpHandler->setupServer();
  httpHandler->setupOTA();

  ioHandler->ledRed(false);
  ioHandler->ledGreen(false);
 
}

const char rebootlog[] PROGMEM = "REBOOTED, Heap Size %d";
void loop() {

  Debug.handle();

  httpHandler->update();
  mqttHandler->update();
  ioHandler->update();
  if(system_get_free_heap_size() < 1000){
   ESP.restart();
  }

}