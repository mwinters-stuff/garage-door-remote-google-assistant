ARDUINO_VARIANT = nodemcuv2
# UPLOAD_PORT = /dev/nodemcu
UPLOAD_PORT = /dev/ttyUSB1
UPLOAD_SPEED=921600

SERIAL_PORT = $(UPLOAD_PORT)
SERIAL_BAUD = 115200

LOG_SERIAL_TO_FILE=yes
USER_DEFINE = -DTEST
OTA_IP = garage-door.fritz.box
OTA_PORT = 8266 
OTA_AUTH = 123
GLOBAL_USER_LIBDIR=./libs
LOCAL_LIBS=./libs
USER_LIBS=$(LOCAL_LIBS)/Time \
					$(LOCAL_LIBS)/NtpClientLib/src \
					$(LOCAL_LIBS)/RemoteDebug \
					$(LOCAL_LIBS)/DallasTemperature \
					$(LOCAL_LIBS)/OneWire \
					$(LOCAL_LIBS)/ArduinoJson/src \
					$(LOCAL_LIBS)/ESPAsyncTCP/src \
					$(LOCAL_LIBS)/ESPAsyncWebServer/src \
          $(LOCAL_LIBS)/Adafruit_IO_Arduino \
          $(LOCAL_LIBS)/Adafruit_MQTT_Library \
					$(LOCAL_LIBS)/ESP8266DebounceButtons/src

include ~/Arduino/Esp8266-Arduino-Makefile/espXArduino.mk
