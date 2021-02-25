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

extern void log_printf(PGM_P fmt_P, ...);

MQTTHandler *MQTTHandler::m_instance;

void doorPositionCommandCallback(char *data, uint16_t len){
  log_printf((String(PSTR("Received POSITION COMMAND Message ")) + String(data)).c_str());
  MQTTHandler::_getInstance()->doorPositionCommand(String(data));
}

void doorLockCommandCallback(char *data, uint16_t len){
  log_printf((String(PSTR("Received LOCK COMMAND Message ")) + String(data)).c_str());
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
      pub_sonic_cm(&mqtt, configFile->mqtt_feed_sonic_cm.c_str(), MQTT_QOS_1),
      sub_door_position_command(&mqtt, configFile->mqtt_feed_door_position_command.c_str(), MQTT_QOS_1),
      sub_door_locked_command(&mqtt, configFile->mqtt_feed_door_locked_command.c_str(), MQTT_QOS_1)
{
  m_instance = this;
  sub_door_position_command.setCallback(doorPositionCommandCallback);
  sub_door_locked_command.setCallback(doorLockCommandCallback);
  mqtt.subscribe(&sub_door_position_command);
  mqtt.subscribe(&sub_door_locked_command);
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
      log_printf((String(PSTR("MQTT Connect Failed ")) +  mqtt.connectErrorString(ret)).c_str());
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

void MQTTHandler::doorPositionCommand(String data) {
  log_printf(String(PSTR("Action: Received door action ->") + data).c_str());
  bool open = data == "OPEN";
  if(open != settingsFile->isOpen()){
    doorCommandCallback(data);
  }
}

void MQTTHandler::setClosed(bool closed) {
  settingsFile->setClosed(closed);
  pub_door_position.publish(String(closed ? CLOSED : OPEN).c_str());
}

void MQTTHandler::setSonicReading(int cm) {
  pub_sonic_cm.publish(cm);
}


void MQTTHandler::toggleLocked() { 
  setLocked(!settingsFile->isLocked()); 
}

void MQTTHandler::setLocked(bool locked) {
  if(locked != settingsFile->isLocked()){
    if (locked) {
      log_printf(PSTR("MQTTHandler Setting lock to %s"), LOCKED);
      settingsFile->setLocked();
      pub_door_locked.publish(String(LOCKED).c_str());
    } else {
      log_printf(PSTR("MQTTHandler Setting lock to %s"), UNLOCKED);
      pub_door_locked.publish(String(UNLOCKED).c_str());
      settingsFile->setUnLocked();
    }
  }
}


void MQTTHandler::setTemperature(double temperature){
  pub_temperature.publish(temperature,3);
}