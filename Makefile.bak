ARDUINO_PATH = /opt/arduino-1.8.5
ARDUINO_DOT = $(HOME)/.arduino15
SKETCHBOOK = $(HOME)/Arduino
ESPTOOL = $(HOME)/.arduino15/packages/esp8266/tools/esptool/0.4.13/esptool
ESPOTA = $(HOME)/.arduino15/packages/esp8266/hardware/esp8266/2.4.1/tools/espota.py
SKETCH = $(notdir $(CURDIR)).ino
TARGET_DIR = $(CURDIR)/build-esp8266
MONITOR_PORT = /dev/ttyUSB1
ESP_IP = garage-door-test
OTA_SERVER = garage-door-test
OTA_PASSWD = harper
#DEBUG = ,Debug=Serial,DebugLevel=all_____
DEBUG = 
BAUD=115200
BOARD=esp8266:esp8266:nodemcuv2
BOARD_OPTIONS=CpuFrequency=80,FlashSize=4M1M,LwIPVariant=v2mss536,FlashErase=none


all:
	@ mkdir -p $(TARGET_DIR)

	$(ARDUINO_PATH)/arduino-builder -compile \
		-hardware $(ARDUINO_PATH)/hardware \
		-hardware $(ARDUINO_DOT)/packages \
		-tools $(ARDUINO_PATH)/tools-builder \
		-tools $(ARDUINO_PATH)/hardware/tools/avr \
		-tools $(ARDUINO_DOT)/packages \
		-built-in-libraries $(ARDUINO_PATH)/libraries \
		-libraries $(SKETCHBOOK)/libraries \
		-fqbn=$(BOARD):$(BOARD_OPTIONS),UploadSpeed=$(BAUD)$(DEBUG) \
		-ide-version=10805 \
		-build-path "$(TARGET_DIR)" \
		-warnings=all \
		-verbose \
		-prefs=build.extra_flags=-DTEST \
		-prefs=build.warn_data_percentage=75 \
		-prefs=runtime.tools.mkspiffs.path=$(ARDUINO_DOT)/packages/esp8266/tools/mkspiffs/0.2.0 \
		-prefs=runtime.tools.xtensa-lx106-elf-gcc.path=$(ARDUINO_DOT)/packages/esp8266/tools/xtensa-lx106-elf-gcc/1.20.0-26-gb404fb9-2 \
		-prefs=runtime.tools.esptool.path=$(ARDUINO_DOT)/packages/esp8266/tools/esptool/0.4.13 \
		"$(SKETCH)"

upload:
	$(ESPTOOL) -v -cd nodemcu -cb $(BAUD) -cp $(MONITOR_PORT) -ca 0x00000 -cf $(TARGET_DIR)/$(SKETCH).bin

ota:
	$(ESPOTA) -d -r -i $(ESP_IP) -I $(OTA_SERVER) -p 8266 -P 8266 -a $(OTA_PASSWD) -f $(TARGET_DIR)/$(SKETCH).bin

clean:
	rm -rf $(TARGET_DIR)

monitor:
	# screen $(MONITOR_PORT) $(BAUD)
	python /usr/lib/python2.7/dist-packages/serial/tools/miniterm.py $(MONITOR_PORT) $(BAUD)