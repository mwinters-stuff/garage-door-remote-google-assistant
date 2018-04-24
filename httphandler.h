#ifndef _HTTP_H
#define _HTTP_H

#include <Arduino.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ArduinoOTA.h>
#include <memory>


class HTTPHandler{
  public:
    HTTPHandler();
    void setupServer();
    void setupOTA();
    void update();

    static std::shared_ptr<HTTPHandler> get();
    static std::shared_ptr<HTTPHandler> init();

  private:
    static std::shared_ptr<HTTPHandler> m_instance;
    ESP8266WebServer httpServer;
    ESP8266HTTPUpdateServer httpUpdater;

    bool handleFileRead(String path);
    String doDoorAction(String action);
    String doLockAction(String action);
    String doInHomeArea(String action);
};



#endif
