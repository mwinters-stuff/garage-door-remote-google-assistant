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

#define HOMEASSISTANT_STATUS_TOPIC F("homeassistant/status")
#define HOMEASSISTANT_STATUS_ONLINE F("online")


MQTTHandler::MQTTHandler(std::shared_ptr<SettingsFile> settingsFile,std::shared_ptr<ConfigFile> configFile)
    : settingsFile(settingsFile),
      configFile(configFile),
      wifiClient(),
      isConnected(false),
      mqttClient()
{
  mqttClient.onConnect([this](bool sessionPresent) { onMQTTConnect(sessionPresent); });
  mqttClient.onDisconnect([this](AsyncMqttClientDisconnectReason reason) { onMQTTDisconnect(reason); });

    mqttClient.onMessage([this, configFile](char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
    Debug.printf_P(PSTR("MQTT Received Message %s %d %d %d\n"), topic, len, index, total);
    if (String(topic) == HOMEASSISTANT_STATUS_TOPIC && String(payload).startsWith(HOMEASSISTANT_STATUS_ONLINE)) {
      publishHomeAssistantDetect();
    }
    auto data = String(payload).substring(0,len);
    if (String(topic).endsWith(configFile->mqtt_feed_door_position_command)) {
      Debug.printf_P(PSTR("Received POSITION COMMAND Message %s\n"), data);
      doorPositionCommand(String(data));
    }
    if (String(topic).endsWith(configFile->mqtt_feed_door_locked_command)) {
      Debug.printf_P(PSTR("Received LOCK COMMAND Message %s\n"), data);
      setLocked(String(data) == LOCK);
    }
  });

  mqttClient.onPublish([this](uint16_t packetId) { Debug.printf_P(PSTR("MQTT Publish Ack: %" PRIu16 "\n"), packetId); });

  mqttClient.onSubscribe([this](uint16_t packetId, uint8_t qos) { Debug.printf_P(PSTR("MQTT Subscribe Ack: %" PRIu16 " %d\n"), packetId, qos); });

  mqttClient.onUnsubscribe([this](uint16_t packetId) { Debug.printf_P(PSTR("MQTT UnSubscribe Ack: %" PRIu16 "\n"), packetId); });
}


void MQTTHandler::connectToMQTT() {
  Debug.println(F("Connecting to MQTT..."));

  
  if(!configFile->mqtt_username.isEmpty() && !configFile->mqtt_password.isEmpty()){
    mqttClient.setCredentials(configFile->mqtt_username.c_str(), configFile->mqtt_password.c_str());
  }

  mqttClient.setServer(configFile->mqtt_hostname.c_str(), configFile->mqtt_port);
 
  willTopic = configFile->mqtt_device_id + "/" + configFile->mqtt_feed_online;
  
  mqttClient.setWill(willTopic.c_str(), 0, true, "OFFLINE", 7);
  mqttClient.connect();
}

void MQTTHandler::onMQTTConnect(bool sessionPresent) {
  Debug.println(F("Connected to MQTT..."));

  publishValue(configFile->mqtt_feed_online, ONLINE);
  publishValue(configFile->mqtt_feed_door_locked, settingsFile->isLocked() ? LOCKED : UNLOCKED);
  publishValue(configFile->mqtt_feed_door_position, settingsFile->isClosed() ? CLOSED : OPEN);
  
  mqttClient.subscribe(String(HOMEASSISTANT_STATUS_TOPIC).c_str(), 0);
  
  mqttClient.subscribe(String(configFile->mqtt_device_id + "/" + configFile->mqtt_feed_door_locked_command).c_str(), 0);
  mqttClient.subscribe(String(configFile->mqtt_device_id + "/" + configFile->mqtt_feed_door_position_command).c_str(), 0);

  //publishConfig();
  timeConnected = millis();
  isConnected = true;

  if(connectedCallback){
    connectedCallback();
  }
}

void MQTTHandler::stop() {
  dontReconnect = true;
  mqttReconnectTimer.detach();
  mqttClient.disconnect();
}

