
#ifdef ENABLE_GDB
	#include "gdbstub.h"
#endif

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <Syslog.h>

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

WiFiUDP udpClient;
Syslog* syslog = NULL;

void log_printf(PGM_P fmt_P, ...){
  va_list args;
  va_start(args, fmt_P);

  syslog->vlogf_P(LOG_INFO, fmt_P, args);
  Debug.printf_P(fmt_P, args);
  Debug.println();
  va_end(args);

}


void setup() {
  pinMode(RELAY, OUTPUT);
  digitalWrite(RELAY,LOW);

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

  if(!configFile->syslog_server.isEmpty()){
    syslog = new Syslog(udpClient, configFile->syslog_server.c_str(), configFile->syslog_port, configFile->hostname.c_str(), configFile->syslog_app_name.c_str(), LOG_INFO, SYSLOG_PROTO_BSD);
    syslog->log(LOG_INFO, F("STARTED"));
  }else{
    syslog = new Syslog(udpClient, LOG_INFO);
  }

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

const char rebootlog[] PROGMEM = "REBOOTED, Heap Size %d";
void loop() {

  Debug.handle();

  httpHandler->update();
  mqttHandler->update();
  ioHandler->update();
  if(system_get_free_heap_size() < 1000){
    syslog->logf_P(LOG_ERR, rebootlog, system_get_free_heap_size());
    ESP.restart();
  }

}