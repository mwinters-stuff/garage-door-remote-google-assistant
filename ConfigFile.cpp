#include "ConfigFile.h"

#define JSON_WIFI_AP            "wifi_ap"
#define JSON_WIFI_PASSWORD      "wifi_password"
#define JSON_HOSTNAME           "hostname"
#define JSON_UPDATE_INTERVAL    "update_interval"

#define JSON_INFLUX_HOST        "influx_host"
#define JSON_INFLUX_PORT        "influx_port"
#define JSON_INFLUX_USERNAME    "influx_username"
#define JSON_INFLUX_PASSWORD    "influx_password"
#define JSON_INFLUX_DATABASE    "influx_database"
#define JSON_INFLUX_MEASUREMENT "influx_measurement"

#define JSON_IO_FEED_DOOR_ACTION  "io_feed_garagedooraction"
#define JSON_IO_FEED_POSITION     "io_feed_garagedoorposition"
#define JSON_IO_FEED_IN_HOME_AREA "io_feed_inhomearea"
#define JSON_IO_FEED_USERNAME     "io_feed_username"
#define JSON_IO_FEED_KEY          "io_feed_key"


#ifdef TEST
#define CONFIG_FILE "/config-test.json"
#else
#define CONFIG_FILE "/config.json"
#endif

// ConfigFile *ConfigFile::m_instance;

ConfigFile::ConfigFile(): JSONFileBase(CONFIG_FILE){
  if(!readFile()){
    Serial.print(F("Failed to read "));
    Serial.println(fileName);
  }
}

void ConfigFile::getJson(JsonObject & root){

  root[JSON_WIFI_AP           ] = wifi_ap          ;
  root[JSON_WIFI_PASSWORD     ] = wifi_password    ;
  root[JSON_HOSTNAME          ] = hostname         ;
  root[JSON_INFLUX_HOST       ] = influx_host      ;
  root[JSON_INFLUX_PORT       ] = influx_port      ;
  root[JSON_INFLUX_USERNAME   ] = influx_username  ;
  root[JSON_INFLUX_PASSWORD   ] = influx_password  ;
  root[JSON_INFLUX_DATABASE   ] = influx_database  ;
  root[JSON_INFLUX_MEASUREMENT] = influx_measurement;
  root[JSON_UPDATE_INTERVAL   ] = update_interval;
  
  root[JSON_IO_FEED_DOOR_ACTION  ] = io_feed_door_action;
  root[JSON_IO_FEED_IN_HOME_AREA ] = io_feed_in_home_area;
  root[JSON_IO_FEED_POSITION     ] = io_feed_position;
  
  root[JSON_IO_FEED_USERNAME  ] = io_feed_username;
  root[JSON_IO_FEED_KEY       ] = io_feed_key;
}

void ConfigFile::setJson(const JsonObject &json){
  wifi_ap             = json[JSON_WIFI_AP           ].as<String>();
  wifi_password       = json[JSON_WIFI_PASSWORD     ].as<String>();
  hostname            = json[JSON_HOSTNAME          ].as<String>();
  influx_host         = json[JSON_INFLUX_HOST       ].as<String>();
  influx_port         = json[JSON_INFLUX_PORT       ].as<String>();
  influx_username     = json[JSON_INFLUX_USERNAME   ].as<String>();
  influx_password     = json[JSON_INFLUX_PASSWORD   ].as<String>();
  influx_database     = json[JSON_INFLUX_DATABASE   ].as<String>();
  influx_measurement  = json[JSON_INFLUX_MEASUREMENT].as<String>();
  update_interval     = json[JSON_UPDATE_INTERVAL   ].as<String>();
  update_interval_ms = update_interval.toInt() * 1000;

  io_feed_door_action  = json[JSON_IO_FEED_DOOR_ACTION ].as<String>();
  io_feed_in_home_area = json[JSON_IO_FEED_IN_HOME_AREA].as<String>();
  io_feed_position     = json[JSON_IO_FEED_POSITION    ].as<String>();

  io_feed_username     = json[JSON_IO_FEED_USERNAME    ].as<String>();
  io_feed_key          = json[JSON_IO_FEED_KEY         ].as<String>();
}

String ConfigFile::getInfluxUrl(bool timestamp){
  return "http://" + influx_host + ":" + influx_port 
    + "/write?db=" + influx_database 
    + "&u=" + influx_username + "&p=" + influx_password +
    (timestamp > 0 ? "&precision=s" : "");

}