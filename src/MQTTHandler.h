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

    void doorPositionCommand(String data);
    void toggleLocked();
    void setLocked(bool locked);
    void setClosed(bool closed);
    void setSonicReading(int cm);

    // void sendToInflux(const String &dataPoint, const String &dataValue);
    void setTemperature(double temperature);
    
    bool isMQTTConnected(){
      return isConnected;
    }

    static MQTTHandler* _getInstance(){return m_instance;};
    // static MQTTHandler* init();
    stringCallback doorCommandCallback;
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
    Adafruit_MQTT_Publish pub_sonic_cm;

    Adafruit_MQTT_Subscribe sub_door_position_command;
    Adafruit_MQTT_Subscribe sub_door_locked_command;

    uint32_t reconnectTimeout = 0;
    uint32_t lastSentPing = 0;
    String last_http_reponse_str;
    static MQTTHandler* m_instance;
    // void sendInflux(const String &body);
    void connect();
    
};



#endif