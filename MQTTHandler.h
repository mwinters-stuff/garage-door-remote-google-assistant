#ifndef _DATA_STORE_H
#define _DATA_STORE_H
#include <Arduino.h>
#include <AdafruitIO_Feed.h>
#include "AdafruitIO_Custom.h"
#include "ConfigFile.h"
#include "SettingsFile.h"
#include <ESP8266WiFi.h>
#include <memory>

enum doorPositions{
  dpStartup,
  dpUnknown,
  dpOpen,
  dpClosed,
  dpOpenToClosed,
  dpClosedToOpen,
  dpManualOpenToClosed,
  dpManualClosedToOpen
};

typedef std::function<void(String) > stringCallback;

class MQTTHandler{
  public:
    MQTTHandler(SettingsFile *settingsFile, ConfigFile *configFile);
    
    doorPositions door_position;
    // doorPositions start_move_door_position;
    bool door_moving;
    AdafruitIO_Custom *io;
    AdafruitIO_Feed* io_door_action;
    AdafruitIO_Feed* io_door_position;
    AdafruitIO_Feed* io_in_home_area;

    void initFeeds();
    void connect();
    void update();

    void inHomeMessage(AdafruitIO_Data *data);
    void doorAction(AdafruitIO_Data *data);
    void updateDoorPosition(doorPositions current_door_position, doorPositions _door_position);
    void updateDoorPosition(doorPositions current_door_position, doorPositions _door_position, bool _door_moving);
    void toggleLocked();
    void setLocked(bool locked);
    

    static MQTTHandler* _getInstance(){return m_instance;};
    // static MQTTHandler* init();
    stringCallback doorActionCallback;

  private:
    SettingsFile *settingsFile;
    ConfigFile *configFile;
    static MQTTHandler* m_instance;
};



#endif