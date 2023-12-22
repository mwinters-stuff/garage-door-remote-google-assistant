#ifndef _DATA_STORE_H
#define _DATA_STORE_H
#include <Arduino.h>
#include <Ticker.h>
// #include <Adafruit_MQTT_Client.h>
// #include <Adafruit_MQTT.h>
#include <AsyncMqttClient.h>
#include <ESP8266WiFi.h>
#include "ConfigFile.h"
#include "SettingsFile.h"
#include <ESP8266WiFi.h>
#include <functional>
#include <memory>



typedef std::function<void(String) > stringCallback;
typedef std::function<void() > voidCallback;

class MQTTHandler{
  public:
    MQTTHandler(std::shared_ptr<SettingsFile> settingsFile,std::shared_ptr<ConfigFile> configFile);

    void connectToMQTT();
    void onMQTTConnect(bool sessionPresent);
    void onMQTTDisconnect(AsyncMqttClientDisconnectReason reason);
    void onWifiDisconnect();
    
    void update();

    void doorPositionCommand(String data);
    void toggleLocked();
    void setLocked(bool locked);
    void setClosed(bool closed);
    void setClosing();
    void setOpening();
    void setSonicReading(int cm);

    // void sendToInflux(const String &dataPoint, const String &dataValue);
    void setTemperature(double temperature);
    
    bool isMQTTConnected(){
      return isConnected;
    }


    void stop() ;
    void publishValue(const String &topic,  const String &value);
    

    stringCallback doorCommandCallback;
    voidCallback connectedCallback;
  private:
    std::shared_ptr<SettingsFile> settingsFile;
    std::shared_ptr<ConfigFile> configFile;

    WiFiClient wifiClient;

    bool isConnected;
    AsyncMqttClient mqttClient;
    Ticker mqttReconnectTimer;
    bool dontReconnect;
    uint32_t timeConnected;
    String willTopic;
    String wifiTopic;

    // Adafruit_MQTT_Subscribe sub_door_position_command;
    // Adafruit_MQTT_Subscribe sub_door_locked_command;

    uint32_t reconnectTimeout = 0;

    void publishHomeAssistantDetect();
    void publishConfig(const String &uid, const String &kind, const String &config);
    void publishDeviceConfig(const String &uid, const String &kind, const String &name,const String &mqttStat, const String &uom, const String &deviceClass, const String &icon);
    // void publishDeviceConfig(const String &uid, const String &kind, const String &name,const String &mqttStat, const String &uom, const String &icon);
    // void publishDeviceConfig(const String &uid, const String &kind, const String &name,const String &mqttStat, const String &icon);


};



#endif