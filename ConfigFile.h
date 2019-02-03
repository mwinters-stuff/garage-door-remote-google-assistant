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

    String mqtt_hostname;
    String mqtt_feed_door_report_position;
    String mqtt_feed_door_set_position;
    String mqtt_feed_door_report_locked;
    String mqtt_feed_door_set_locked;
    String mqtt_feed_online;
    String mqtt_feed_temperature;
    String mqtt_username;
    String mqtt_password;
    uint16_t mqtt_port;

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
