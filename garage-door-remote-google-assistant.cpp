
#ifdef ENABLE_GDB
	#include "gdbstub.h"
#endif

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>


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

void setup() {
  #ifdef ENABLE_GDB
	  gdbstub_init();
  #endif
  Serial.begin(115200);
  for(int i = 0; i < 10; i++){
    Serial.println();
  }
  settingsFile = new SettingsFile();
  configFile = new ConfigFile();

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
	NTP.begin(configFile->ntp_server); 

  mqttHandler = new MQTTHandler(settingsFile, configFile);
  
  ioHandler = new IOHandler(mqttHandler,settingsFile, configFile);
  ioHandler->ledRed(true);
  ioHandler->ledGreen(true);

  httpHandler = new HTTPHandler(ioHandler, settingsFile, configFile);
  httpHandler->setupServer();
  httpHandler->setupOTA();

  ioHandler->ledRed(false);
  ioHandler->ledGreen(false);

  // mqttHandler->initFeeds();


}

void loop() {

  Debug.handle();

  httpHandler->update();
  mqttHandler->update();
  ioHandler->update();

  if(system_get_free_heap_size() < 1000){
    ESP.restart();
  }

}