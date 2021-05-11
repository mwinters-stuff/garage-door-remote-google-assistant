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

