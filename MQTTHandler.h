#ifndef _DATA_STORE_H
#define _DATA_STORE_H
#include <Arduino.h>
#include <AdafruitIO_Feed.h>
#include "AdafruitIO_Custom.h"
#include "ConfigFile.h"
#include "SettingsFile.h"
#include <ESP8266WiFi.h>
#include <memory>



typedef std::function<void(String) > stringCallback;

class MQTTHandler{
  public:
    MQTTHandler(SettingsFile *settingsFile, ConfigFile *configFile);
    
    // doorPositions start_move_door_position;
    AdafruitIO_Custom *io;
    AdafruitIO_Feed* io_door_action;
    AdafruitIO_Feed* io_door_position;
    AdafruitIO_Feed* io_in_home_area;
    AdafruitIO_Feed* io_temperature;

    void initFeeds();
    void connect();
    void update();

    void inHomeMessage(AdafruitIO_Data *data);
    void doorAction(AdafruitIO_Data *data);
    void updateDoorPosition(doorPositions current_door_position, doorPositions _door_position);
    void updateDoorPosition(doorPositions current_door_position, doorPositions _door_position, bool _door_moving);
    void toggleLocked();
    void setLocked(bool locked);
    void sendToInflux(const String &dataPoint, const String &dataValue);
    void setTemperature(double temperature);
    

    static MQTTHandler* _getInstance(){return m_instance;};
    // static MQTTHandler* init();
    stringCallback doorActionCallback;
    String getLastHTTPResponseString(){ return last_http_reponse_str;};
  private:
    SettingsFile *settingsFile;
    ConfigFile *configFile;
    String last_http_reponse_str;
    static MQTTHandler* m_instance;
    void sendInflux(const String &body);
};



#endif