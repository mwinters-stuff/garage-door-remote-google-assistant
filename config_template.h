// Copy this and name it config.h, then edit config.h
#define WIFI_SSID "<<WIFI SID>>"
#define WIFI_PASS "<<WIFI PASSWORD>>"

#define IO_USERNAME "<<Adifruit.io username>>"
#define IO_KEY "<<Adafruit.io api key>>"
#define ARDUINO_OTA_PASSWORD "<<apassword>>"

#define GEOFENCE

#define IO_FEED_DOOR_ACTION "garagedooraction"
#define IO_FEED_POSITION "garagedoorposition"
#define IO_FEED_TEMPERATURE "garagetemperature"
#define IO_FEED_IN_HOME_AREA "inhomearea"
#define IO_FEED_RESET_REASON  "resetreason"

#define OPEN_CLOSE_BUTTON D0
#define ONE_WIRE_PIN D1
#define LED_GREEN D2
#define LED_RED D3
#define RELAY D8
#define SWITCH_OPEN D5
#define SWITCH_CLOSED D6

#define LED_ON HIGH
#define LED_OFF LOW

#define RELAY_CLOSED HIGH
#define RELAY_OPEN LOW

#define TEMPERATURE_DELAY 600000

// Shouldnt need to change these.

#define EEPROM_SIZE 16
#define  EEPROM_IN_GEOFENCE_ADDR 0
#define  EEPROM_LOCKED_ADDR 1