#include "SettingsFile.h"

#define JSON_CURRENT_DOOR_POSITION "current_door_position"
#define JSON_LAST_DOOR_ACTION      "last_door_action"
#define JSON_IS_LOCKED             "is_locked"
#define JSON_IS_IN_HOME_AREA       "is_in_home_area"
#define JSON_TEMPERATURE           "temperature"


#ifdef TEST
#define SETTINGS_FILE "/settings-test.json"
#else
#define SETTINGS_FILE "/settings.json"
#endif

// SettingsFile *SettingsFile::m_instance;

SettingsFile::SettingsFile():JSONFileBase(SETTINGS_FILE){
  if(!readFile()){
    Serial.print(F("Failed to read "));
    Serial.println(fileName);
    current_door_position = "";
    last_door_action = "";
    is_locked = false;
    is_in_home_area = true;
  }
}

void SettingsFile::getJson(JsonObject & root){

  root[JSON_CURRENT_DOOR_POSITION] = current_door_position;
  root[JSON_LAST_DOOR_ACTION     ] = last_door_action;
  root[JSON_IS_LOCKED            ] = is_locked;
  root[JSON_IS_IN_HOME_AREA      ] = is_in_home_area;
  root[JSON_TEMPERATURE          ] = temperature;
}

void SettingsFile::setJson(const JsonObject &json){
  current_door_position = json[JSON_CURRENT_DOOR_POSITION].as<String>();
  last_door_action      = json[JSON_LAST_DOOR_ACTION     ].as<String>();
  is_locked             = json[JSON_IS_LOCKED            ].as<bool>();
  is_in_home_area       = json[JSON_IS_IN_HOME_AREA      ].as<bool>();
  temperature           = json[JSON_TEMPERATURE          ].as<double>();
}
