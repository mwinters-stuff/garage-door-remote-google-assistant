UPLOAD_PORT = /dev/ttyUSB1
BOARD=nodemcuv2
SKETCH=garage-door-remote-google-assistant.ino
BUILD_ROOT=build

ESP_ADDR=garage-door-test
ESP_PWD=123
HTTP_ADDR = garage-door-test
HTTP_URI = /update

LIBS=$(ESP_LIBS)/../cores/esp8266 \
  $(ESP_LIBS)/ESP8266WiFi \
  $(ESP_LIBS)/ESP8266mDNS \
	$(ESP_LIBS)/ESP8266WebServer \
	$(ESP_LIBS)/ArduinoOTA \
	$(ESP_LIBS)/EEPROM \
	$(ESP_LIBS)/ESP8266HTTPUpdateServer \
	$(ARDUINO_LIBS)/Adafruit_MQTT_Library \
  $(ARDUINO_LIBS)/ArduinoHttpClient \
  $(ARDUINO_LIBS)/r89m_Buttons \
	$(ARDUINO_LIBS)/r89m_PushButton \
	$(ARDUINO_LIBS)/Bounce2 \
	$(ARDUINO_LIBS)/OneWire \
	$(ARDUINO_LIBS)/Adafruit_IO_Arduino \
	$(ARDUINO_LIBS)/DallasTemperature 
	
BUILD_EXTRA_FLAGS=-DTEST 
# -DARDUINO_OTA_PASSWORD=\"$(ESP_PWD)\"
# test:
# 	BUILD_EXTRA_FLAGS=-DTEST 

# release:
	
include makeEspArduino.mk

