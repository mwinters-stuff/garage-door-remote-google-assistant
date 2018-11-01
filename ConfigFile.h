#ifndef _CONFIGFILE_H
#define _CONFIGFILE_H
#include <Arduino.h>
#include <ArduinoJson.h>
#include <Time.h>
#include "JSONFileBase.h"

class ConfigFile: public JSONFileBase{
  public:
    String wifi_ap;
    String wifi_password;
    String hostname;
    String influx_database;
    String influx_measurement;
    String influx_door;
    String influx_host;
    String influx_username;
    String influx_password;
    String influx_port;
    String update_interval;
    String influx_measurement_temperature;
    String influx_temperature_tags;
    uint16_t update_interval_ms;

    String io_feed_door_action;
    String io_feed_in_home_area;
    String io_feed_position;
    String io_feed_username;
    String io_feed_key;

    ConfigFile();
    // static ConfigFile* getInstance(){
    //   return ConfigFile::m_instance;
    // }

    void getJson(JsonObject & root);
    void setJson(const JsonObject &object);
    String getInfluxUrl();
    bool influxOk();
  private:
    // static ConfigFile* m_instance;

};


#endif
