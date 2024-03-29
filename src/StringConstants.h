#ifndef _STRING_CONSTANTS_H
#define _STRING_CONSTANTS_H
#include <WString.h>

extern const char _OKRESULT                       [] PROGMEM;
extern const char _ACCESS_CONTROL_HEADER          [] PROGMEM;
extern const char _TEXT_PLAIN                     [] PROGMEM;
extern const char _INVALID_ARGUMENTS              [] PROGMEM;
extern const char _HEAP                           [] PROGMEM;
extern const char _TEMPERATURE                    [] PROGMEM;
extern const char _DOOR_CLOSED                    [] PROGMEM;
extern const char _LOCK_ACTION                    [] PROGMEM;
extern const char _DOOR_COMMAND                   [] PROGMEM;
extern const char _DOOR_POSITION                  [] PROGMEM;
extern const char _DOOR_LOCKED                    [] PROGMEM;
extern const char _SONIC_DISTANCE                 [] PROGMEM;
extern const char _UP_TIME                        [] PROGMEM;
extern const char _BOOT_TIME                      [] PROGMEM;
extern const char _TIME_STAMP                     [] PROGMEM;
extern const char _UNKNOWN_ACTION                 [] PROGMEM;
extern const char _IS_LOCKED                      [] PROGMEM;
extern const char _WIFI_AP                        [] PROGMEM;
extern const char _WIFI_PASSWORD                  [] PROGMEM;
extern const char _HOSTNAME                       [] PROGMEM;
extern const char _UPDATE_INTERVAL                [] PROGMEM;
extern const char _NTP_SERVER                     [] PROGMEM;
extern const char _IS_CLOSED                      [] PROGMEM;
extern const char _DISTANCE_OPEN_MIN              [] PROGMEM;
extern const char _DISTANCE_OPEN_MAX              [] PROGMEM;
extern const char _SYSLOG_SERVER                  [] PROGMEM;
extern const char _SYSLOG_PORT                    [] PROGMEM;
extern const char _SYSLOG_APP_NAME                [] PROGMEM;
extern const char _MQTT_FEED_DOOR_POSITION        [] PROGMEM;
extern const char _MQTT_FEED_DOOR_LOCKED          [] PROGMEM;
extern const char _MQTT_FEED_DOOR_POSITION_COMMAND[] PROGMEM;
extern const char _MQTT_FEED_DOOR_LOCKED_COMMAND  [] PROGMEM;
extern const char _MQTT_FEED_ONLINE               [] PROGMEM;
extern const char _MQTT_FEED_TEMPERATURE          [] PROGMEM;
extern const char _MQTT_FEED_SONIC_CM             [] PROGMEM;
extern const char _MQTT_USERNAME                  [] PROGMEM;
extern const char _MQTT_PASSWORD                  [] PROGMEM;
extern const char _MQTT_HOSTNAME                  [] PROGMEM;
extern const char _MQTT_PORT                      [] PROGMEM;
extern const char _MQTT_DEVICE_ID                 [] PROGMEM;
extern const char _MQTT_DEVICE_NAME               [] PROGMEM;
extern const char _OPEN                           [] PROGMEM;
extern const char _CLOSE                          [] PROGMEM;
extern const char _OPENING                        [] PROGMEM;
extern const char _CLOSING                        [] PROGMEM;
extern const char _FORCE                          [] PROGMEM;
extern const char _CLOSED                         [] PROGMEM;
extern const char _LOCK                           [] PROGMEM;
extern const char _LOCKED                         [] PROGMEM;
extern const char _UNLOCKED                       [] PROGMEM;
extern const char _UNLOCK                         [] PROGMEM;
extern const char _ONLINE                         [] PROGMEM;
extern const char _OFFLINE                        [] PROGMEM;
extern const char _YES                            [] PROGMEM;
extern const char _NO                             [] PROGMEM;
extern const char _ERROR_FAILED                   [] PROGMEM;
extern const char _FAILED_TO_READ                 [] PROGMEM;
extern const char _INFLUX_URL                     [] PROGMEM;

