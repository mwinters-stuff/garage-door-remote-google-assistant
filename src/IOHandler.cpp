#include <Arduino.h>
#include <ESP8266DebounceSwitch.h>
#include "IOHandler.h"
#include "MQTTHandler.h"
#include "StringConstants.h"

#include <RemoteDebug.h>
extern RemoteDebug Debug;

// std::shared_ptr<IOHandler> IOHandler::m_instance;
extern void log_printf(PGM_P fmt_P, ...);

#define SONIC_NO_READING 38000.0

IOHandler::IOHandler(std::shared_ptr<MQTTHandler> mqttHandler, std::shared_ptr<SettingsFile> settingsFile, std::shared_ptr<ConfigFile> configFile):
  mqttHandler(mqttHandler),
  settingsFile(settingsFile),
  configFile(configFile),
  oneWire(ONE_WIRE_PIN),
  DS18B20(&oneWire),
  sonic(SONAR_TRIGGER, SONAR_ECHO,20,500),
  sonarReadMillis(0),
  sonic_reading_commanded(false),
  sonic_read_interval(SONAR_READ_INTERVAL_WAITING)
{

  doInit = false;
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

  mqttHandler->connectedCallback = [&](){
    doInit = true;
  };
}

void IOHandler::init(){
  Debug.println(F("IOHandler::Init"));
  sonic.begin();

  int tryDown = 10;
  do{
    delay(100);
    DS18B20.begin();
    Debug.print(F("DS18B20 Init Found "));
    Debug.println(DS18B20.getDS18Count());
  }while(tryDown-- > 0 && DS18B20.getDS18Count() == 0);
  
  doInit = false;
}

void IOHandler::ledGreen(bool on){
  digitalWrite(LED_GREEN,on ? LED_ON : LED_OFF);
}

void IOHandler::ledRed(bool on){
  digitalWrite(LED_RED,on ? LED_ON : LED_OFF);
}

void IOHandler::update(){
  switches.update();
#ifdef TEST
  sonic_read_interval = 500;
#endif
}

void IOHandler::setupTimers(){
  ticker_read_sonic.detach();
  ticker_read_sonic.attach_ms_scheduled(sonic_read_interval,[&](){
    readSonar();
  });

  ticker_read_temperature.detach();
  ticker_read_temperature.attach_ms_scheduled(configFile->update_interval_ms, [&]{
    readTemperature();
  });
}

void IOHandler::readSonar(){
  sonic.setTemperature(settingsFile->getTemperature());
  noInterrupts();
  sonic_last_distance = SONIC_NO_READING;
  float readingssum = 0.0f;
  float readings_count = 0;
  for(int i = 0; i < 5; i++){
      float r = sonic.getDistance();
      // Debug.print("R: ");
      // Debug.print(r);
      // Debug.println();
      
      if(r != SONIC_NO_READING){
        readingssum += r;
        ++readings_count;
      }
  }
  interrupts();
  if(readings_count > 0){
    sonic_last_distance = readingssum / readings_count;
  }
  mqttHandler->setSonicReading(sonic_last_distance == SONIC_NO_READING ? 0 : sonic_last_distance);
  
  Debug.print(F("Distance CM "));
  Debug.print(sonic_last_distance);
  Debug.print(F(" Min "));
  Debug.print(configFile->open_distance_min);
  Debug.print(F(" Max "));
  Debug.print(configFile->open_distance_max);

  String log = String(F("Sonic CM ")) + String(sonic_last_distance);
  Debug.print(F(" Open "));
  bool within_open = (sonic_last_distance >= configFile->open_distance_min && sonic_last_distance <= configFile->open_distance_max);
  Debug.print(within_open ? F("TRUE"):F("FALSE"));
  bool is_closed = sonic_last_distance == SONIC_NO_READING || !within_open;
  Debug.print(F(" Closed "));
  Debug.println(is_closed ? F("TRUE"):F("FALSE"));
  if(sonarReadMillis == 0){ 
    // first run update mqtt.
    mqttHandler->setClosed(is_closed);
  }

  if(is_closed){
    ledRed(true);
    ledGreen(false);
    if(!settingsFile->isClosed()){
      log += F(" setting CLOSED");
      mqttHandler->setClosed(true);
    }
  }else{
    ledRed(false);
    ledGreen(true);
    if(!settingsFile->isOpen()){
      log += F(" setting OPEN");
      mqttHandler->setClosed(false);
    }
  }
  log_printf(log.c_str());
  sonarReadMillis = millis();
  #ifndef TEST
  if(sonic_reading_commanded){
    if(millis() - sonic_read_commanded_start > SONAR_READ_COMMANDED_FOR){
      log_printf(PSTR("No Longer reading sonic for commanded interval"));
      sonic_reading_commanded = false;
      sonic_read_interval = SONAR_READ_INTERVAL_WAITING;

      ticker_read_sonic.detach();
      ticker_read_sonic.attach_ms_scheduled(sonic_read_interval,[&](){
        readSonar();
      });

    }
  }
  #endif
}


void IOHandler::doorCommand(String command){
  String logString = String(F("DoorCommand ")) + command;
  if(!settingsFile->isLocked()){
    if(command.compareTo(FORCE) == 0){
      logString += F(": Force Door Requested");
      
      toggleRelay();
    }

    if (command.compareTo(OPEN) == 0) {
      if(settingsFile->isClosed()){
        logString += F(": Open Requested");
        ledGreen(true);
        toggleRelay();
        mqttHandler->setOpening();
      }else{
        logString += F(": Already Open");
        
        ledRed(true);
      }
    }
    if (command.compareTo(CLOSE) == 0) {
      if(settingsFile->isOpen()){
        logString += F(": Close Requested");
        ledGreen(true);
        toggleRelay();
        mqttHandler->setClosing();
      }else{
        logString += F(": Already Closed");
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
    if(settingsFile->isClosed()){
      mqttHandler->setOpening();
    }else if(settingsFile->isOpen()){
      mqttHandler->setClosing();
    }
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

  ticker_read_sonic.detach();
  ticker_read_sonic.attach_ms_scheduled(sonic_read_interval,[&](){
    readSonar();
  });


  log_printf(PSTR("Toggle Relay"));
  ledRed(true);
  digitalWrite(RELAY,RELAY_CLOSED);
  ticker_relay.attach_ms(RELAY_TOGGLE_TIME, [&](){
    ledRed(false);
    digitalWrite(RELAY,RELAY_OPEN);
  });
}

void IOHandler::readTemperature(){
  if(DS18B20.getDS18Count() > 0){
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
    ESP.wdtEnable((uint32_t)0);
  }
}
