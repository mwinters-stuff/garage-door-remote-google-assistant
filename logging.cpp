#include <Arduino.h>
#include "ArduinoJson.hpp"
#include "logging.h"
#include "config.h"

// Logging to a thingsboard on a local network.

Logging* Logging::m_instance;


#ifdef MQTTLOGGING
Logging::Logging(): client(), mqtt(&client,LOGGING_SERVER, LOGGING_SERVER_PORT, LOGGING_ACCESS_TOKEN),
  telemetry(&mqtt, "v1/devices/me/telemetry", MQTT_QOS_1), lastPing(0){

}
#else
Logging::Logging(){

}
#endif

void Logging::log(String from, String text){
  Serial.printf("%s: %s\n",from.c_str(), text.c_str() );
  
#ifdef MQTTLOGGING
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["from"] = from;
  root["message"] = text;
  String data;

  root.printTo(data);
  telemetry.publish(data.c_str());
#endif  
}

void Logging::update(){
#ifdef MQTTLOGGING
  mqtt_connect();
  mqtt.processPackets(1);
  if(millis() - lastPing > 10000){
    mqtt.ping();
    lastPing = millis();
  }
#endif  
}

#ifdef MQTTLOGGING
void Logging::mqtt_connect(){
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println("MQTT Connected!");
}

#endif

Logging* Logging::init(){
  m_instance = new Logging();
  return m_instance;
}

Logging* Logging::get(){
  return m_instance;
}