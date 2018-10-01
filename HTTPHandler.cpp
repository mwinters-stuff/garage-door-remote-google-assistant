#include <Arduino.h>
#include <ESP8266mDNS.h>
#include <FS.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncJson.h>
#include <ArduinoJson.h>
#include <TimeLib.h>
#include <NtpClientLib.h>
#include <RemoteDebug.h>

#include "ConfigFile.h"
#include "HTTPHandler.h"

#include "MQTTHandler.h"
#include "IOHandler.h"
#include "SettingsFile.h"

extern RemoteDebug Debug;

// HTTPHandler* HTTPHandler::m_instance;

HTTPHandler::HTTPHandler(IOHandler *ioHandler, SettingsFile *settingsFile, ConfigFile *configFile): httpServer(80),
  ioHandler(ioHandler), settingsFile(settingsFile), configFile(configFile){
}

// HTTPHandler* HTTPHandler::init(){
//   m_instance = new HTTPHandler();
//   return m_instance;
// }

// HTTPHandler* HTTPHandler::get(){
//   return m_instance;
// }


void HTTPHandler::update(){
  ArduinoOTA.handle();
}

void HTTPHandler::setupServer(){
  SPIFFS.begin();

  httpServer.on("/all", HTTP_GET, [this](AsyncWebServerRequest *request) {
    AsyncJsonResponse * response = new AsyncJsonResponse();
    JsonObject& root = response->getRoot();

    root[F("heap")] = ESP.getFreeHeap();
    root[F("temperature")] = settingsFile->temperature;
    root[F("door_action")] = settingsFile->last_door_action;
    root[F("door_position")] = settingsFile->current_door_position;
    root[F("door_locked")] =  settingsFile->is_locked;
    root[F("in_home_area")] = settingsFile->is_in_home_area;
    // root[F("last_http_response")] = sensors->getLastResponse();
    root[F("up_time")] = NTP.getUptimeString();
    root[F("boot_time")] = NTP.getTimeDateString(NTP.getLastBootTime());
    root[F("time_stamp")] = NTP.getTimeDateString();
    // root[F("reading_time_stamp")] = NTP.getTimeDateString(reading.last_send_time);

    root.prettyPrintTo(Debug);
    
    response->addHeader(F("Access-Control-Allow-Origin"), "*");
    response->setLength();
    request->send(response);
  
  });

  httpServer.on("/config", HTTP_GET, [this](AsyncWebServerRequest *request) {
    AsyncJsonResponse * response = new AsyncJsonResponse();
    Debug.println(F("Get Config"));
    JsonObject& root = response->getRoot();
    configFile->getJson(root);
    response->setLength();
    request->send(response);
  });

  AsyncCallbackJsonWebHandler* handler = new AsyncCallbackJsonWebHandler("/save-config", [this](AsyncWebServerRequest *request, JsonVariant &json) {
    JsonObject& root = json.as<JsonObject>();
    Debug.println(F("Save Config"));
    configFile->setJson(root);
    configFile->saveFile();

    AsyncWebServerResponse *response = request->beginResponse(200, "text/plain","OK");
    response->addHeader("Access-Control-Allow-Origin", "*");
    request->send(response);
  });
  httpServer.addHandler(handler);


  // httpServer.on("/action", HTTP_GET,[this](){
  //   httpServer.sendHeader("Access-Control-Allow-Origin","*");
  //   if(httpServer.args() == 0) 
  //     return httpServer.send(500, "text/plain", "BAD ARGS");
  //   String name= httpServer.argName(0);
  //   String action = httpServer.arg(0);

  //   Serial.print(F("Action: "));
  //   Serial.print(name);
  //   Serial.print(" = ");
  //   Serial.println(action);
  //   String result = "ERROR: " + name + ": " + action + " is an unknown action";

  //   if(name.equals("door_action")){
  //     result = doDoorAction(action);
  //   }

  //   if(name.equals("lock_action")){
  //     result = doLockAction(action);
  //   }

  //   if(name.equals("in_home_area")){
  //     result = doInHomeArea(action);
  //   }

  //   httpServer.send(200, "text/plain", result);

  // });

  httpServer.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");

  httpServer.onNotFound([this](AsyncWebServerRequest *request) {
    request->send(404, F("text/plain"), F("Not found"));
  });

  httpServer.begin();

  MDNS.addService("http", "tcp", 80);
}

void HTTPHandler::setupOTA(){
  // setup arduino ArduinoOTA
  ArduinoOTA.setPassword(ARDUINO_OTA_PASSWORD);
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      Debug.println(F("Start updating sketch"));
    else // U_SPIFFS
      Debug.println(F("Start updating filesystem"));

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    SPIFFS.end();
  });
  ArduinoOTA.onEnd([]() {
    SPIFFS.begin();
    Debug.println(F("\nEnd"));
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Debug.printf(("Progress: %u%%\r"), (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Debug.printf(("Error[%u]: "), error);
    if (error == OTA_AUTH_ERROR) Debug.println(F("Auth Failed"));
    else if (error == OTA_BEGIN_ERROR) Debug.println(F("Begin Failed"));
    else if (error == OTA_CONNECT_ERROR) Debug.println(F("Connect Failed"));
    else if (error == OTA_RECEIVE_ERROR) Debug.println(F("Receive Failed"));
    else if (error == OTA_END_ERROR) Debug.println(F("End Failed"));
  });
  ArduinoOTA.begin();

  

}


String HTTPHandler::doDoorAction(String action){
  if(action.compareTo(F("OPEN")) == 0 || action.compareTo(F("CLOSE"))==0){
    // TODO: DataStore::get()->io_door_action->save(action);
    ioHandler->actionDoor(action);
    return "OK";
  }

  return "ERROR: " + action + " is not a door action";
}

String HTTPHandler::doLockAction(String action){
  if(action.compareTo(F("LOCK")) == 0){
    // DataStore::get()->setLocked(true);
    return "OK";
  }

  if(action.compareTo(F("UNLOCK")) == 0){
    // DataStore::get()->setLocked(false);
    return "OK";
  }

  return "ERROR: " + action + " is not a lock action";
}

String HTTPHandler::doInHomeArea(String action){
  return "ERROR: " + action + " is not in home action";
}

