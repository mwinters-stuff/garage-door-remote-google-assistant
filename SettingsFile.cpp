#include "SettingsFile.h"
#include <map>
#include <RemoteDebug.h>
extern RemoteDebug Debug;
//#define Debug Serial
#include "StringConstants.h"

#ifdef TEST
#define SETTINGS_FILE "/settings-test.json"
#else
#define SETTINGS_FILE "/settings.json"
#endif

// SettingsFile *SettingsFile::m_instance;
static std::map<doorPositions, String> door_position_strings = {
    { dpStartup, String(STARTUP)}, 
    { dpUnknown, String(UNKNOWN)}, 
    { dpOpen, String(OPEN)}, 
    { dpClosed, String(CLOSED)}, 
    { dpOpenToClosed, String(OPEN_CLOSED)}, 
    { dpClosedToOpen, String(CLOSED_OPEN)}, 
    { dpManualOpenToClosed, String(MANUAL_OPEN_CLOSED)}, 
    { dpManualClosedToOpen, String(MANUAL_CLOSED_OPEN)}
};

SettingsFile::SettingsFile():JSONFileBase(SETTINGS_FILE){
  if(!readFile()){
    Serial.printf(String(FAILED_TO_READ).c_str(), fileName.c_str());
    last_door_action = UNKNOWN;
    is_locked = false;
    is_in_home_area = true;
    door_position = dpStartup;
    is_door_moving = false;
  }
}

void SettingsFile::getJson(JsonObject & root){

  root[CURRENT_DOOR_POSITION] = doorPositionToString(door_position);
  root[LAST_DOOR_ACTION     ] = last_door_action;
  root[IS_LOCKED            ] = is_locked;
  root[IS_IN_HOME_AREA      ] = is_in_home_area;
  root[TEMPERATURE          ] = temperature;
}

void SettingsFile::setJson(const JsonObject &json){
  door_position         = stringToDoorPosition(json[CURRENT_DOOR_POSITION].as<String>());
  last_door_action      = json[LAST_DOOR_ACTION     ].as<String>();
  is_locked             = json[IS_LOCKED            ].as<bool>();
  is_in_home_area       = json[IS_IN_HOME_AREA      ].as<bool>();
  temperature           = json[TEMPERATURE          ].as<double>();
}

bool SettingsFile::updateDoorPosition(doorPositions _current_door_position, doorPositions _door_position) {
  if (_current_door_position != _door_position) {
    door_position = _door_position;
    Debug.printf(String(ACTION_UPDATE_DOOR_POSITION).c_str(), 
      doorPositionToString(_current_door_position).c_str(), 
      doorPositionToString(door_position).c_str());
    return true;
  }
  return false;
}

doorPositions SettingsFile::stringToDoorPosition(const String &str){
  for (auto it = door_position_strings.begin(); it != door_position_strings.end(); ++it) {
    if(it->second == str){
      return it->first;
    }
  }
  return dpUnknown;
}

String SettingsFile::doorPositionToString(const  doorPositions position){
  return  door_position_strings[position];
}
