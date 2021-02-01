#include <Arduino.h>
#include <ESP8266DebounceSwitch.h>
#include "IOHandler.h"
#include "MQTTHandler.h"
#include "StringConstants.h"

#include <RemoteDebug.h>
extern RemoteDebug Debug;

IOHandler *IOHandler::m_instance;
extern void log_printf(PGM_P fmt_P, ...);

IOHandler::IOHandler(MQTTHandler *mqttHandler, SettingsFile *settingsFile, ConfigFile *configFile):
  mqttHandler(mqttHandler),
  settingsFile(settingsFile),
  configFile(configFile),
  greenMillisFlash(0),
  redMillisFlash(0),
  oneWire(ONE_WIRE_PIN),
  DS18B20(&oneWire),
  sonic(SONAR_TRIGGER, SONAR_ECHO),
  sonarReadMillis(0),
  sonic_reading_commanded(false),
  sonic_read_interval(SONAR_READ_INTERVAL_WAITING)
{
  m_instance = this;

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(RELAY, OUTPUT);


  digitalWrite(LED_RED,LED_ON);
  digitalWrite(LED_GREEN,LED_ON);
  digitalWrite(LED_BUILTIN,HIGH);

#ifdef OPEN_CLOSE_BUTTON
  switches.addButtonPin(OPEN_CLOSE_BUTTON, [&](uint8_t pin, uint32_t msPressed){
    onOpenCloseButtonPress(pin, msPressed);
  },true);
#endif

  mqttHandler->doorCommandCallback = [&](String command){
    doorCommand(command);
  };
  DS18B20.begin();
  Debug.print(F("DS18B20 Init Found "));
  Debug.println(DS18B20.getDS18Count());
  temperatureLastRead = millis();
}

void IOHandler::ledGreen(bool on){
  digitalWrite(LED_GREEN,on ? LED_ON : LED_OFF);
}

void IOHandler::ledRed(bool on){
  digitalWrite(LED_RED,on ? LED_ON : LED_OFF);
}

void IOHandler::update(){
  if(!mqttHandler->isMQTTConnected()){
    return;
  }

  readTemperature();
  switches.update();
  readSonar();

}

void IOHandler::readSonar(){
  if(millis() - sonarReadMillis > sonic_read_interval){
   
    sonic_last_distance = sonic.measureDistanceCm(settingsFile->getTemperature());
    mqttHandler->setSonicReading(sonic_last_distance);
    
    String log = String(F("Sonic CM ")) + String(sonic_last_distance);
    if(sonarReadMillis == 0){ 
      // first run update mqtt.
      mqttHandler->setClosed(sonic_last_distance < 0 || sonic_last_distance > configFile->distance_open);
    }


    if(sonic_last_distance < 0.0 || sonic_last_distance > configFile->distance_open){
      if(!settingsFile->isClosed()){
        log += F(" setting CLOSED");
        mqttHandler->setClosed(true);
      }
    }else{
      if(!settingsFile->isOpen()){
        log += F(" setting OPEN");
        mqttHandler->setClosed(false);
      }
    }
    log_printf(log.c_str());
    sonarReadMillis = millis();
    if(sonic_reading_commanded){
      if(millis() - sonic_read_commanded_start > SONAR_READ_COMMANDED_FOR){
        log_printf(PSTR("No Longer reading sonic for commanded interval"));
        sonic_reading_commanded = false;
        sonic_read_interval = SONAR_READ_INTERVAL_WAITING;
      }
    }
  }

}


void IOHandler::doorCommand(String command){
  String logString = String(F("DoorCommand ")) + command;
  if(!settingsFile->isLocked()){
    if(command.compareTo(FORCE) == 0){
      logString += F(": Force Door Requested");
      redMillisFlash = millis();
      toggleRelay();
    }

    if (command.compareTo(OPEN) == 0) {
      if(settingsFile->isClosed()){
        logString += F(": Open Requested");
        greenMillisFlash = 10000;
        ledGreen(true);
        toggleRelay();
      }else{
        logString += F(": Already Open");
        redMillisFlash = millis() + 1000;
        ledRed(true);
      }
    }
    if (command.compareTo(CLOSE) == 0) {
      if(settingsFile->isOpen()){
        logString += F(": Close Requested");
        greenMillisFlash = 10000;
        ledGreen(true);
        toggleRelay();
      }else{
        logString += F(": Already Closed");
        redMillisFlash = millis() + 1000;
        ledRed(true);
      }
    }
  }else{
    ledRed(true);
    logString += LOCK;
  }
  log_printf(logString.c_str());
}

#ifdef OPEN_CLOSE_BUTTON
void IOHandler::onOpenCloseButtonPress(uint8_t pin, uint32_t msPressed){
  String logString;
  if(!settingsFile->isLocked() && msPressed < 1000){
    logString += F("Toggle ");
    greenMillisFlash = 10000;
    ledGreen(true);
    toggleRelay();
  }else if(msPressed > 5000){
    logString += F("Changing to ");
    logString += (settingsFile->isLocked() ? LOCK : UNLOCK);     
    mqttHandler->toggleLocked();
    ledRed(settingsFile->isLocked());
  }else{
    logString += F("IS ");
    logString += (settingsFile->isLocked() ? LOCK : UNLOCK);     
    
  }
  log_printf(PSTR("onOpenCloseButtonPress %d %s"), msPressed, logString.c_str());
}
#endif

void IOHandler::toggleRelay(){
  sonic_reading_commanded = true;
  sonic_read_interval = SONAR_READ_INTERVAL_COMMANDED;
  sonic_read_commanded_start = millis();


  log_printf(PSTR("Toggle Relay"));
  ledRed(true);
  digitalWrite(RELAY,RELAY_CLOSED);
  delay(RELAY_TOGGLE_TIME);
  ledRed(false);
  digitalWrite(RELAY,RELAY_OPEN);

}

void IOHandler::readTemperature(){
  if(DS18B20.getDS18Count() > 0){
    if(temperatureLastRead == 0 || (millis() - temperatureLastRead) >= configFile->update_interval_ms ){
      ESP.wdtDisable();
      DS18B20.requestTemperatures(); 
      float temp = DS18B20.getTempCByIndex(0);
      if(temp != 85.0 && temp != (-127.0)){
        Debug.print(F("Temperature -> "));
        Debug.println(temp);
        settingsFile->setTemperature(temp);
        mqttHandler->setTemperature(temp);
      }else{
        Debug.println(F("Temperature Reading Failed"));
      }
      temperatureLastRead = millis();
      ESP.wdtEnable((uint32_t)0);
    }
  }
}
