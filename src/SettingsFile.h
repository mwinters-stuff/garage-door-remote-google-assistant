#ifndef _SETTINGS_H
#define _SETTINGS_H
#include "JSONFileBase.h"

enum doorPositions{
  // dpStartup,
  dpUnknown,
  dpOpen,
  dpClosed,
  dpOpenToClosed,
  dpClosedToOpen,
  dpOpenRequested,
  dpCloseRequested
};

class SettingsFile: public JSONFileBase{
  public:

    SettingsFile();

    void getJson(JsonDocument & root) override;
    void setJson(const JsonDocument &object) override;
    bool updateDoorPosition(doorPositions current_door_position, doorPositions _door_position);

    static doorPositions stringToDoorPosition(const String &str);
    static String doorPositionToString(const  doorPositions position);

    doorPositions getCurrentDoorPosition() { return door_position;};
    void setCurrentDoorPosition(const doorPositions position){ this->door_position = position;};
    void setCurrentDoorPosition(const String &position){ this->door_position = stringToDoorPosition(position);};

    bool isLocked(){return is_locked;};
    void setLocked(){ is_locked = true;};
    void setUnLocked(){ is_locked = false;};
    
    bool isDoorMoving(){return is_door_moving;};
    void setDoorMoving(){ is_door_moving = true;};
    void setDoorNotMoving(){ is_door_moving = false;};

    double getTemperature(){ return temperature;};
    void setTemperature(double temperature){ this->temperature = temperature;};

    void setLastDoorAction(const String &action){ last_door_action = action;};
    String getLastDoorAction(){ return last_door_action;};

  private:
    String last_door_action;
    bool   is_locked;
    double temperature;
    doorPositions door_position;
    bool is_door_moving;

};


#endif
