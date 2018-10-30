#include <Arduino.h>
#include <ESP8266DebounceSwitch.h>
#include "IOHandler.h"
#include "MQTTHandler.h"
#include "StringConstants.h"

#include <RemoteDebug.h>
extern RemoteDebug Debug;

IOHandler *IOHandler::m_instance;

IOHandler::IOHandler(MQTTHandler *mqttHandler, SettingsFile *settingsFile, ConfigFile *configFile):
  mqttHandler(mqttHandler),
  settingsFile(settingsFile),
  configFile(configFile),
  greenMillisFlash(0),
  redMillisFlash(0),
  oneWire(ONE_WIRE_PIN),
  DS18B20(&oneWire),
  firstLoop(true)
{
  m_instance = this;

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(RELAY, OUTPUT);
  // pinMode(OPEN_CLOSE_BUTTON,INPUT_PULLUP);
  pinMode(SWITCH_OPEN,INPUT_PULLUP);
  pinMode(SWITCH_CLOSED,INPUT_PULLUP);

  digitalWrite(LED_RED,LED_ON);
  digitalWrite(LED_GREEN,LED_ON);
  digitalWrite(LED_BUILTIN,HIGH);

  switches.addButtonPin(OPEN_CLOSE_BUTTON, [&](uint8_t pin, uint32_t msPressed){
    onOpenCloseButtonPress(pin, msPressed);
  },true);

  mqttHandler->doorActionCallback = [&](String action){
    setDoorAction(action);
  };
  DS18B20.begin();
  temperatureLastRead = millis();
}

void IOHandler::_switchCallback(uint8_t pin, bool closed){
  if(pin == SWITCH_OPEN){
    IOHandler::_getInstance()->onOpenSwitchCallback(closed);
  }else if(pin == SWITCH_CLOSED){
    IOHandler::_getInstance()->onClosedSwitchCallback(closed);
  }
}

void IOHandler::ledGreen(bool on){
  digitalWrite(LED_GREEN,on ? LED_ON : LED_OFF);
}

void IOHandler::ledRed(bool on){
  digitalWrite(LED_RED,on ? LED_ON : LED_OFF);
}

void IOHandler::update(){
  if(firstLoop){
    firstLoop = false;
    doorPositions door_position = dpStartup;
    if(digitalRead(SWITCH_OPEN) == LOW && digitalRead(SWITCH_CLOSED) == HIGH){
      // door open
      Debug.println("Inital OPEN");
      door_position = dpOpen;
    }else if(digitalRead(SWITCH_CLOSED) == LOW && digitalRead(SWITCH_OPEN) == HIGH){
      Debug.println("Inital CLOSED");
      door_position = dpClosed;
    } else {
      Debug.println("Inital Unknown");
    }
    mqttHandler->updateDoorPosition(dpUnknown, door_position, false);
    switches.addSwitchPin(SWITCH_OPEN, digitalRead(SWITCH_OPEN) == HIGH, _switchCallback);
    switches.addSwitchPin(SWITCH_CLOSED, digitalRead(SWITCH_CLOSED) == HIGH, _switchCallback);
  }else{
    readTemperature();
    switches.update();
    if(doorAction.length() > 0){
      actionDoor(doorAction);
      doorAction = "";
    }

    doorPositions door_position = settingsFile->getCurrentDoorPosition();

    if(door_position == dpOpen || door_position == dpClosed){
      if(millis() - greenMillisFlash >= WAITING_TIME_OFF && digitalRead(LED_GREEN) == LED_OFF){
        ledGreen(true);
        greenMillisFlash = millis();
      }
      if(millis() - greenMillisFlash >= FLASH_TIME_ON && digitalRead(LED_GREEN) == LED_ON){
        ledGreen(false);
        greenMillisFlash = millis();
      }
    }


    if(door_position == dpClosedToOpen || door_position == dpOpenToClosed){
      if(millis() - greenMillisFlash >= MOVING_FLASH_TIME_OFF && digitalRead(LED_GREEN) == LED_OFF){
        ledGreen(true);
        greenMillisFlash = millis();
      }
      if(millis() - greenMillisFlash >= FLASH_TIME_ON && digitalRead(LED_GREEN) == LED_ON){
        ledGreen(false);
        greenMillisFlash = millis();
      }
    }

    if(door_position == dpManualClosedToOpen || door_position == dpManualOpenToClosed){
      if(millis() - greenMillisFlash>= MANUAL_FLASH_TIME_OFF && digitalRead(LED_GREEN) == LED_OFF){
        ledGreen(true);
        greenMillisFlash = millis();
      }
      if(millis() - greenMillisFlash >= FLASH_TIME_ON && digitalRead(LED_GREEN) == LED_ON){
        ledGreen(false);
        greenMillisFlash = millis();
      }
    }
  }
  if(redMillisFlash >= millis()){
    ledRed(false);
    redMillisFlash = 0;
  }

}

void IOHandler::onOpenSwitchCallback(bool closed){
  switch_open_closed = closed;
  Debug.println(String(F("onOpenSwitchCallback ")) + String(closed ? CLOSED : OPEN));
  processSwitchs();
  switches.addSwitchPin(SWITCH_OPEN, !closed, _switchCallback);

}

void IOHandler::onClosedSwitchCallback(bool closed){
  switch_closed_closed = closed;
  Debug.println(String(F("onClosedSwitchCallback ")) + String(closed ? CLOSED : OPEN));
  processSwitchs();
  switches.addSwitchPin(SWITCH_CLOSED, !closed, _switchCallback);
}

