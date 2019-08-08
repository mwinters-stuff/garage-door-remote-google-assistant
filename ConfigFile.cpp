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

void ConfigFile::getJson(JsonObject & root){

  root[WIFI_AP           ] = wifi_ap          ;
  root[WIFI_PASSWORD     ] = wifi_password    ;
  root[HOSTNAME          ] = hostname         ;
  root[NTP_SERVER        ] = ntp_server       ;
  root[UPDATE_INTERVAL   ] = update_interval;

  root[MQTT_FEED_DOOR_POSITION        ] = mqtt_feed_door_position;
  root[MQTT_FEED_DOOR_LOCKED          ] = mqtt_feed_door_locked;
  root[MQTT_FEED_ONLINE               ] = mqtt_feed_online;
  root[MQTT_FEED_TEMPERATURE          ] = mqtt_feed_temperature;
  root[MQTT_USERNAME                  ] = mqtt_username;
  root[MQTT_PASSWORD                  ] = mqtt_password;
  root[MQTT_HOSTNAME                  ] = mqtt_hostname;
  root[MQTT_PORT                      ] = mqtt_port;

}

void ConfigFile::setJson(const JsonObject &json){
  wifi_ap             = json[WIFI_AP           ].as<String>();
  wifi_password       = json[WIFI_PASSWORD     ].as<String>();
  hostname            = json[HOSTNAME          ].as<String>();
  ntp_server          = json[NTP_SERVER        ].as<String>();
  update_interval     = json[UPDATE_INTERVAL   ].as<String>();
  update_interval_ms = update_interval.toInt() * 1000;

  mqtt_feed_door_position=        json[MQTT_FEED_DOOR_POSITION        ].as<String>();
  mqtt_feed_door_locked=          json[MQTT_FEED_DOOR_LOCKED          ].as<String>();
  mqtt_feed_online=               json[MQTT_FEED_ONLINE               ].as<String>();
  mqtt_feed_temperature=          json[MQTT_FEED_TEMPERATURE          ].as<String>();
  mqtt_username=                  json[MQTT_USERNAME                  ].as<String>();
  mqtt_password=                  json[MQTT_PASSWORD                  ].as<String>();
  mqtt_hostname=                  json[MQTT_HOSTNAME                  ].as<String>();
  mqtt_port=                      json[MQTT_PORT                      ].as<uint16_t>();
}