void MQTTHandler::onMQTTDisconnect(AsyncMqttClientDisconnectReason reason) {
  String r = F("unknown");
  isConnected = false;
  switch (reason) {
    case AsyncMqttClientDisconnectReason::TCP_DISCONNECTED:
      r = F("TCP_DISCONNECTED ");
      break;
    case AsyncMqttClientDisconnectReason::MQTT_UNACCEPTABLE_PROTOCOL_VERSION:
      r = F("MQTT_UNACCEPTABLE_PROTOCOL_VERSION ");
      break;
    case AsyncMqttClientDisconnectReason::MQTT_IDENTIFIER_REJECTED:
      r = F("MQTT_IDENTIFIER_REJECTED ");
      break;
    case AsyncMqttClientDisconnectReason::MQTT_SERVER_UNAVAILABLE:
      r = F("MQTT_SERVER_UNAVAILABLE ");
      break;
    case AsyncMqttClientDisconnectReason::MQTT_MALFORMED_CREDENTIALS:
      r = F("MQTT_MALFORMED_CREDENTIALS ");
      break;
    case AsyncMqttClientDisconnectReason::MQTT_NOT_AUTHORIZED:
      r = F("MQTT_NOT_AUTHORIZED ");
      break;
    case AsyncMqttClientDisconnectReason::ESP8266_NOT_ENOUGH_SPACE:
      r = F("ESP8266_NOT_ENOUGH_SPACE ");
      break;
    case AsyncMqttClientDisconnectReason::TLS_BAD_FINGERPRINT:
      r = F("TLS_BAD_FINGERPRINT");
      break;
  }

  Debug.printf("Disconnected from MQTT: Reason: %s\n\n", r.c_str());

  if (WiFi.isConnected() && !dontReconnect) {
    mqttReconnectTimer.once(2, [this]() { connectToMQTT(); });
  }
}

void MQTTHandler::onWifiDisconnect() { mqttReconnectTimer.detach(); }

void MQTTHandler::publishValue(const String &topic, const String &value) { 
  Debug.printf_P(PSTR("Publish: \"%s\" -> \"%s\"\n"),(configFile->mqtt_device_id + "/" + topic).c_str(), value.c_str() );
  mqttClient.publish((configFile->mqtt_device_id + "/" + topic).c_str(), 0, true, value.c_str()); 
}


void MQTTHandler::update() {
  if (timeConnected > 0 && millis() - timeConnected > 2000) {
    timeConnected = 0;
    publishHomeAssistantDetect();
  }
}

void MQTTHandler::doorPositionCommand(String data) {
  Debug.printf_P(PSTR("Action: Received door action -> %s\n"),data);
  bool open = data.compareTo(OPEN) == 0;
  if(open != settingsFile->isOpen()){
    doorCommandCallback(data);
  }
}

void MQTTHandler::setClosed(bool closed) {
  settingsFile->setClosed(closed);
  publishValue(configFile->mqtt_feed_door_position,closed ? CLOSED : OPEN );
}

void MQTTHandler::setClosing() {
  publishValue(configFile->mqtt_feed_door_position,CLOSING );
}

void MQTTHandler::setOpening() {
  publishValue(configFile->mqtt_feed_door_position,OPENING );
}


void MQTTHandler::setSonicReading(int cm) {
  publishValue(configFile->mqtt_feed_sonic_cm,String(cm) );
}


void MQTTHandler::toggleLocked() { 
  setLocked(!settingsFile->isLocked()); 
}

void MQTTHandler::setLocked(bool locked) {
  if(locked != settingsFile->isLocked()){
    if (locked) {
      Debug.printf_P(PSTR("MQTTHandler Setting lock to %s\n"), LOCKED);
      settingsFile->setLocked();
      publishValue(configFile->mqtt_feed_door_locked,LOCKED );
    } else {
      Debug.printf_P(PSTR("MQTTHandler Setting lock to %s\n"), UNLOCKED);
      publishValue(configFile->mqtt_feed_door_locked,UNLOCKED );
      settingsFile->setUnLocked();
    }
  }
}


void MQTTHandler::setTemperature(double temperature){
  publishValue(configFile->mqtt_feed_temperature,String(temperature,2) );
  publishValue(wifiTopic, String(WiFi.RSSI()));
}

void MQTTHandler::publishConfig(const String &uid, const String& kind, const String &config){
  String deviceId = configFile->mqtt_device_id;
  char topic[256];

  snprintf_P(topic, 256, PSTR("homeassistant/%s/%s/%s-%s/config"), kind.c_str(), deviceId.c_str(),deviceId.c_str(), uid.c_str());
  Debug.println(topic);
  Debug.println(config);
  mqttClient.publish(topic, 0, true, config.c_str());

}

