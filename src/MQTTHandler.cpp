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
extern void log_printf(PGM_P fmt_P, ...);

MQTTHandler *MQTTHandler::m_instance;

void doorPositionCallback(char *data, uint16_t len){
  log_printf(PSTR("Received POSITION Message %s "), data);
  MQTTHandler::_getInstance()->doorAction(String(data));
}

void doorLockCallback(char *data, uint16_t len){
  log_printf(PSTR("Received LOCK Message %s"), data);
  MQTTHandler::_getInstance()->setLocked(String(data) == "LOCK");
}

MQTTHandler::MQTTHandler(SettingsFile *settingsFile, ConfigFile *configFile)
    : settingsFile(settingsFile),
      configFile(configFile),
      wifiClient(),
      isConnected(false),
      mqtt(&wifiClient, configFile->mqtt_hostname.c_str(), configFile->mqtt_port, configFile->mqtt_username.c_str(), configFile->mqtt_password.c_str()),
      pub_door_position(&mqtt, configFile->mqtt_feed_door_position.c_str(), MQTT_QOS_1),
      pub_door_locked(&mqtt, configFile->mqtt_feed_door_locked.c_str(), MQTT_QOS_1),
      pub_online(&mqtt,configFile->mqtt_feed_online.c_str(), MQTT_QOS_1),
      pub_temperature(&mqtt, configFile->mqtt_feed_temperature.c_str(), MQTT_QOS_1),
      sub_door_position(&mqtt, configFile->mqtt_feed_door_position.c_str(), MQTT_QOS_1),
      sub_door_locked(&mqtt, configFile->mqtt_feed_door_locked.c_str(), MQTT_QOS_1)
{
  m_instance = this;
  sub_door_position.setCallback(doorPositionCallback);
  sub_door_locked.setCallback(doorLockCallback);
  mqtt.subscribe(&sub_door_position);
  mqtt.subscribe(&sub_door_locked);
}

void MQTTHandler::update() {
  // if (mqtt) {
    connect();
  // }
}

void MQTTHandler::connect() {
  int8_t ret;
  mqtt.processPackets(10);

  // Stop if already connected.
  if (mqtt.connected()) {
    // ping the server to keep the mqtt connection alive
    // NOT required if you are publishing once every KEEPALIVE seconds
    if (millis() - lastSentPing > 30000) {
      lastSentPing = millis();
      if (mqtt.connected()) {
        Debug.print(millis() / 1000);
        Debug.println(F(" Sending Ping"));
        if (!mqtt.ping()) {
          Debug.println(F("Ping Failed, Disconnect"));
          mqtt.disconnect();
          isConnected = false;
        } else {
          pub_online.publish((uint32_t) millis());
          // TODO: report position and locked;
          isConnected = true;
        }
      }
    }
    return;
  }

  if (millis() - reconnectTimeout > 5000) {
    Debug.println(configFile->mqtt_feed_online);
    Debug.print(millis() / 1000);
    reconnectTimeout = millis();
    log_printf(PSTR(" Connecting to MQTT... "));
    if ((ret = mqtt.connect()) != 0) {
      // WiFi.printDiag(Serial);
      log_printf(PSTR("MQTT Connect Failed %s"), mqtt.connectErrorString(ret));
      log_printf(PSTR("Retrying MQTT connection in 5 seconds..."));
      mqtt.disconnect();
      isConnected = false;
      return;
    }
    Debug.print(millis() / 1000);
    log_printf(PSTR(" MQTT Connected!"));
    isConnected = true;

    pub_online.publish((uint32_t)millis());
 
 
    //reportFanSpeed();
  }
}

void MQTTHandler::doorAction(String data) {
  log_printf(PSTR("Action: Received door action -> %s"), data.c_str());
  if(data != settingsFile->getLastDoorAction() && data != String(UNKNOWN_ACTION)){
    settingsFile->setLastDoorAction(data);
    doorActionCallback(data);
  }
}

void MQTTHandler::updateDoorPosition(doorPositions current_door_position, doorPositions _door_position) {
  log_printf(PSTR("updateDoorPosition from %s to %s"), SettingsFile::doorPositionToString(current_door_position).c_str(), SettingsFile::doorPositionToString(_door_position).c_str());

  settingsFile->updateDoorPosition(current_door_position, _door_position);
  String pos = SettingsFile::doorPositionToString(settingsFile->getCurrentDoorPosition());
  pub_door_position.publish(pos.c_str());
}

void MQTTHandler::updateDoorPosition(doorPositions current_door_position, doorPositions _door_position, bool _door_moving) {
  if(_door_moving){
    settingsFile->setDoorMoving();
  }else{
    settingsFile->setDoorNotMoving();
  }
  updateDoorPosition(current_door_position, _door_position);
}

void MQTTHandler::toggleLocked() { 
  setLocked(!settingsFile->isLocked()); 
}

void MQTTHandler::setLocked(bool locked) {
  if(locked != settingsFile->isLocked()){
    if (locked) {
      log_printf(PSTR("MQTTHandler Setting lock to %s"), LOCK);
      settingsFile->setLocked();
      pub_door_locked.publish("LOCK");
    } else {
      log_printf(PSTR("MQTTHandler Setting lock to %s"), UNLOCK);
      pub_door_locked.publish("UNLOCK");
      settingsFile->setUnLocked();
    }
  }
}


void MQTTHandler::setTemperature(double temperature){
  pub_temperature.publish(temperature,3);
}