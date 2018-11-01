#include <Arduino.h>
// #include <AdafruitIO_Feed.h>
// #include <AdafruitIO_WiFi.h>
// #include <EEPROM.h>
// #define Debug Serial
#include <ESP8266HTTPClient.h>

#include "MQTTHandler.h"
#include "StringConstants.h"

#include <RemoteDebug.h>
extern RemoteDebug Debug;

//#define AIO_SERVER "io.adafruit.com"
//#define AIO_SERVERPORT 8883

MQTTHandler *MQTTHandler::m_instance;


void handleInHomeAreaMessage(AdafruitIO_Data *data) { MQTTHandler::_getInstance()->inHomeMessage(data); }

void handleDoorActionMessage(AdafruitIO_Data *data) { MQTTHandler::_getInstance()->doorAction(data); }

MQTTHandler::MQTTHandler(SettingsFile *settingsFile, ConfigFile *configFile)
    : settingsFile(settingsFile),
      configFile(configFile),
      io(NULL) {
  m_instance = this;
}

void MQTTHandler::initFeeds() {
  Debug.println(F("Init Feeds"));

  if (configFile->io_feed_username.length() > 0 && configFile->io_feed_key.length() > 0) {
    Debug.println(F("Starting Adafruit IO"));
    io = new AdafruitIO_Custom(configFile->io_feed_username.c_str(), configFile->io_feed_key.c_str());

    io_door_action = io->feed(configFile->io_feed_door_action.c_str());
    io_door_position = io->feed(configFile->io_feed_position.c_str());
    io_in_home_area = io->feed(configFile->io_feed_in_home_area.c_str());

    connect();
  } else {
    Debug.println(F("Adafruit IO Not Configured"));
    if (io) {
      delete io;
      delete io_door_action;
      delete io_door_position;
      delete io_in_home_area;
      delete io_temperature;
    }
    io = NULL;
    io_door_action = NULL;
    io_door_position = NULL;
    io_in_home_area = NULL; 
    io_temperature = NULL; 
  }
}

void MQTTHandler::update() {
  if (io) {
    io->run();
  }
}

void MQTTHandler::connect() {
  int8_t ret;

  // Stop if already connected.
  if (!io || io->mqttStatus() < AIO_CONNECTED) {
    return;
  }

  Debug.println(F("Connecting to Adafruit IO... "));
  io->connect();

  uint8_t retries = 5;
  while ((ret = io->mqttStatus()) < AIO_CONNECTED) {  // connect will return 0 for connected
    Debug.println(io->statusText());
    Debug.println(F("Retrying Adafruit IO connection in 5 seconds..."));
    delay(500);  // wait 5 seconds
    retries--;
    if (retries == 0) {
      // basically die and wait for WDT to reset me
      while (1)
        ;
    }
  }

  Debug.println(F("Adafruit IO Connected!"));

  io_in_home_area->onMessage(handleInHomeAreaMessage);
  io_door_action->onMessage(handleDoorActionMessage);

  // io_door_action->get();
  // io_door_position->get();
}

void MQTTHandler::inHomeMessage(AdafruitIO_Data *data) {
  Debug.print(F("Action: Received in home area -> "));
  Debug.print(data->value());

  if(strcmp_P(data->value(), PSTR("entered")) == 0){
    settingsFile->setInHomeArea();
  }else{
    settingsFile->setOutHomeArea();
  }
  sendToInflux("in-home-area",settingsFile->isInHomeArea() ? "true" : "false");
  Debug.print(F("inHomeMessage "));
  Debug.println(settingsFile->isInHomeArea() ? F(" inside geofence") : F(" outside geofence"));
}

void MQTTHandler::doorAction(AdafruitIO_Data *data) {
  settingsFile->setLastDoorAction(data->value());
  Debug.print(F("Action: Received door action -> "));
  Debug.println(data->value());

  if (!settingsFile->isInHomeArea()) {
    Debug.println(F("MQTTHandler Not near home, won't action door!"));
  } else if (doorActionCallback) {
    Debug.println("doorActionCallback");
    doorActionCallback(data->value());
    sendToInflux("door-action",data->value());
  }
}

void MQTTHandler::updateDoorPosition(doorPositions current_door_position, doorPositions _door_position) {
  if (settingsFile->updateDoorPosition(current_door_position, _door_position) && io_door_position) {
    String pos = SettingsFile::doorPositionToString(settingsFile->getCurrentDoorPosition());
    io_door_position->save(pos);
    sendToInflux("door-position",pos);
  }
}

void MQTTHandler::updateDoorPosition(doorPositions current_door_position, doorPositions _door_position, bool _door_moving) {
  if(_door_moving){
    settingsFile->setDoorMoving();
  }else{
    settingsFile->setDoorNotMoving();
  }
  updateDoorPosition(current_door_position, _door_position);
}

void MQTTHandler::toggleLocked() { setLocked(!settingsFile->isLocked()); }

void MQTTHandler::setLocked(bool locked) {
  Debug.print(F("MQTTHandler Setting lock to "));
  if (locked) {
    Debug.println(LOCKED);
    settingsFile->setLocked();
    sendToInflux("door-lock",LOCKED);
  } else {
    Debug.println(UNLOCKED);
    settingsFile->setUnLocked();
    sendToInflux("door-lock",UNLOCKED);
  }
  
}

void MQTTHandler::sendInflux(const String &body){
  if(configFile->influxOk()){
    HTTPClient http;
    String url = configFile->getInfluxUrl();

    Debug.print(F("Sending to influx url: "));
    Debug.print(url);
    Debug.print(F(" body "));
    Debug.println(body);

    http.begin(url);
    
    int result = http.POST(body);
    http.end();
    Debug.print(F("HTTP Result: "));
    last_http_reponse_str = String(result) + String(" - ") + http.errorToString(result);
    Debug.println(last_http_reponse_str);
  }

}

void MQTTHandler::sendToInflux(const String &dataPoint, const String &dataValue){
  if(configFile->influx_measurement.length() > 0 && configFile->influx_door.length() > 0){
    sendInflux(configFile->influx_measurement + F(",door=") + configFile->influx_door + " " + dataPoint + "=\"" + dataValue + "\"");
  }
}

void MQTTHandler::setTemperature(double temperature){
  if(configFile->influx_measurement_temperature.length() > 0 && configFile->influx_temperature_tags.length() > 0){
    sendInflux(configFile->influx_measurement_temperature + F(",") + 
      configFile->influx_temperature_tags + F(" temperature=") + String(temperature,3) );
  }
}