#include <Arduino.h>
#include <ESP8266mDNS.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <TimeLib.h>
#include <NtpClientLib.h>
#include <RemoteDebug.h>
#include <WString.h>

#include "ConfigFile.h"
#include "HTTPHandler.h"

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
  if (dnsServer) {
    dnsServer->processNextRequest();
  }
}

void HTTPHandler::setupServer(){
  setupBaseHandlers();
}

void HTTPHandler::setupBaseHandlers(){
  httpServer.on(F("/all"), HTTP_GET, [this]() {
    DynamicJsonDocument root(1024);

    root[HEAP] = ESP.getFreeHeap();
    if(ioHandler){
      root[TEMPERATURE] = settingsFile->getTemperature();
      root[DOOR_CLOSED] = settingsFile->isClosed();
      root[DOOR_LOCKED] =  settingsFile->isLocked();
      root[SONIC_DISTANCE] = ioHandler->sonicLastDistance();
      root[UP_TIME] = NTP.getUptimeString();
      root[BOOT_TIME] = NTP.getTimeDateString(NTP.getLastBootTime());
      root[TIME_STAMP] = NTP.getTimeDateString();
    }

    // serializeJsonPretty(root, Debug);
    // Debug.println();
    String response;
    serializeJson(root, response);

    
    httpServer.sendHeader(ACCESS_CONTROL_HEADER, "*");
    httpServer.send(200,F("application/json"),response);
    
  });

  httpServer.on(String(F("/reboot")).c_str(), HTTP_GET, std::bind(&HTTPHandler::handleReboot, this));

  httpServer.on(String(F("/config")).c_str(), HTTP_GET, [this]() {
    DynamicJsonDocument root(1024);
    Debug.println(F("Get Config"));
    
    configFile->getJson(root);

    // serializeJsonPretty(root, Debug);
    // Debug.println();
    String response;
    serializeJson(root, response);

    httpServer.sendHeader(ACCESS_CONTROL_HEADER, "*");
    httpServer.send(200,F("application/json"),response);

  });

  httpServer.on(String(F("/settings")).c_str(), HTTP_GET, [this]() {
    DynamicJsonDocument root(1024);

    Debug.println(F("Get Settings"));
    
    settingsFile->getJson(root);

    // serializeJsonPretty(root, Debug);
    // Debug.println();
    String response;
    serializeJson(root, response);

    httpServer.sendHeader(ACCESS_CONTROL_HEADER, "*");
    httpServer.send(200,F("application/json"),response);

  });

  httpServer.on(String(F("/save-config")).c_str(), HTTP_POST, [this]() {
    if(httpServer.hasArg(F("plain"))){
      DynamicJsonDocument root(1500);
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

  if(ioHandler){
    httpServer.on(String(F("/command")).c_str(), HTTP_GET, [this](){
      if(httpServer.args() == 0){
        httpServer.send(500,TEXT_PLAIN,INVALID_ARGUMENTS);
        return;
      }
      
      String name = httpServer.argName(0);
      String action = httpServer.arg(0);

      Debug.printf_P(PSTR("Action: %s = %s \r\n"), name.c_str(), action.c_str());

      String result = formatUnknownAction(name, action);

      if(name.equals(DOOR_COMMAND)){
        result = doDoorCommand(action);
      }

      if(name.equals(LOCK_ACTION)){
        result = doLockAction(action);
      }

      httpServer.sendHeader(ACCESS_CONTROL_HEADER, "*");
      httpServer.send(200,TEXT_PLAIN,result);
    });
  }

  

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
  if(LittleFS.exists(pathWithGz) || LittleFS.exists(path)){  // If the file exists, either as a compressed archive, or normal
    if(LittleFS.exists(pathWithGz)){                          // If there's a compressed version available
      path += ".gz";                                         // Use the compressed version
    }
    File file = LittleFS.open(path, "r");                    // Open the file
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
    LittleFS.end();
  });
  ArduinoOTA.onEnd([]() {
    LittleFS.begin();
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


void HTTPHandler::setupPortalServer() {
  dnsServer = new DNSServer();
  delay(500);  // Without delay I've seen the IP address blank
  Debug.print(F("AP IP address: "));
  Debug.println(WiFi.softAPIP());

  /* Setup the DNS server redirecting all the domains to the apIP */
  dnsServer->setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer->start(DNS_PORT, "*", WiFi.softAPIP());

  httpServer.on(F("/generate_204"), std::bind(&HTTPHandler::handleRedirect, this));
  httpServer.on(F("/gen_204"), std::bind(&HTTPHandler::handleRedirect, this));

  setupBaseHandlers();
}

void HTTPHandler::handleRedirect() {
  String redirect = String(F("http://")) + WiFi.softAPIP().toString() + String("/");
  Debug.print(F("handleRedirect "));
  Debug.println(redirect);
  httpServer.sendHeader(F("Location"), redirect);
  httpServer.send(302, TEXT_PLAIN, INVALID_ARGUMENTS);
  httpServer.client().stop();
}

void HTTPHandler::handleReboot() {
  httpServer.send(200, TEXT_PLAIN, F("Rebooting soon"));

  Debug.println(F("Rebooting"));

  delay(5000);
  ESP.reset();
  delay(2000);
}

String HTTPHandler::doDoorCommand(const String &command){
  if(command.compareTo(OPEN) == 0 || command.compareTo(CLOSE) == 0 || command.compareTo(FORCE) == 0){
    // TODO: DataStore::get()->io_door_action->save(action);
    ioHandler->doorCommand(command);
    return OKRESULT;
  }

  return formatUnknownAction(DOOR_COMMAND, command);
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

