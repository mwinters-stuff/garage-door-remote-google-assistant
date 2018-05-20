
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <AdafruitIO_WiFi.h>
#include <ESP8266mDNS.h>

#include <AdafruitIO_Feed.h>
#include <AdafruitIO_WiFi.h>
#include <OneWire.h>
#include <DallasTemperature.h>


#include "config.h"
#include "datastore.h"
#include "httphandler.h"
#include "iohandler.h"
#include "logging.h"


std::shared_ptr<DataStore> the_data_store;
std::shared_ptr<IOHandler> the_io_handler;
std::shared_ptr<HTTPHandler> the_http_handler;
std::shared_ptr<Logging> the_logger;

AdafruitIO_WiFi io(IO_USERNAME, IO_KEY, WIFI_SSID, WIFI_PASS);

OneWire oneWire(ONE_WIRE_PIN);
DallasTemperature DS18B20(&oneWire);
uint32_t temperatureLastRead;

void handleDoorActionMessage(AdafruitIO_Data *data);

void setup() {
  Serial.begin(115200);
  for(int i = 0; i < 10; i++){
    Serial.println();
  }
  the_io_handler = IOHandler::init();
  the_io_handler->setup();
  the_io_handler->ledRed(true);
  the_io_handler->ledGreen(true);
  // Serial.println("Connecting Wifi");
  // WiFi.begin(WIFI_SSID, WIFI_PASS);
  // while (WiFi.status() != WL_CONNECTED) {
  //   delay(500);
  //   Serial.print(".");
  // }
  // Serial.println();


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

void loop() {
  io.run();
  the_http_handler->update();
  the_io_handler->update();
  the_logger->update();
  read_temperature();

}