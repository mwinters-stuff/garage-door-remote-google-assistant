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
#include <WString.h>

#include "ConfigFile.h"
#include "HTTPHandler.h"

#include "MQTTHandler.h"
#include "IOHandler.h"
#include "SettingsFile.h"
#include "StringConstants.h"

extern RemoteDebug Debug;

HTTPHandler::HTTPHandler(IOHandler *ioHandler, SettingsFile *settingsFile, ConfigFile *configFile): httpServer(80),
  ioHandler(ioHandler), settingsFile(settingsFile), configFile(configFile){
}

void HTTPHandler::update(){
  ArduinoOTA.handle();
}

void HTTPHandler::setupServer(){
  SPIFFS.begin();

  httpServer.on(String(F("/all")).c_str(), HTTP_GET, [this](AsyncWebServerRequest *request) {
    AsyncJsonResponse * response = new AsyncJsonResponse();
    JsonObject& root = response->getRoot();

    root[HEAP] = ESP.getFreeHeap();
    root[TEMPERATURE] = settingsFile->getTemperature();
    root[DOOR_ACTION] = settingsFile->getLastDoorAction();
    root[DOOR_POSITION] = SettingsFile::doorPositionToString(settingsFile->getCurrentDoorPosition());
    root[DOOR_LOCKED] =  settingsFile->isLocked();
    root[IN_HOME_AREA] = settingsFile->isInHomeArea();
    // root[F("last_http_response")] = sensors->getLastResponse();
    root[UP_TIME] = NTP.getUptimeString();
    root[BOOT_TIME] = NTP.getTimeDateString(NTP.getLastBootTime());
    root[TIME_STAMP] = NTP.getTimeDateString();
    // root[F("reading_time_stamp")] = NTP.getTimeDateString(reading.last_send_time);

    // root.prettyPrintTo(Debug);
    // Debug.println();
    
    response->addHeader(ACCESS_CONTROL_HEADER, "*");
    response->setLength();
    request->send(response);
  
  });

  httpServer.on(String(F("/config")).c_str(), HTTP_GET, [this](AsyncWebServerRequest *request) {
    AsyncJsonResponse * response = new AsyncJsonResponse();
    Debug.println(F("Get Config"));
    JsonObject& root = response->getRoot();
    configFile->getJson(root);

    root.prettyPrintTo(Debug);
    Debug.println();

    response->setLength();
    request->send(response);
  });

  httpServer.on(String(F("/settings")).c_str(), HTTP_GET, [this](AsyncWebServerRequest *request) {
    AsyncJsonResponse * response = new AsyncJsonResponse();
    Debug.println(F("Get Settings"));
    
    JsonObject& root = response->getRoot();
    settingsFile->getJson(root);

    root.prettyPrintTo(Debug);
    Debug.println();

    response->setLength();
    request->send(response);
  });

  AsyncCallbackJsonWebHandler* handler = new AsyncCallbackJsonWebHandler(F("/save-config"), [this](AsyncWebServerRequest *request, JsonVariant &json) {
    JsonObject& root = json.as<JsonObject>();
    Debug.println(F("Save Config"));

    root.prettyPrintTo(Debug);
    Debug.println();

    configFile->setJson(root);
    configFile->saveFile();

    AsyncWebServerResponse *response = request->beginResponse(200, TEXT_PLAIN,OKRESULT);
    response->addHeader(ACCESS_CONTROL_HEADER, "*");
    request->send(response);
  });
  httpServer.addHandler(handler);

  httpServer.on(String(F("/action")).c_str(), HTTP_GET, [this](AsyncWebServerRequest *request){
    if(request->args() == 0){
      request->send(500,TEXT_PLAIN,INVALID_ARGUMENTS);
      return;
    }
    
    String name = request->argName(0);
    String action = request->arg((size_t)0);

    Debug.printf_P(PSTR("Action: %s = %s "), name.c_str(), action.c_str());

    String result = formatUnknownAction(name, action);

    if(name.equals(DOOR_ACTION)){
      result = doDoorAction(action);
    }

    if(name.equals(LOCK_ACTION)){
      result = doLockAction(action);
    }

    if(name.equals(IN_HOME_AREA)){
      result = doInHomeArea(action);
    }

    AsyncWebServerResponse *response = request->beginResponse(200,TEXT_PLAIN, result);
    
    response->addHeader(ACCESS_CONTROL_HEADER, "*");

    request->send(response);
  });

  httpServer.serveStatic("/", SPIFFS, "/").setDefaultFile(String(F("index.html")).c_str());

  httpServer.onNotFound([this](AsyncWebServerRequest *request) {
    request->send(404, TEXT_PLAIN, F("Not found"));
  });

  httpServer.begin();

  MDNS.addService("http", "tcp", 80);
}

void HTTPHandler::setupOTA(){
  // setup arduino ArduinoOTA
  ArduinoOTA.setPassword(ARDUINO_OTA_PASSWORD);
  ArduinoOTA.onStart([]() {
    String type;
    Debug.print(F("Start updating "));
    if (ArduinoOTA.getCommand() == U_FLASH)
      Debug.println(F("sketch"));
    else // U_SPIFFS
      Debug.println(F("filesystem"));

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    SPIFFS.end();
  });
  ArduinoOTA.onEnd([]() {
    SPIFFS.begin();
    Debug.println(F("\nEnd"));
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Debug.printf_P(PSTR("Progress: %u%%\r"), (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    String error_s;
    switch(error){
      case OTA_AUTH_ERROR:
        error_s = F("Auth");
        break;
      case OTA_BEGIN_ERROR:
        error_s = F("Begin");
        break;
      case OTA_CONNECT_ERROR:
        error_s = F("Connect");
        break;
      case OTA_RECEIVE_ERROR:
        error_s = F("Receive");
        break;
      case OTA_END_ERROR:
        error_s = F("End");
        break;
    }
    Debug.printf(String(ERROR_FAILED).c_str(), error, error_s.c_str());
  });
  ArduinoOTA.begin();

}


String HTTPHandler::doDoorAction(const String &action){
  if(action.compareTo(OPEN) == 0 || action.compareTo(CLOSE)==0){
    // TODO: DataStore::get()->io_door_action->save(action);
    ioHandler->setDoorAction(action);
    return OKRESULT;
  }

  return formatUnknownAction(DOOR_ACTION, action);
}

String HTTPHandler::doLockAction(const String &action){
  if(action.compareTo(LOCK) == 0){
    settingsFile->setLocked();
    return OKRESULT;
  }

  if(action.compareTo(UNLOCK) == 0){
    settingsFile->setUnLocked();
    return OKRESULT;
  }

  return formatUnknownAction(LOCK_ACTION, action);
}

String HTTPHandler::doInHomeArea(const String &action){
    if(action.compareTo(YES) == 0){
    settingsFile->setInHomeArea();
    return OKRESULT;
  }

  if(action.compareTo(NO) == 0){
    settingsFile->setOutHomeArea();
    return OKRESULT;
  }

  return formatUnknownAction(IN_HOME_AREA, action);
}

String HTTPHandler::formatUnknownAction(const String &what, const String &action){
  size_t len = 40 + what.length() + action.length();
  char buffer[len];
  snprintf_P(buffer, len, _UNKNOWN_ACTION, what.c_str(), action.c_str());
  return String(buffer);
}

