#!/bin/bash
(
  cd webapp || exit
  npm install
  polymer build 
  mkdir -p ../data
  cp build/es6prod/* ../data/
)
rm ./data/*.gz
gzip ./data/index.html
gzip ./data/manifest.json
gzip ./data/service-worker.js

# make fs
# /home/mathew/Arduino/Esp8266-Arduino-Makefile/esp8266-2.4.2/tools/espota.py -i garage-door.fritz.box -p 8266  -a 123 -s -f build.nodemcuv2-2.4.2/spiffs/spiffs.bin
# /home/mathew/Arduino/Esp8266-Arduino-Makefile/esp8266-2.4.2/tools/espota.py -i garage-door-test.fritz.box -p 8266  -a leo -s -f build.nodemcuv2-2.4.2/spiffs/spiffs.bin
