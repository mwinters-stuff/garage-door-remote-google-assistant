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
  root[INFLUX_HOST       ] = influx_host      ;
  root[INFLUX_PORT       ] = influx_port      ;
  root[INFLUX_USERNAME   ] = influx_username  ;
  root[INFLUX_PASSWORD   ] = influx_password  ;
  root[INFLUX_DATABASE   ] = influx_database  ;
  root[INFLUX_MEASUREMENT] = influx_measurement;
  root[INFLUX_MEASUREMENT_TEMPERATURE] = influx_measurement_temperature;
  root[INFLUX_TEMPERATURE_TAGS]= influx_temperature_tags;
  root[INFLUX_DOOR       ] = influx_door;
  root[UPDATE_INTERVAL   ] = update_interval;

  root[MQTT_FEED_DOOR_SET_POSITION    ] = mqtt_feed_door_set_position;
  root[MQTT_FEED_DOOR_REPORT_POSITION ] = mqtt_feed_door_report_position;
  root[MQTT_FEED_DOOR_SET_LOCKED      ] = mqtt_feed_door_set_locked;
  root[MQTT_FEED_DOOR_REPORT_LOCKED   ] = mqtt_feed_door_report_locked;
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
  influx_host         = json[INFLUX_HOST       ].as<String>();
  influx_port         = json[INFLUX_PORT       ].as<String>();
  influx_username     = json[INFLUX_USERNAME   ].as<String>();
  influx_password     = json[INFLUX_PASSWORD   ].as<String>();
  influx_database     = json[INFLUX_DATABASE   ].as<String>();
  influx_measurement  = json[INFLUX_MEASUREMENT].as<String>();
  influx_measurement_temperature  = json[INFLUX_MEASUREMENT_TEMPERATURE].as<String>();
  influx_temperature_tags  = json[INFLUX_TEMPERATURE_TAGS].as<String>();
  influx_door         = json[INFLUX_DOOR       ].as<String>();
  update_interval     = json[UPDATE_INTERVAL   ].as<String>();
  update_interval_ms = update_interval.toInt() * 1000;

  mqtt_feed_door_set_position=    json[MQTT_FEED_DOOR_SET_POSITION    ].as<String>();
  mqtt_feed_door_report_position= json[MQTT_FEED_DOOR_REPORT_POSITION ].as<String>();
  mqtt_feed_door_set_locked=      json[MQTT_FEED_DOOR_SET_LOCKED      ].as<String>();
  mqtt_feed_door_report_locked=   json[MQTT_FEED_DOOR_REPORT_LOCKED   ].as<String>();
  mqtt_feed_online=               json[MQTT_FEED_ONLINE               ].as<String>();
  mqtt_feed_temperature=          json[MQTT_FEED_TEMPERATURE          ].as<String>();
  mqtt_username=                  json[MQTT_USERNAME                  ].as<String>();
  mqtt_password=                  json[MQTT_PASSWORD                  ].as<String>();
  mqtt_hostname=                  json[MQTT_HOSTNAME                  ].as<String>();
  mqtt_port=                      json[MQTT_PORT                      ].as<uint16_t>();
}

String ConfigFile::getInfluxUrl(){
  char buffer[200];
  snprintf(buffer, 200, String(INFLUX_URL).c_str(),
    influx_host.c_str(), influx_port.c_str(), influx_database.c_str(),
    influx_username.c_str(), influx_password.c_str());
  String r = String(buffer);
  if(influx_username.length() > 0 && influx_password.length() > 0){
    r += "&u=" + influx_username + "&p=" + influx_password;
  }
  return r;
}

bool ConfigFile::influxOk(){
  return influx_host.length() > 0 && influx_database.length() > 0
    && influx_port.length() > 0 && influx_measurement.length() > 0
    && influx_door.length() > 0;
}