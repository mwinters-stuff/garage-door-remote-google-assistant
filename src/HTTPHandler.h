#ifndef _HTTPHANDLER_H
#define _HTTPHANDLER_H

#include <Arduino.h>
#include <ESP8266WebServer.h>
#include <ArduinoOTA.h>
#include <DNSServer.h>

#include "SettingsFile.h"
#include "ConfigFile.h"
#include "IOHandler.h"

class HTTPHandler{
  public:
    HTTPHandler(IOHandler *ioHandler, SettingsFile *settingsFile, ConfigFile *configFile);
    void setupServer();
    void setupOTA();
    void update();
    void setupPortalServer();


    // static HTTPHandler* get();
    // static HTTPHandler* init();

  private:
    // static HTTPHandler* m_instance;
    ESP8266WebServer httpServer;
    IOHandler *ioHandler;
    SettingsFile *settingsFile;
    ConfigFile *configFile;
     DNSServer *dnsServer;
         const byte    DNS_PORT = 53;

    // bool handleFileRead(AsyncWebServerRequest *request);
    String doDoorCommand(const String &action);
    String doLockAction(const String &action);
    String getContentType(String filename);
    bool handleFileRead(String path);
    void setupBaseHandlers();
        void handleRedirect();
    void handleReboot();

    String formatUnknownAction(const String &what, const String &action);
};



#endif