void IOHandler::processSwitchs(){
  bool s_open = switch_open_closed;
  bool s_closed = switch_closed_closed;

  doorPositions door_position = settingsFile->getCurrentDoorPosition();
  doorPositions current_door_position = door_position;
  bool door_moving = settingsFile->isDoorMoving();
  Debug.print(F("IOHandler Switch: "));
  switch(door_position){
    case dpStartup:
    case dpUnknown:
      Debug.print(F("Startup "));
      if(s_closed){
        Debug.println(CLOSED);
        door_position = dpClosed;
        ledRed(false);
      }else if(s_open){
        door_position = dpOpen;
        Debug.println(OPEN);
        ledRed(false);
      }else{
        Debug.println(UNKNOWN);
        ledRed(true);
        delay(1000);
      }
      break;
    case dpClosedToOpen:
      Debug.print(CLOSED_OPEN);
      Debug.print(":");
      if(!s_closed && ! door_moving){
        Debug.println(MOVING);
        door_moving = true;
      }
      if(s_open && door_moving){
        Debug.println(OPEN);
        door_position = dpOpen;
        door_moving = false;
      }
      if(s_closed && door_moving){
        Debug.println(CLOSED);
        door_position = dpClosed;
        door_moving = false;
      }
      break;
    case dpOpenToClosed:
      Debug.print(OPEN_CLOSED);
      Debug.print(":");
      if(!s_open && !door_moving){
        Debug.println(MOVING);
        door_moving = true;
      }
      if(s_open && door_moving){
        Debug.println(OPEN);
        door_position = dpOpen;
        door_moving = false;
      }
      if(s_closed && door_moving){
        Debug.println(CLOSED);
        door_position = dpClosed;
        door_moving = false;
      }
      break;
    case dpClosed:
      Debug.print(CLOSED);
      if(!s_closed && !door_moving){
        Debug.print(":");
        Debug.print(MOVING);
        Debug.print(":");
        Debug.println(MANUAL_CLOSED_OPEN);
        door_moving = true;
        door_position = dpManualClosedToOpen;
      }
      break;
    case dpOpen:
      Debug.print(OPEN);
      if(!s_open && !door_moving){
        Debug.print(":");
        Debug.print(MOVING);
        Debug.print(":");
        Debug.println(MANUAL_OPEN_CLOSED);
        door_moving = true;
        door_position = dpManualOpenToClosed;
      }
      break; 
    case dpManualClosedToOpen:
      Debug.print(MANUAL_CLOSED_OPEN);
      Debug.print(":");
      if(s_open && door_moving){
        Debug.println(OPEN);
        door_position = dpOpen;
        door_moving = false;
      }
      if(s_closed && door_moving){
        Debug.println(CLOSED);
        door_position = dpClosed;
        door_moving = false;
      }
      break;
    case dpManualOpenToClosed:
        Debug.print(MANUAL_OPEN_CLOSED);
        Debug.print(":");
      if(s_open && door_moving){
        Debug.println(OPEN);
        door_position = dpOpen;
        door_moving = false;
      }
      if(s_closed && door_moving){
        Debug.println(CLOSED);
        door_position = dpClosed;
        door_moving = false;
      }
      break;
  }
  mqttHandler->updateDoorPosition(current_door_position, door_position, door_moving);
  Debug.println("");
}

void IOHandler::actionDoor(String position){
  Debug.print(F("IOHandler Action: "));
  if(!settingsFile->isLocked()){
    doorPositions current_door_position = settingsFile->getCurrentDoorPosition();
    doorPositions door_position = current_door_position;
    
    if (position.compareTo(OPEN) == 0)
    {
      if(door_position == dpClosed){
        Debug.println(CLOSED_OPEN);
        // start_move_door_position = door_position;
        door_position = dpClosedToOpen;
        toggleRelay();
      }else{
        redMillisFlash = millis() + 1000;
        ledRed(true);
      }
    }
    if (position.compareTo(CLOSE) == 0)
    {
      if(door_position == dpOpen){
        Debug.println(OPEN_CLOSED);
        // start_move_door_position = door_position;
        door_position = dpOpenToClosed;
        toggleRelay();
      }else{
        redMillisFlash = millis() + 1000;
        ledRed(true);
      }
    }

    mqttHandler->updateDoorPosition(current_door_position, door_position);
  }else{
    ledRed(true);
    Debug.println(LOCKED);
  }
}


void IOHandler::onOpenCloseButtonPress(uint8_t pin, uint32_t msPressed){
  Debug.print(F("IOHandler onOpenCloseButtonPress "));
  if(!settingsFile->isLocked() && msPressed < 1000){
    Debug.println(F("Toggle"));
    toggleRelay();
  }else if(msPressed > 5000){
    Debug.println(LOCK);     
    mqttHandler->toggleLocked();
    ledRed(settingsFile->isLocked());
  }else{
    Debug.println(settingsFile->isLocked() ? LOCKED : UNLOCKED);
  }
}

void IOHandler::toggleRelay(){
  ledRed(true);
  digitalWrite(RELAY,RELAY_CLOSED);
  delay(500);
  ledRed(false);
  digitalWrite(RELAY,RELAY_OPEN);

}

void IOHandler::readTemperature(){
  if(DS18B20.getDS18Count() > 0){
    if(temperatureLastRead == 0 || (millis() - temperatureLastRead) >= configFile->update_interval_ms ){
      ESP.wdtDisable();
      for(int d = 0; d < 5; d++) {
        DS18B20.requestTemperatures(); 
        float temp = DS18B20.getTempCByIndex(0);
        if(temp != 85.0 && temp != (-127.0)){
          settingsFile->setTemperature(temp);
          mqttHandler->setTemperature(temp);
          break;
        }
        Debug.println(F("Temperature Reading Failed"));
        delay(1000);
      }
      temperatureLastRead = millis();
      ESP.wdtEnable((uint32_t)0);
    }
  }
}
