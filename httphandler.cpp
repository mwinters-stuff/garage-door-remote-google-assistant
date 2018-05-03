#include <Arduino.h>
#include <ESP8266mDNS.h>
#include <FS.h>

#include "config.h"
#include "httphandler.h"

#include "datastore.h"
#include "iohandler.h"

std::shared_ptr<HTTPHandler> HTTPHandler::m_instance;

HTTPHandler::HTTPHandler(): httpServer(80){

}

std::shared_ptr<HTTPHandler> HTTPHandler::init(){
  m_instance = std::make_shared<HTTPHandler>();
  return m_instance;
}

std::shared_ptr<HTTPHandler> HTTPHandler::get(){
  return m_instance;
}


void HTTPHandler::update(){
  httpServer.handleClient();
  ArduinoOTA.handle();
}

bool HTTPHandler::handleFileRead(String path){
  Serial.println("handleFileRead: " + path);
  if(path.endsWith("/")) path += "index.html";
  String contentType = "text/html";
  String pathWithGz = path + ".gz";
  if(SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)){
    if(SPIFFS.exists(pathWithGz))
      path += ".gz";
    File file = SPIFFS.open(path, "r");
    httpServer.streamFile(file, contentType);
    file.close();
    return true;
  }
  return false;
}


void HTTPHandler::setupServer(){
  SPIFFS.begin();

  httpUpdater.setup(&httpServer);
  
  httpServer.on("/all", HTTP_GET, [this](){
    httpServer.sendHeader("Access-Control-Allow-Origin","*");
    std::shared_ptr<DataStore> dataStore = DataStore::get();
    String json = "{";
    json += "\"heap\": " + String(ESP.getFreeHeap());
    json += ", \"door_action\": \"" + dataStore->io_door_action_value + "\"";
    json += ", \"door_position\": \"" + dataStore->io_door_position_value + "\"";
    json += ", \"door_locked\": " + String(dataStore->is_locked ? "true" : "false");
    json += ", \"in_home_area\": " + String(dataStore->in_geofence ? "true" : "false") ;
    json += ", \"garage_temperature\": \"" + dataStore->io_garage_temperature_value + "\"";
    json += "}";
    httpServer.send(200, "text/json", json);
    json = String();
  });

  httpServer.on("/action", HTTP_GET,[this](){
    httpServer.sendHeader("Access-Control-Allow-Origin","*");
    if(httpServer.args() == 0) 
      return httpServer.send(500, "text/plain", "BAD ARGS");
    String name= httpServer.argName(0);
    String action = httpServer.arg(0);

    Serial.print(F("Action: "));
    Serial.print(name);
    Serial.print(" = ");
    Serial.println(action);
    String result = "ERROR: " + name + ": " + action + " is an unknown action";

    if(name.equals("door_action")){
      result = doDoorAction(action);
    }

    if(name.equals("lock_action")){
      result = doLockAction(action);
    }

    if(name.equals("in_home_area")){
      result = doInHomeArea(action);
    }

    httpServer.send(200, "text/plain", result);

  });

  //called when the url is not defined here
  //use it to load content from SPIFFS
  httpServer.onNotFound([this](){
    if(!handleFileRead(httpServer.uri()))
      httpServer.send(404, "text/plain", "FileNotFound");
  });

  httpServer.begin();
}

void HTTPHandler::setupOTA(){
  // setup arduino ArduinoOTA
  ArduinoOTA.setPassword(ARDUINO_OTA_PASSWORD);
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      Serial.println(F("Start updating sketch"));
    else // U_SPIFFS
      Serial.println(F("Start updating filesystem"));

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    SPIFFS.end();
  });
  ArduinoOTA.onEnd([]() {
    SPIFFS.begin();
    Serial.println(F("\nEnd"));
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf(("Progress: %u%%\r"), (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf(("Error[%u]: "), error);
    if (error == OTA_AUTH_ERROR) Serial.println(F("Auth Failed"));
    else if (error == OTA_BEGIN_ERROR) Serial.println(F("Begin Failed"));
    else if (error == OTA_CONNECT_ERROR) Serial.println(F("Connect Failed"));
    else if (error == OTA_RECEIVE_ERROR) Serial.println(F("Receive Failed"));
    else if (error == OTA_END_ERROR) Serial.println(F("End Failed"));
  });
  ArduinoOTA.begin();

  MDNS.addService("http", "tcp", 80);
  

}


String HTTPHandler::doDoorAction(String action){
  if(action.compareTo(F("OPEN")) == 0 || action.compareTo(F("CLOSE"))==0){
    DataStore::get()->io_door_action->save(action);
    IOHandler::get()->actionDoor(action);
    return "OK";
  }

  return "ERROR: " + action + " is not a door action";
}

String HTTPHandler::doLockAction(String action){
  if(action.compareTo(F("LOCK")) == 0){
    DataStore::get()->setLocked(true);
    return "OK";
  }

  if(action.compareTo(F("UNLOCK")) == 0){
    DataStore::get()->setLocked(false);
    return "OK";
  }

  return "ERROR: " + action + " is not a lock action";
}

String HTTPHandler::doInHomeArea(String action){
  return "ERROR: " + action + " is not in home action";
}

