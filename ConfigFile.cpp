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
  root[INFLUX_DOOR       ] = influx_door;
  root[UPDATE_INTERVAL   ] = update_interval;
  root[IO_FEED_DOOR_ACTION  ] = io_feed_door_action;
  root[IO_FEED_IN_HOME_AREA ] = io_feed_in_home_area;
  root[IO_FEED_POSITION     ] = io_feed_position;
  root[IO_FEED_USERNAME  ] = io_feed_username;
  root[IO_FEED_KEY       ] = io_feed_key;
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
  influx_door         = json[INFLUX_DOOR       ].as<String>();
  update_interval     = json[UPDATE_INTERVAL   ].as<String>();
  update_interval_ms = update_interval.toInt() * 1000;

  io_feed_door_action  = json[IO_FEED_DOOR_ACTION ].as<String>();
  io_feed_in_home_area = json[IO_FEED_IN_HOME_AREA].as<String>();
  io_feed_position     = json[IO_FEED_POSITION    ].as<String>();

  io_feed_username     = json[IO_FEED_USERNAME    ].as<String>();
  io_feed_key          = json[IO_FEED_KEY         ].as<String>();
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