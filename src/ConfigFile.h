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
    String ntp_server;
    String update_interval;
    uint16_t update_interval_ms;

    String mqtt_hostname;
    String mqtt_feed_door_position;
    String mqtt_feed_door_locked;
    String mqtt_feed_online;
    String mqtt_feed_temperature;
    String mqtt_username;
    String mqtt_password;
    uint16_t mqtt_port;

    ConfigFile();

    void getJson(JsonDocument & root) override;
    void setJson(const JsonDocument &object) override;
};


#endif
