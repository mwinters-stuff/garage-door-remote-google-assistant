#include "ConfigFile.h"
#include "StringConstants.h"

#ifdef TEST
#define CONFIG_FILE "/config-test.json"
#else
#define CONFIG_FILE "/config.json"
#endif

// ConfigFile *ConfigFile::m_instance;

ConfigFile::ConfigFile(): JSONFileBase(CONFIG_FILE){
  if(!readFile()){
    Serial.printf(String(FAILED_TO_READ).c_str(), fileName.c_str());
  }
}

void ConfigFile::getJson(JsonDocument & root){

  root[WIFI_AP                         ] = wifi_ap;
  root[WIFI_PASSWORD                   ] = wifi_password;
  root[HOSTNAME                        ] = hostname;
  // root[NTP_SERVER                      ] = ntp_server;
  root[UPDATE_INTERVAL                 ] = update_interval;
  root[MQTT_FEED_DOOR_POSITION         ] = mqtt_feed_door_position;
  root[MQTT_FEED_DOOR_LOCKED           ] = mqtt_feed_door_locked;
  root[MQTT_FEED_DOOR_POSITION_COMMAND ] = mqtt_feed_door_position_command;
  root[MQTT_FEED_DOOR_LOCKED_COMMAND   ] = mqtt_feed_door_locked_command;
  root[MQTT_FEED_ONLINE                ] = mqtt_feed_online;
  root[MQTT_FEED_TEMPERATURE           ] = mqtt_feed_temperature;
  root[MQTT_FEED_SONIC_CM              ] = mqtt_feed_sonic_cm;
  root[MQTT_USERNAME                   ] = mqtt_username;
  root[MQTT_PASSWORD                   ] = mqtt_password;
  root[MQTT_HOSTNAME                   ] = mqtt_hostname;
  root[MQTT_PORT                       ] = mqtt_port;
  root[MQTT_DEVICE_ID                  ] = mqtt_device_id;
  root[MQTT_DEVICE_NAME                ] = mqtt_device_name;
  root[DISTANCE_OPEN_MIN               ] = open_distance_min;
  root[DISTANCE_OPEN_MAX               ] = open_distance_max;
  // root[SYSLOG_APP_NAME                 ] = syslog_app_name;
  // root[SYSLOG_PORT                     ] = syslog_port;
  // root[SYSLOG_SERVER                   ] = syslog_server;

}

void ConfigFile::setJson(const JsonDocument &json){
  wifi_ap                         = json[WIFI_AP                        ].as<String>();
  wifi_password                   = json[WIFI_PASSWORD                  ].as<String>();
  hostname                        = json[HOSTNAME                       ].as<String>();
  // ntp_server                      = json[NTP_SERVER                     ].as<String>();
  update_interval                 = json[UPDATE_INTERVAL                ].as<String>();
  update_interval_ms              = update_interval.toInt() * 1000;
  mqtt_feed_door_position         = json[MQTT_FEED_DOOR_POSITION        ].as<String>();
  mqtt_feed_door_locked           = json[MQTT_FEED_DOOR_LOCKED          ].as<String>();
  mqtt_feed_door_position_command = json[MQTT_FEED_DOOR_POSITION_COMMAND].as<String>();
  mqtt_feed_door_locked_command   = json[MQTT_FEED_DOOR_LOCKED_COMMAND  ].as<String>();
  mqtt_feed_online                = json[MQTT_FEED_ONLINE               ].as<String>();
  mqtt_feed_temperature           = json[MQTT_FEED_TEMPERATURE          ].as<String>();
  mqtt_feed_sonic_cm              = json[MQTT_FEED_SONIC_CM             ].as<String>();
  mqtt_username                   = json[MQTT_USERNAME                  ].as<String>();
  mqtt_password                   = json[MQTT_PASSWORD                  ].as<String>();
  mqtt_hostname                   = json[MQTT_HOSTNAME                  ].as<String>();
  mqtt_port                       = json[MQTT_PORT                      ].as<uint16_t>();
  mqtt_device_id                  = json[MQTT_DEVICE_ID                 ].as<String>();
  mqtt_device_name                = json[MQTT_DEVICE_NAME               ].as<String>();
  open_distance_min               = json[DISTANCE_OPEN_MIN              ].as<uint16_t>();
  open_distance_max               = json[DISTANCE_OPEN_MAX              ].as<uint16_t>();
  // syslog_app_name                 = json[SYSLOG_APP_NAME                ].as<String>();
  // syslog_port                     = json[SYSLOG_PORT                    ].as<uint16_t>();
  // syslog_server                   = json[SYSLOG_SERVER                  ].as<String>();
}
