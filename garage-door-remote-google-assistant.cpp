
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <AdafruitIO_WiFi.h>
#include <ESP8266mDNS.h>

#include <AdafruitIO.h>
#include <AdafruitIO_Feed.h>
#include <AdafruitIO_WiFi.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#include "config.h"
#include "datastore.h"
#include "httphandler.h"
#include "iohandler.h"
#include "logging.h"

#include <user_interface.h>

DataStore* the_data_store;
IOHandler* the_io_handler;
HTTPHandler* the_http_handler;
Logging* the_logger;



AdafruitIO_WiFi io(IO_USERNAME, IO_KEY, WIFI_SSID, WIFI_PASS);

OneWire oneWire(ONE_WIRE_PIN);
DallasTemperature DS18B20(&oneWire);
uint32_t temperatureLastRead;

void handleDoorActionMessage(AdafruitIO_Data *data);
void connect_adafruit_io();

uint32_t heap = 0;
void trackMem(String point){
  uint32_t h = system_get_free_heap_size();
  if(heap > 0){
    Serial.printf("Heap Delta at %s %d %d\n",point.c_str(), h - heap, h);
  }
  heap = h;
}

void setup() {
  Serial.begin(115200);
  for(int i = 0; i < 10; i++){
    Serial.println();
  }
  trackMem("setup start");
  the_io_handler = IOHandler::init();
  the_io_handler->setup();
  the_io_handler->ledRed(true);
  the_io_handler->ledGreen(true);

  the_data_store = DataStore::init();
  the_data_store->initFeeds(io);

  connect_adafruit_io();

  Serial.println();
  Serial.println(io.statusText());

  the_logger = Logging::init();
  the_logger->update();

  the_data_store->afterIOConnect();

  the_http_handler = HTTPHandler::init();
  the_http_handler->setupServer();
  the_http_handler->setupOTA();

  the_io_handler->ledRed(false);
  the_io_handler->ledGreen(false);

  the_logger->log("setup","Started");

  trackMem("setup end");
}


void connect_adafruit_io(){
  Serial.println(F("Connecting to Adafruit IO"));
  WiFi.hostname(HOSTNAME);
  WiFi.setAutoReconnect(true);
  WiFi.mode(WIFI_STA);

  io.connect();
  the_data_store->io_door_action->onMessage(handleDoorActionMessage);

  while(io.status() < AIO_NET_CONNECTED){
    Serial.print(F("Net Status "));
    Serial.println(io.statusText());
    // WiFi.printDiag(Serial);
    delay(500);
  }
  MDNS.begin(HOSTNAME);
}

void handleDoorActionMessage(AdafruitIO_Data *data) {
  if(the_data_store->doorAction(data)){
    the_io_handler->actionDoor(data->value());
  }
}

void read_temperature(){
  if(temperatureLastRead == 0 || (millis() - temperatureLastRead) >= TEMPERATURE_DELAY ){
    ESP.wdtDisable();
    for(int d = 0; d < 5; d++) {
      DS18B20.requestTemperatures(); 
      float temp = DS18B20.getTempCByIndex(0);
      if(temp != 85.0 && temp != (-127.0)){
        the_data_store->updateTemperature(temp);
        break;
      }
      the_data_store->io_garage_temperature_value = "Failed";
      the_logger->log(F("read_temperature"),F("Temperature Reading Failed"));
      delay(1000);
    }
    temperatureLastRead = millis();
    ESP.wdtEnable((uint32_t)0);
  }
}

// #define MILLIS_6_HOURS 60000
// #define MILLIS_6_HOURS 21600000


uint32_t msx = 0;
void loop() {
  if(system_get_free_heap_size() < 1000){
    ESP.restart();
  }
  io.run();
  the_http_handler->update();
  the_io_handler->update();
  the_logger->update();
  read_temperature();

  if(millis() - msx > 10000){
    trackMem("Loop");
    msx = millis();
  }
}