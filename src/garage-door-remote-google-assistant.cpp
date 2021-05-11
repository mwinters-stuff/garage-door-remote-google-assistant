

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <DNSServer.h>
#include <WiFiUdp.h>
#include <LittleFS.h>

#include <TimeLib.h> 
#include <NtpClientLib.h>

#include "ConfigFile.h"
#include "SettingsFile.h"
#include "MQTTHandler.h"
#include "HTTPHandler.h"
#include "IOHandler.h"
#include "HardwareConfig.h"

#include <user_interface.h>

SettingsFile *settingsFile;
ConfigFile *configFile;
MQTTHandler* mqttHandler;

IOHandler* ioHandler;
HTTPHandler* httpHandler;

#include <RemoteDebug.h>

RemoteDebug Debug;
uint32_t start_delay;


void log_printf(PGM_P fmt_P, ...){
  va_list args;
  va_start(args, fmt_P);

  Debug.printf_P(fmt_P, args);
  Debug.println();
  va_end(args);

}


bool apMode = false;

void setupAP() {
  apMode = true;
  Serial.println(F("Setting soft-AP ... "));
  String apName = String("ESP_") + String(ESP.getChipId());
  boolean result = WiFi.softAP((apName).c_str());
  if (result == true) {
    Debug.begin(apName);
    Debug.setResetCmdEnabled(true);
    Debug.setSerialEnabled(true);

    Debug.println(F("Ready Setting Up Portal"));
        httpHandler = new HTTPHandler(NULL, settingsFile, configFile);

    httpHandler->setupPortalServer();
    IOHandler::ledRed(true);

  } else {
    Debug.begin(apName);
    Debug.setResetCmdEnabled(true);
    Debug.setSerialEnabled(true);
    Debug.println("Failed!");
  }
}

void setupWIFI(){
  Serial.print(F("Connecting to Wifi"));
  WiFi.hostname(configFile->hostname);
  WiFi.setAutoReconnect(true);
  WiFi.mode(WIFI_STA);
  WiFi.begin(configFile->wifi_ap.c_str(), configFile->wifi_password.c_str());

  uint8_t count = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    IOHandler::ledRedToggle();
    delay(500);
    Serial.print(".");
    if(++count == 20){
      setupAP();
      return;
    }
  }    

  Serial.print(F("Connected, IP address: "));
  Serial.println(WiFi.localIP());  
  Serial.println();
  MDNS.begin(configFile->hostname.c_str());
  Debug.begin(configFile->hostname);
  Debug.setResetCmdEnabled(true);
  Debug.setSerialEnabled(true); 

  NTP.begin(configFile->ntp_server); 

  settingsFile->init();

  mqttHandler = new MQTTHandler(settingsFile, configFile);
  
  ioHandler = new IOHandler(mqttHandler,settingsFile, configFile);

  httpHandler = new HTTPHandler(ioHandler, settingsFile, configFile);
  httpHandler->setupServer();
  httpHandler->setupOTA();
  ioHandler->init();
}

void setup() {

  pinMode(RELAY, OUTPUT);
  digitalWrite(RELAY,LOW);
  IOHandler::ledRed(false);
  IOHandler::ledGreen(false);

  LittleFS.begin();

  Serial.begin(115200);
  for(int i = 0; i < 10; i++){
    Serial.println();
  }
  settingsFile = new SettingsFile();
  configFile = new ConfigFile();

  if(!configFile->init()){
    Serial.println(F("Starting Access Point"));
    setupAP();
  }else{
    setupWIFI();
  }
  

}

const char rebootlog[] PROGMEM = "REBOOTED, Heap Size %d";
void loop() {

  Debug.handle();

  httpHandler->update();
  if(mqttHandler && ioHandler){
    mqttHandler->update();
    ioHandler->update();
  }
  if(system_get_free_heap_size() < 1000){
    ESP.restart();
  }

}