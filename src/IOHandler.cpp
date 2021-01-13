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
  Debug.print(F("DS18B20 Init Found "));
  Debug.println(DS18B20.getDS18Count());
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
  if(!mqttHandler->isMQTTConnected()){
    return;
  }
  if(firstLoop){
    firstLoop = false;
    doorPositions door_position;
    if(digitalRead(SWITCH_OPEN) == LOW && digitalRead(SWITCH_CLOSED) == HIGH){
      log_printf(PSTR("Inital OPEN"));
      door_position = dpOpen;
    }else if(digitalRead(SWITCH_CLOSED) == LOW && digitalRead(SWITCH_OPEN) == HIGH){
      log_printf(PSTR("Inital CLOSED"));
      door_position = dpClosed;
    } else {
      door_position = dpOpenToClosed;
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

    if(door_position == dpOpenRequested || door_position == dpCloseRequested){
      if(millis() - redMillisFlash>= REQUEST_FLASH_TIME_OFF && digitalRead(LED_RED) == LED_OFF){
        ledRed(true);
        redMillisFlash = millis();
      }
      if(millis() - redMillisFlash >= REQUEST_FLASH_TIME_ON && digitalRead(LED_RED) == LED_ON){
        ledRed(false);
        redMillisFlash = millis();
      }

      if(millis() - requestMillis > REQUEST_MUST_OPEN){
        ledRed(false);
        redMillisFlash = 0;
        requestMillis = 0;

        requestRetries--;
        if(requestRetries > 0){
          log_printf(PSTR("Request failed %d"), requestRetries);
          toggleRelay();
          requestMillis = millis();
        }else{
          log_printf(PSTR("Request failed leaving as is"));
          doorPositions new_door_position = door_position;
          if(door_position == dpOpenRequested){
            new_door_position = dpClosed;
          }else if(door_position == dpCloseRequested){
            new_door_position = dpOpen;
          }
          mqttHandler->updateDoorPosition(door_position, new_door_position, false);
        }
      }
    }
  }

  doorPositions door_position = settingsFile->getCurrentDoorPosition();
  if(redMillisFlash >= millis() && door_position != dpOpenRequested && door_position != dpCloseRequested){
    ledRed(false);
    redMillisFlash = 0;
  }

}

void IOHandler::onOpenSwitchCallback(bool closed){
  switch_open_closed = closed;
  log_printf(PSTR("onOpenSwitchCallback %s"), String(closed ? CLOSE : OPEN).c_str());
  processSwitchs();
  switches.addSwitchPin(SWITCH_OPEN, !closed, _switchCallback);

}


void IOHandler::onClosedSwitchCallback(bool closed){
  switch_closed_closed = closed;
  log_printf(PSTR("onClosedSwitchCallback %s"), String(closed ? CLOSE : OPEN).c_str());
  processSwitchs();
  switches.addSwitchPin(SWITCH_CLOSED, !closed, _switchCallback);
}

const String colon(":");