#define OKRESULT                        FPSTR(_OKRESULT)
#define ACCESS_CONTROL_HEADER           FPSTR(_ACCESS_CONTROL_HEADER)
#define TEXT_PLAIN                      FPSTR(_TEXT_PLAIN)
#define INVALID_ARGUMENTS               FPSTR(_INVALID_ARGUMENTS)
#define HEAP                            FPSTR(_HEAP)
#define TEMPERATURE                     FPSTR(_TEMPERATURE)
#define DOOR_CLOSED                     FPSTR(_DOOR_CLOSED)
#define LOCK_ACTION                     FPSTR(_LOCK_ACTION)
#define DOOR_COMMAND                    FPSTR(_DOOR_COMMAND)
#define DOOR_POSITION                   FPSTR(_DOOR_POSITION)
#define DOOR_LOCKED                     FPSTR(_DOOR_LOCKED)
#define SONIC_DISTANCE                  FPSTR(_SONIC_DISTANCE)
#define UP_TIME                         FPSTR(_UP_TIME)
#define BOOT_TIME                       FPSTR(_BOOT_TIME)
#define TIME_STAMP                      FPSTR(_TIME_STAMP)
#define UNKNOWN_ACTION                  FPSTR(_UNKNOWN_ACTION)
#define IS_LOCKED                       FPSTR(_IS_LOCKED)
#define WIFI_AP                         FPSTR(_WIFI_AP)
#define WIFI_PASSWORD                   FPSTR(_WIFI_PASSWORD)
#define HOSTNAME                        FPSTR(_HOSTNAME)
#define UPDATE_INTERVAL                 FPSTR(_UPDATE_INTERVAL)
#define NTP_SERVER                      FPSTR(_NTP_SERVER)
#define IS_CLOSED                       FPSTR(_IS_CLOSED)
#define DISTANCE_OPEN_MIN               FPSTR(_DISTANCE_OPEN_MIN)
#define DISTANCE_OPEN_MAX               FPSTR(_DISTANCE_OPEN_MAX)
#define OPEN                            FPSTR(_OPEN)
#define CLOSE                           FPSTR(_CLOSE)
#define OPENING                         FPSTR(_OPENING)
#define CLOSING                         FPSTR(_CLOSING)
#define FORCE                           FPSTR(_FORCE)
#define CLOSED                          FPSTR(_CLOSED)
#define LOCK                            FPSTR(_LOCK)
#define LOCKED                          FPSTR(_LOCKED)
#define UNLOCKED                        FPSTR(_UNLOCKED)
#define UNLOCK                          FPSTR(_UNLOCK)
#define ONLINE                          FPSTR(_ONLINE)
#define OFFLINE                         FPSTR(_OFFLINE)
#define YES                             FPSTR(_YES)
#define NO                              FPSTR(_NO)
#define ERROR_FAILED                    FPSTR(_ERROR_FAILED)
#define FAILED_TO_READ                  FPSTR(_FAILED_TO_READ)
#define INFLUX_URL                      FPSTR(_INFLUX_URL)
#define SYSLOG_SERVER                   FPSTR(_SYSLOG_SERVER)
#define SYSLOG_PORT                     FPSTR(_SYSLOG_PORT)
#define SYSLOG_APP_NAME                 FPSTR(_SYSLOG_APP_NAME)
#define MQTT_FEED_DOOR_POSITION         FPSTR(_MQTT_FEED_DOOR_POSITION)
#define MQTT_FEED_DOOR_LOCKED           FPSTR(_MQTT_FEED_DOOR_LOCKED)
#define MQTT_FEED_DOOR_POSITION_COMMAND FPSTR(_MQTT_FEED_DOOR_POSITION_COMMAND)
#define MQTT_FEED_DOOR_LOCKED_COMMAND   FPSTR(_MQTT_FEED_DOOR_LOCKED_COMMAND)
#define MQTT_FEED_ONLINE                FPSTR(_MQTT_FEED_ONLINE)
#define MQTT_FEED_TEMPERATURE           FPSTR(_MQTT_FEED_TEMPERATURE)
#define MQTT_FEED_SONIC_CM              FPSTR(_MQTT_FEED_SONIC_CM)
#define MQTT_USERNAME                   FPSTR(_MQTT_USERNAME)
#define MQTT_PASSWORD                   FPSTR(_MQTT_PASSWORD)
#define MQTT_HOSTNAME                   FPSTR(_MQTT_HOSTNAME)
#define MQTT_PORT                       FPSTR(_MQTT_PORT)
#define MQTT_DEVICE_ID                  FPSTR(_MQTT_DEVICE_ID)
#define MQTT_DEVICE_NAME                FPSTR(_MQTT_DEVICE_NAME)


#endif