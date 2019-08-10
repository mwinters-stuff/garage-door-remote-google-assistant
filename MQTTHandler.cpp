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

void doorPositionCallback(char *data, uint16_t len){
  Debug.print(F("Received POSITION Message "));
  Debug.println(data);
  MQTTHandler::_getInstance()->doorAction(String(data));
}

void doorLockCallback(char *data, uint16_t len){
  Debug.print(F("Received LOCK Message "));
  Debug.println(data);
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
    Debug.print(F(" Connecting to MQTT... "));
    if ((ret = mqtt.connect()) != 0) {
      // WiFi.printDiag(Serial);
      Debug.println(mqtt.connectErrorString(ret));
      Debug.println(F("Retrying MQTT connection in 5 seconds..."));
      mqtt.disconnect();
      isConnected = false;
      return;
    }
    Debug.print(millis() / 1000);
    Debug.println(F(" MQTT Connected!"));
    isConnected = true;

    pub_online.publish((uint32_t)millis());
 
 
    //reportFanSpeed();
  }
}

void MQTTHandler::doorAction(String data) {
  if(data != settingsFile->getLastDoorAction()){
    settingsFile->setLastDoorAction(data);
    Debug.print(F("Action: Received door action -> "));
    Debug.println(data);

    Debug.println(F("doorActionCallback"));
    doorActionCallback(data);
  }
  // sendToInflux("door-action",data);
}

void MQTTHandler::updateDoorPosition(doorPositions current_door_position, doorPositions _door_position) {
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
    Debug.print(F("MQTTHandler Setting lock to "));
    if (locked) {
      Debug.println(LOCK);
      settingsFile->setLocked();
      pub_door_locked.publish("LOCK");
    } else {
      Debug.println(UNLOCK);
      pub_door_locked.publish("UNLOCK");
      settingsFile->setUnLocked();
    }
  }
}


void MQTTHandler::setTemperature(double temperature){
  pub_temperature.publish(temperature,3);
}