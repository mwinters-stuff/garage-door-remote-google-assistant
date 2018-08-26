#ifndef _LOGGING_H
#define _LOGGING_H

#include <memory>

#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>
#include <ESP8266WiFi.h>

class Logging{
  public:
    Logging();

    void update();

    void log(String from, String text);

    static Logging* get();
    static Logging* init();
  private:
    static Logging* m_instance;
#ifdef MQTTLOGGING
    uint32_t lastPing;

    WiFiClient client; 
    Adafruit_MQTT_Client mqtt;
    Adafruit_MQTT_Publish telemetry;

    void mqtt_connect();
#endif    
};

#endif