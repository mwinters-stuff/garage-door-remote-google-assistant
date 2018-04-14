#!/bin/bash
(
  cd webapp || exit
  bower install
  polymer build --bundle
  mkdir -p ../data
  cp build/default/index.html ../data/
)
rm ./data/index.html.gz
gzip ./data/index.html
gzip -l ./data/index.html.gz

# ~/.arduino15/packages/esp8266/tools/mkspiffs/0.2.0/mkspiffs --create ./data --size 1028096 --page 256 --block 8192 ./spiffs.image
# ~/.arduino15/packages/esp8266/tools/esptool/0.4.13/esptool -ca 0x300000 -cd nodemcu -cp /dev/ttyUSB1 -cb 115200 -cf ~/spiffs.image
