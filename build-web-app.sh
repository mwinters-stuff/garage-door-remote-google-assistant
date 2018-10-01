(
  cd webapp || exit
  npm install
  polymer build --bundle
  mkdir -p ../data
  cp build/default/index.html ../data/
)
rm ./data/index.html.gz
gzip ./data/index.html
gzip -l ./data/index.html.gz

make fs
# /home/mathew/Arduino/Esp8266-Arduino-Makefile/esp8266-2.4.2/tools/espota.py -i weather-station.fritz.box -p 8266  -a 123 -s -f build.nodemcuv2-2.4.2/spiffs/spiffs.bin
