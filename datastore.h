#ifndef _DATA_H
#define _DATA_H
#include <Arduino.h>
#include <AdafruitIO_Feed.h>
#include <AdafruitIO_WiFi.h>
#include "config.h"
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

class DataStore{
  public:
    DataStore();

    doorPositions door_position;
    // doorPositions start_move_door_position;
    bool door_moving;


    std::shared_ptr<AdafruitIO_Feed> io_door_action;
    std::shared_ptr<AdafruitIO_Feed> io_door_position;
    std::shared_ptr<AdafruitIO_Feed> io_garage_temperature;

    std::shared_ptr<AdafruitIO_Feed> io_in_home_area;
    bool in_geofence;

    bool is_locked;

    String io_door_action_value;
    String io_door_position_value;
    String io_garage_temperature_value;
    
    void initFeeds(AdafruitIO_WiFi &io);
    void afterIOConnect();

    void inHomeMessage(AdafruitIO_Data *data);
    bool doorAction(AdafruitIO_Data *data);
    void updateDoorPosition(doorPositions current_door_position, doorPositions _door_position);
    void updateDoorPosition(doorPositions current_door_position, doorPositions _door_position, bool _door_moving);
    void updateTemperature(float temp);
    void toggleLocked();
    void setLocked(bool locked);
    

    static std::shared_ptr<DataStore> get();
    static std::shared_ptr<DataStore> init();
  private:
    static std::shared_ptr<DataStore> m_instance;
};



#endif