#ifndef _CONFIGFILE_H
#define _CONFIGFILE_H
#include <Arduino.h>
#include <ArduinoJson.h>
#include "JSONFileBase.h"

class ConfigFile: public JSONFileBase{
  public:
    String wifi_ap;
    String wifi_password;
    String hostname;
    String ntp_server;
    String update_interval;
    uint16_t update_interval_ms;

    String mqtt_hostname;
    String mqtt_feed_door_position;
    String mqtt_feed_door_locked;
    String mqtt_feed_door_position_command;
    String mqtt_feed_door_locked_command;
    String mqtt_feed_online;
    String mqtt_feed_temperature;
    String mqtt_feed_sonic_cm;
    String mqtt_username;
    String mqtt_password;
    uint16_t mqtt_port;
    String mqtt_device_id;
    String mqtt_device_name;
    

    uint16_t open_distance_min;
    uint16_t open_distance_max;

    // String syslog_server;
    // uint16_t syslog_port;
    // String syslog_app_name;

    ConfigFile();

    void getJson(JsonDocument & root) override;
    void setJson(const JsonDocument &object) override;
};


#endif