void MQTTHandler::publishDeviceConfig(const String &uid, const String& kind, const String &name, const String &mqttstat, const String &uom, const String &deviceClass, const String &icon){
  String deviceId = configFile->mqtt_device_id;

  publishConfig(uid, kind,F(R"(
  {
    "uniq_id": ")") + uid + F(R"(",
    "name": ")") + name + F(R"(",
    "stat_t": ")") + mqttstat + F(R"(",
    "unit_of_meas": ")") + uom + F(R"(",
    "sug_dsp_prc": 2,
    "dev_cla": ")") + deviceClass + F(R"(",
    "state_class": "measurement",
    "icon":")") + icon + F(R"(",
    "dev": {
       "ids": ")") + deviceId + F(R"("
    },
    "avty": {
      "t":")") + willTopic + F(R"(",
      "pl_avail": ")") + ONLINE + F(R"(",
      "pl_not_avail": ")") + OFFLINE + F(R"("
    }
  }
  )"));

}

void MQTTHandler::publishHomeAssistantDetect() {
  Debug.println(F("Publish HomeAssistant Connect..."));
  String deviceId = configFile->mqtt_device_id;
  String deviceName = configFile->mqtt_device_name;
  
  publishConfig(deviceId + F("-garage-door-position"),F("cover"), F(R"(
  {
    "unique_id": "garage-door-position",
    "name": "Position",
    "command_topic": ")") + configFile->mqtt_device_id + "/" + configFile->mqtt_feed_door_position_command + F(R"(",
    "state_topic": ")") + configFile->mqtt_device_id + "/" + configFile->mqtt_feed_door_position + F(R"(",
    "state_open": ")") + OPEN + F(R"(",
    "state_closed": ")") + CLOSE + F(R"(",
    "state_opening": ")") + CLOSING + F(R"(",
    "state_closing": ")") + OPENING + F(R"(",
    "payload_open": ")") + OPEN + F(R"(",
    "payload_close": ")") + CLOSE + F(R"(",
    "icon": "mdi:garage",
    "dev_cla": "garage",
    "dev": {
      "configuration_url": "http://)") + WiFi.getHostname() + F(R"(",
      "ids": ")") + deviceId + F(R"(",
      "sa": "Garage",
      "name": ")") + deviceName + F(R"(",
      "cns": [
        [
          "mac",
          ")") + WiFi.macAddress() + F(R"("
        ]
      ]
    },
    "avty": {
      "t":")") + willTopic + F(R"(",
      "pl_avail": ")") + ONLINE + F(R"(",
      "pl_not_avail": ")") + OFFLINE + F(R"("
    }
  }
  )"));

  publishConfig(deviceId + F("-garage-door-lock"),F("lock"), F(R"(
  {
    "unique_id": "garage-door-lock",
    "name": "Lock",
    "command_topic": ")") + configFile->mqtt_device_id + "/" + configFile->mqtt_feed_door_locked_command + F(R"(",
    "state_topic": ")") + configFile->mqtt_device_id + "/" + configFile->mqtt_feed_door_locked + F(R"(",
    "state_lock": ")") + LOCKED + F(R"(",
    "state_unlocked": ")") + UNLOCKED + F(R"(",
    "payload_lock": ")") + LOCK + F(R"(",
    "payload_unlock": ")") + UNLOCK + F(R"(",
    "icon": "mdi:key",
    "dev": {
      "ids": ")") + deviceId + F(R"("
    },
    "avty": {
      "t":")") + willTopic + F(R"(",
      "pl_avail": ")") + ONLINE + F(R"(",
      "pl_not_avail": ")") + OFFLINE + F(R"("
    }
  }
  )"));

  publishDeviceConfig(deviceId + F("-temperature"),
    F("sensor"),
    F("Temperature"),
    configFile->mqtt_device_id + "/" + configFile->mqtt_feed_temperature,
    "°C",
    F("temperature"),
    F("mdi:thermometer")
  );

  publishDeviceConfig(deviceId + F("-sonic_cm"),
    F("sensor"),
    F("Sonic Distance"),
    configFile->mqtt_device_id + "/" +  configFile->mqtt_feed_sonic_cm,
    "cm",
    F("distance"),
    F("mdi:signal-distance-variant")
  );

  wifiTopic = F("wifi-rssi");
  publishDeviceConfig(deviceId + F("-rssi"),
    F("sensor"),
    F("WiFi RSSI"),
    configFile->mqtt_device_id + "/" + wifiTopic,
    "dB",
    F("signal_strength"),
    F("mdi:wifi")
  );
}