void IOHandler::processSwitchs(){
  bool s_open = switch_open_closed;
  bool s_closed = switch_closed_closed;

  doorPositions door_position = settingsFile->getCurrentDoorPosition();
  doorPositions current_door_position = door_position;
  bool door_moving = settingsFile->isDoorMoving();
  String logString;
  switch(door_position){
    // case dpStartup:
    case dpUnknown:
      logString += F("Startup ");
      if(s_closed){
        logString += CLOSE;
        door_position = dpClosed;
        ledRed(false);
      }else if(s_open){
        door_position = dpOpen;
        logString += OPEN;
        ledRed(false);
      }else{
        logString += UNKNOWN;
        ledRed(true);
        delay(1000);
      }
      break;
    case dpClosedToOpen:
      logString += CLOSED_OPEN;
      logString += colon;
      if(!s_closed && ! door_moving){
        logString += MOVING;
        door_moving = true;
      }
      if(s_open && door_moving){
        logString += OPEN;
        door_position = dpOpen;
        door_moving = false;
      }
      if(s_closed && door_moving){
        logString += CLOSE;
        door_position = dpClosed;
        door_moving = false;
      }
      break;
    case dpOpenToClosed:
      logString += OPEN_CLOSED;
      logString += colon;
      if(!s_open && !door_moving){
        logString += MOVING;
        door_moving = true;
      }
      if(s_open && door_moving){
        logString += OPEN;
        door_position = dpOpen;
        door_moving = false;
      }
      if(s_closed && door_moving){
        logString += CLOSED;
        door_position = dpClosed;
        door_moving = false;
      }
      break;
    case dpOpenRequested:
      logString += F("Open requested");
      if(!s_closed && !door_moving){
        logString += colon + MOVING + colon + CLOSED_OPEN;
        door_moving = true;
        door_position = dpClosedToOpen;
        redMillisFlash = 0;
        ledRed(false);
      }
      break;
    case dpCloseRequested:
      logString += F("Close requested");
      if(!s_open && !door_moving){
        logString += colon + MOVING + colon + OPEN_CLOSED;
        door_moving = true;
        door_position = dpOpenToClosed;
        redMillisFlash = 0;
        ledRed(false);
      }
      break; 
    case dpClosed:
      logString += F("Door Opening");
      if(!s_closed && !door_moving){
        logString += colon + MOVING + colon + CLOSED_OPEN;
        door_moving = true;
        door_position = dpClosedToOpen;
      }
      break;
    case dpOpen:
      logString += F("Door closing");
      if(!s_open && !door_moving){
        logString += colon + MOVING + colon + OPEN_CLOSED;
        door_moving = true;
        door_position = dpOpenToClosed;
      }
      break; 
    
  }
  log_printf(PSTR("processSwitches %s"), logString.c_str());

  mqttHandler->updateDoorPosition(current_door_position, door_position, door_moving);
}

void IOHandler::actionDoor(String position){
  String logString;
  if(!settingsFile->isLocked()){
    doorPositions current_door_position = settingsFile->getCurrentDoorPosition();
    doorPositions door_position = current_door_position;
    
    if(position.compareTo(FORCE) == 0){
      logString += F("Force Door Requested");
      redMillisFlash = millis();
      requestMillis = millis();
      toggleRelay();
    }

    if (position.compareTo(OPEN) == 0) {
      if(door_position == dpClosed){
        logString += F("Open Requested");
        // start_move_door_position = door_position;
        door_position = dpOpenRequested;
        redMillisFlash = millis();
        requestMillis = millis();
        requestRetries = REQUEST_RETRIES;
        toggleRelay();
      }else{
        logString += F("Already Open");
        redMillisFlash = millis() + 1000;
        ledRed(true);
      }
    }
    if (position.compareTo(CLOSE) == 0) {
      if(door_position == dpOpen){
        logString += F("Close Requested");
        // start_move_door_position = door_position;
        door_position = dpCloseRequested;
        redMillisFlash = millis();
        requestMillis = millis();
        requestRetries = REQUEST_RETRIES;
        toggleRelay();
      }else{
        logString += F("Already Closed");
        redMillisFlash = millis() + 1000;
        ledRed(true);
      }
    }
    settingsFile->setCurrentDoorPosition(door_position);
    // mqttHandler->updateDoorPosition(current_door_position, door_position);
  }else{
    ledRed(true);
    logString += LOCK;
  }
  log_printf(PSTR("actionDoor %s"), logString.c_str());
}

void IOHandler::onOpenCloseButtonPress(uint8_t pin, uint32_t msPressed){
  String logString;
  if(!settingsFile->isLocked() && msPressed < 1000){
    logString += F("Toggle ");

    doorPositions current_door_position = settingsFile->getCurrentDoorPosition();
    doorPositions door_position = current_door_position;

    switch(current_door_position){
      case dpCloseRequested:
      case dpClosedToOpen:
      case dpClosed:
        logString += F("Button press open requested");
        door_position = dpOpenRequested;
        redMillisFlash = millis();
        requestMillis = millis();
        requestRetries = REQUEST_RETRIES;
        break;
      case dpOpenRequested:
      case dpOpenToClosed:
      case dpOpen:
        logString += F("Button press close requested");
        door_position = dpOpenRequested;
        redMillisFlash = millis();
        requestMillis = millis();
        requestRetries = REQUEST_RETRIES;
        break;
      default:
        logString += F("Button press, incompatible state");
        return;
    }
    settingsFile->setCurrentDoorPosition(door_position);

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

void IOHandler::toggleRelay(){
  log_printf(PSTR("Toggle Relay"));
  ledRed(true);
  digitalWrite(RELAY,RELAY_CLOSED);
  delay(1000);
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
