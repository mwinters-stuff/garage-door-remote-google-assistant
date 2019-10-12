#include <Arduino.h>
#include <ESP8266mDNS.h>
#include <FS.h>
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
  httpServer.handleClient();
  ArduinoOTA.handle();
}

void HTTPHandler::setupServer(){
  SPIFFS.begin();

  httpServer.on(F("/all"), HTTP_GET, [this]() {
    DynamicJsonDocument root(1024);

    root[HEAP] = ESP.getFreeHeap();
    root[TEMPERATURE] = settingsFile->getTemperature();
    root[DOOR_ACTION] = settingsFile->getLastDoorAction();
    root[DOOR_POSITION] = SettingsFile::doorPositionToString(settingsFile->getCurrentDoorPosition());
    root[DOOR_LOCKED] =  settingsFile->isLocked();
    root[UP_TIME] = NTP.getUptimeString();
    root[BOOT_TIME] = NTP.getTimeDateString(NTP.getLastBootTime());
    root[TIME_STAMP] = NTP.getTimeDateString();

    serializeJsonPretty(root, Debug);
    Debug.println();
    String response;
    serializeJson(root, response);

    
    httpServer.sendHeader(ACCESS_CONTROL_HEADER, "*");
    httpServer.send(200,F("application/json"),response);
    
  });

  httpServer.on(String(F("/config")).c_str(), HTTP_GET, [this]() {
    DynamicJsonDocument root(1024);
    Debug.println(F("Get Config"));
    
    configFile->getJson(root);

    serializeJsonPretty(root, Debug);
    Debug.println();
    String response;
    serializeJson(root, response);

    httpServer.sendHeader(ACCESS_CONTROL_HEADER, "*");
    httpServer.send(200,F("application/json"),response);

  });

  httpServer.on(String(F("/settings")).c_str(), HTTP_GET, [this]() {
    DynamicJsonDocument root(1024);

    Debug.println(F("Get Settings"));
    
    settingsFile->getJson(root);

    serializeJsonPretty(root, Debug);
    Debug.println();
    String response;
    serializeJson(root, response);

    httpServer.sendHeader(ACCESS_CONTROL_HEADER, "*");
    httpServer.send(200,F("application/json"),response);

  });

  httpServer.on(String(F("/save-config")).c_str(), HTTP_POST, [this]() {
    if(httpServer.hasArg(F("plain"))){
      DynamicJsonDocument root(1024);
      auto error = deserializeJson(root, httpServer.arg(F("plain")));
      if(!error){

        Debug.println(F("Save Config"));

        serializeJsonPretty(root, Debug);
        Debug.println();

        configFile->setJson(root);
        configFile->saveFile();

        httpServer.sendHeader(ACCESS_CONTROL_HEADER, "*");
        httpServer.send(200,TEXT_PLAIN,OKRESULT);
        return;
      }else{
       Debug.print(F("deserializeJson() failed with code "));
       Debug.println(error.c_str());
      }
    }
    httpServer.sendHeader(ACCESS_CONTROL_HEADER, "*");
    httpServer.send(500,TEXT_PLAIN,INVALID_ARGUMENTS);
   
  });

  httpServer.on(String(F("/action")).c_str(), HTTP_GET, [this](){
    if(httpServer.args() == 0){
      httpServer.send(500,TEXT_PLAIN,INVALID_ARGUMENTS);
      return;
    }
    
    String name = httpServer.argName(0);
    String action = httpServer.arg(0);

    Debug.printf_P(PSTR("Action: %s = %s "), name.c_str(), action.c_str());

    String result = formatUnknownAction(name, action);

    if(name.equals(DOOR_ACTION)){
      result = doDoorAction(action);
    }

    if(name.equals(LOCK_ACTION)){
      result = doLockAction(action);
    }

    httpServer.sendHeader(ACCESS_CONTROL_HEADER, "*");
    httpServer.send(200,TEXT_PLAIN,result);
  });

  

  httpServer.onNotFound([this]() {
    if (!handleFileRead(httpServer.uri())) {
      httpServer.send(404, TEXT_PLAIN, F("Not found"));
    }
  });

  httpServer.begin();

  MDNS.addService("http", "tcp", 80);
}

String HTTPHandler::getContentType(String filename){
  if(filename.endsWith(".html")) return "text/html";
  else if(filename.endsWith(".css")) return "text/css";
  else if(filename.endsWith(".js")) return "application/javascript";
  else if(filename.endsWith(".ico")) return "image/x-icon";
  else if(filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

bool HTTPHandler::handleFileRead(String path){  // send the right file to the client (if it exists)
  Serial.println("handleFileRead: " + path);
  if(path.endsWith("/")) {
    path += "index.html";           // If a folder is requested, send the index file
  }
  String contentType = getContentType(path);             // Get the MIME type
  String pathWithGz = path + ".gz";
  if(SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)){  // If the file exists, either as a compressed archive, or normal
    if(SPIFFS.exists(pathWithGz)){                          // If there's a compressed version available
      path += ".gz";                                         // Use the compressed version
    }
    File file = SPIFFS.open(path, "r");                    // Open the file
    httpServer.streamFile(file, contentType);    // Send it to the client
    file.close();                                          // Close the file again
    Serial.println(String("\tSent file: ") + path);
    return true;
  }
  Serial.println(String("\tFile Not Found: ") + path);
  return false;                                          // If the file doesn't exist, return false
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


String HTTPHandler::formatUnknownAction(const String &what, const String &action){
  size_t len = 40 + what.length() + action.length();
  char buffer[len];
  snprintf_P(buffer, len, _UNKNOWN_ACTION, what.c_str(), action.c_str());
  return String(buffer);
}

