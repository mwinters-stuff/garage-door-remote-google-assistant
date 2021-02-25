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

SettingsFile::SettingsFile():JSONFileBase(SETTINGS_FILE){
  if(!readFile()){
    Serial.printf(String(FAILED_TO_READ).c_str(), fileName.c_str());
    is_closed = false;
    is_locked = false;
  }
}

void SettingsFile::getJson(JsonDocument & root){

  root[IS_CLOSED            ] = is_closed;
  root[IS_LOCKED            ] = is_locked;
  root[TEMPERATURE          ] = temperature;
}

void SettingsFile::setJson(const JsonDocument &json){
  is_closed             = json[IS_CLOSED            ].as<bool>();
  is_locked             = json[IS_LOCKED            ].as<bool>();
  temperature           = json[TEMPERATURE          ].as<double>();
}

