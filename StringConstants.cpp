#include "StringConstants.h"
#include <Arduino.h>

#include <WString.h>

const char _OKRESULT[] PROGMEM              = "OK";
const char _ACCESS_CONTROL_HEADER[] PROGMEM = "Access-Control-Allow-Origin";
const char _TEXT_PLAIN[] PROGMEM            = "text/plain";
const char _INVALID_ARGUMENTS[] PROGMEM     = "Invalid Arguments";
const char _HEAP[] PROGMEM                  = "heap";
const char _TEMPERATURE[] PROGMEM           = "temperature";
const char _DOOR_ACTION[] PROGMEM           = "door_action";
const char _LOCK_ACTION[] PROGMEM           = "lock_action";
const char _DOOR_POSITION[] PROGMEM         = "door_position";
const char _DOOR_LOCKED[] PROGMEM           = "door_locked";
const char _IN_HOME_AREA[] PROGMEM          = "in_home_area";
const char _LAST_HTTP_RESPONSE[] PROGMEM    = "last_http_response";
const char _UP_TIME[] PROGMEM               = "up_time";
const char _BOOT_TIME[] PROGMEM             = "boot_time";
const char _TIME_STAMP[] PROGMEM            = "time_stamp";

const char _UNKNOWN_ACTION[] PROGMEM = "ERROR: %s : %s is an unknown action";

const char _CURRENT_DOOR_POSITION[] PROGMEM = "current_door_position";
const char _LAST_DOOR_ACTION[] PROGMEM      = "last_door_action";
const char _IS_LOCKED[] PROGMEM             = "is_locked";
const char _IS_IN_HOME_AREA[] PROGMEM       = "is_in_home_area";

const char _WIFI_AP[] PROGMEM              = "wifi_ap";
const char _WIFI_PASSWORD[] PROGMEM        = "wifi_password";
const char _HOSTNAME[] PROGMEM             = "hostname";
const char _UPDATE_INTERVAL[] PROGMEM      = "update_interval";
const char _INFLUX_HOST[] PROGMEM          = "influx_host";
const char _INFLUX_PORT[] PROGMEM          = "influx_port";
const char _INFLUX_USERNAME[] PROGMEM      = "influx_username";
const char _INFLUX_PASSWORD[] PROGMEM      = "influx_password";
const char _INFLUX_DATABASE[] PROGMEM      = "influx_database";
const char _INFLUX_MEASUREMENT[] PROGMEM   = "influx_measurement";
const char _INFLUX_MEASUREMENT_TEMPERATURE[] PROGMEM   = "influx_measurement_temperature";
const char _INFLUX_TEMPERATURE_TAGS[] PROGMEM      = "influx_temperature_tags";
const char _INFLUX_DOOR[] PROGMEM          = "influx_door";

const char _MQTT_FEED_DOOR_SET_POSITION[] PROGMEM = "mqtt_feed_set_position";
const char _MQTT_FEED_DOOR_REPORT_POSITION[] PROGMEM = "mqtt_feed_report_position";
const char _MQTT_FEED_DOOR_SET_LOCKED[] PROGMEM = "mqtt_feed_set_locked";
const char _MQTT_FEED_DOOR_REPORT_LOCKED[] PROGMEM = "mqtt_feed_report_locked";
const char _MQTT_FEED_ONLINE[] PROGMEM = "mqtt_feed_online";
const char _MQTT_FEED_TEMPERATURE[] PROGMEM = "mqtt_feed_temperature";
const char _MQTT_USERNAME[] PROGMEM = "mqtt_username";
const char _MQTT_PASSWORD[] PROGMEM = "mqtt_password";
const char _MQTT_PORT[] PROGMEM = "mqtt_port";
const char _MQTT_HOSTNAME[] PROGMEM = "mqtt_hostname";


// const char _IO_FEED_DOOR_ACTION[] PROGMEM  = "io_feed_garagedooraction";
// const char _IO_FEED_POSITION[] PROGMEM     = "io_feed_garagedoorposition";
// const char _IO_FEED_IN_HOME_AREA[] PROGMEM = "io_feed_inhomearea";
// const char _IO_FEED_USERNAME[] PROGMEM     = "io_feed_username";
// const char _IO_FEED_KEY[] PROGMEM          = "io_feed_key";

const char _OPEN[] PROGMEM   = "OPEN";
const char _CLOSE[] PROGMEM  = "CLOSE";
const char _CLOSED[] PROGMEM = "CLOSED";
const char _LOCK[] PROGMEM   = "LOCK";
const char _UNLOCK[] PROGMEM = "UNLOCK";
const char _LOCKED[] PROGMEM   = "LOCKED";
const char _UNLOCKED[] PROGMEM   = "UNLOCKED";
const char _YES[] PROGMEM    = "YES";
const char _NO[] PROGMEM     = "NO";
const char _MOVING[] PROGMEM = "MOVING";

const char _ERROR_FAILED[] PROGMEM = "Error[%u]: %s Failed\n";

const char _STARTUP[] PROGMEM                     = "STARTUP";
const char _UNKNOWN[] PROGMEM                     = "UNKNOWN";
const char _OPEN_CLOSED[] PROGMEM                 = "OPEN_CLOSED";
const char _CLOSED_OPEN[] PROGMEM                 = "CLOSED_OPEN";
const char _MANUAL_OPEN_CLOSED[] PROGMEM          = "MANUAL_OPEN_CLOSED";
const char _MANUAL_CLOSED_OPEN[] PROGMEM          = "MANUAL_CLOSED_OPEN";
const char _FAILED_TO_READ[] PROGMEM              = "Failed to read %s\n";
const char _ACTION_UPDATE_DOOR_POSITION[] PROGMEM = "Action: Update Door Position %s --> %s\n";
const char _INFLUX_URL[] PROGMEM = "http://%s:%s/write?db=%s";