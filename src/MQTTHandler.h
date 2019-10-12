#ifndef _DATA_STORE_H
#define _DATA_STORE_H
#include <Arduino.h>
#include <Adafruit_MQTT_Client.h>
#include <Adafruit_MQTT.h>
#include <ESP8266WiFi.h>
#include "ConfigFile.h"
#include "SettingsFile.h"
#include <ESP8266WiFi.h>
#include <memory>



typedef std::function<void(String) > stringCallback;

class MQTTHandler{
  public:
    MQTTHandler(SettingsFile *settingsFile, ConfigFile *configFile);
    
    void update();

    void doorAction(String data);
    void updateDoorPosition(doorPositions current_door_position, doorPositions _door_position);
    void updateDoorPosition(doorPositions current_door_position, doorPositions _door_position, bool _door_moving);
    void toggleLocked();
    void setLocked(bool locked);
    // void sendToInflux(const String &dataPoint, const String &dataValue);
    void setTemperature(double temperature);
    
    bool isMQTTConnected(){
      return isConnected;
    }

    static MQTTHandler* _getInstance(){return m_instance;};
    // static MQTTHandler* init();
    stringCallback doorActionCallback;
    String getLastHTTPResponseString(){ return last_http_reponse_str;};
  private:
    SettingsFile *settingsFile;
    ConfigFile *configFile;
    WiFiClient wifiClient;
    bool isConnected;
    Adafruit_MQTT_Client mqtt;

    Adafruit_MQTT_Publish pub_door_position;
    Adafruit_MQTT_Publish pub_door_locked;
    Adafruit_MQTT_Publish pub_online;
    Adafruit_MQTT_Publish pub_temperature;

    Adafruit_MQTT_Subscribe sub_door_position;
    Adafruit_MQTT_Subscribe sub_door_locked;

    uint32_t reconnectTimeout = 0;
    uint32_t lastSentPing = 0;
    String last_http_reponse_str;
    static MQTTHandler* m_instance;
    // void sendInflux(const String &body);
    void connect();
    
};



#endif