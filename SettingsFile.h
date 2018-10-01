#ifndef _SETTINGS_H
#define _SETTINGS_H
#include "JSONFileBase.h"

class SettingsFile: public JSONFileBase{
  public:
    String current_door_position;
    String last_door_action;
    bool   is_locked;
    bool   is_in_home_area;
    double temperature;

    SettingsFile();
    // static SettingsFile* getInstance(){
    //   return SettingsFile::m_instance;
    // }

    void getJson(JsonObject & root);
    void setJson(const JsonObject &object);
  private:
    // static SettingsFile* m_instance;

};


#endif
