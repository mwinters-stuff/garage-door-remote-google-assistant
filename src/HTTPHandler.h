#ifndef _HTTPHANDLER_H
#define _HTTPHANDLER_H

#include <Arduino.h>
#include <ESP8266WebServer.h>
#include <ArduinoOTA.h>
#include "SettingsFile.h"
#include "ConfigFile.h"
#include "IOHandler.h"

class HTTPHandler{
  public:
    HTTPHandler(std::shared_ptr<IOHandler> ioHandler, std::shared_ptr<SettingsFile> settingsFile, std::shared_ptr<ConfigFile> configFile);
    void setupServer();
    void setupOTA();
    void update();

    // static HTTPHandler* get();
    // static HTTPHandler* init();

  private:
    // static HTTPHandler* m_instance;
    ESP8266WebServer httpServer;
    std::shared_ptr<IOHandler> ioHandler;
    std::shared_ptr<SettingsFile> settingsFile;
    std::shared_ptr<ConfigFile> configFile;

    // bool handleFileRead(AsyncWebServerRequest *request);
    String doDoorCommand(const String &action);
    String doLockAction(const String &action);
    String getContentType(String filename);
    bool handleFileRead(String path);

    String formatUnknownAction(const String &what, const String &action);
};



#endif
