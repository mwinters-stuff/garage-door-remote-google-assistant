#include "iohandler.h"
#include "datastore.h"
#include "logging.h"

std::shared_ptr<IOHandler> IOHandler::m_instance;

IOHandler::IOHandler():
  greenMillisFlash(0),
  redMillisFlash(0)
{

}

std::shared_ptr<IOHandler> IOHandler::init(){
  m_instance = std::make_shared<IOHandler>();
  return m_instance;
}

std::shared_ptr<IOHandler> IOHandler::get(){
  return m_instance;
}

void IOHandler::_switchCallback(uint8_t pin, bool closed){
  if(pin == SWITCH_OPEN){
    IOHandler::get()->onOpenSwitchCallback(closed);
  }else if(pin == SWITCH_CLOSED){
    IOHandler::get()->onClosedSwitchCallback(closed);
  }
}

void IOHandler::setup(){
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

  switches.addSwitchPin(SWITCH_OPEN, digitalRead(SWITCH_OPEN) == HIGH, _switchCallback);
  switches.addSwitchPin(SWITCH_CLOSED, digitalRead(SWITCH_CLOSED) == HIGH, _switchCallback);

}

void IOHandler::ledGreen(bool on){
  digitalWrite(LED_GREEN,on ? LED_ON : LED_OFF);
}

void IOHandler::ledRed(bool on){
  digitalWrite(LED_RED,on ? LED_ON : LED_OFF);
}

void IOHandler::update(){
  switches.update();

  doorPositions door_position = DataStore::get()->door_position;

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

  if(redMillisFlash >= millis()){
    ledRed(false);
    redMillisFlash = 0;
  }

}

void IOHandler::onOpenSwitchCallback(bool closed){
  switch_open_closed = closed;
  processSwitchs();
  switches.addSwitchPin(SWITCH_OPEN, !closed, _switchCallback);

}

void IOHandler::onClosedSwitchCallback(bool closed){
  switch_closed_closed = closed;
  processSwitchs();
  switches.addSwitchPin(SWITCH_CLOSED, !closed, _switchCallback);
}

void IOHandler::processSwitchs(){
  bool s_open = switch_open_closed;
  bool s_closed = switch_closed_closed;

  doorPositions door_position = DataStore::get()->door_position;
  doorPositions current_door_position = door_position;
  bool door_moving = DataStore::get()->door_moving;
    
  switch(door_position){
    case dpStartup:
    case dpUnknown:
      if(s_closed){
        door_position = dpClosed;
        Logging::get()->log(F("IOHandler"),F("Startup Door CLOSED"));
        ledRed(false);
      }else if(s_open){
        door_position = dpOpen;
        ledRed(false);
        Logging::get()->log(F("IOHandler"),F("Startup Door OPEN"));
      }else{
        Logging::get()->log(F("IOHandler"),F("Startup Door UNKNOWN"));
        ledRed(true);
        delay(1000);
      }
      break;
    case dpClosedToOpen:
      if(!s_closed && ! door_moving){
        Logging::get()->log(F("IOHandler"),F("Switch: ClosedToOpen Door Moving"));
        door_moving = true;
      }
      if(s_open && door_moving){
        Logging::get()->log(F("IOHandler"),F("Switch: ClosedToOpen Door Open"));
        door_position = dpOpen;
        door_moving = false;
      }
      if(s_closed && door_moving){
        Logging::get()->log(F("IOHandler"),F("Switch: ClosedToOpen Door Closed"));
        door_position = dpClosed;
        door_moving = false;
      }
      break;
    case dpOpenToClosed:
      if(!s_open && !door_moving){
        Logging::get()->log(F("IOHandler"),F("Switch: OpenToClosed Door Moving"));
        door_moving = true;
      }
      if(s_open && door_moving){
        Logging::get()->log(F("IOHandler"),F("Switch: OpenToClosed Door Open"));
        door_position = dpOpen;
        door_moving = false;
      }
      if(s_closed && door_moving){
        Logging::get()->log(F("IOHandler"),F("Switch: OpenToClosed Door Closed"));
        door_position = dpClosed;
        door_moving = false;
      }
      break;
    case dpClosed:
      if(!s_closed && !door_moving){
        Logging::get()->log(F("IOHandler"),F("Switch: Closed -> Manual Movement Door Moving"));
        door_moving = true;
        door_position = dpManualClosedToOpen;
      }
      break;
    case dpOpen:
      if(!s_open && !door_moving){
        Logging::get()->log(F("IOHandler"),F("Switch: Open -> Manual Movement Door Moving"));
        door_moving = true;
        door_position = dpManualOpenToClosed;
      }
      break; 
    case dpManualClosedToOpen:
      if(s_open && door_moving){
        Logging::get()->log(F("IOHandler"),F("Switch: ManualClosedToOpen Door Open"));
        door_position = dpOpen;
        door_moving = false;
      }
      if(s_closed && door_moving){
        Logging::get()->log(F("IOHandler"),F("Switch: ManualClosedToOpen Door Closed"));
        door_position = dpClosed;
        door_moving = false;
      }
      break;
    case dpManualOpenToClosed:
      if(s_open && door_moving){
        Logging::get()->log(F("IOHandler"),F("Switch: ManualOpenToClosed Door Open"));
        door_position = dpOpen;
        door_moving = false;
      }
      if(s_closed && door_moving){
        Logging::get()->log(F("IOHandler"),F("Switch: ManualOpenToClosed Door Closed"));
        door_position = dpClosed;
        door_moving = false;
      }
      break;
  }
  DataStore::get()->updateDoorPosition(current_door_position, door_position, door_moving);
}

void IOHandler::actionDoor(String position){
  if(!DataStore::get()->is_locked){
    doorPositions current_door_position = DataStore::get()->door_position;
    doorPositions door_position = current_door_position;

    if (position.compareTo(F("OPEN")) == 0)
    {
      if(door_position == dpClosed){
        Logging::get()->log(F("IOHandler"),F("Action: Door is closed, Open"));
        // start_move_door_position = door_position;
        door_position = dpClosedToOpen;
        toggleRelay();
      }else{
        redMillisFlash = millis() + 1000;
        ledRed(true);
      }
    }
    if (position.compareTo(F("CLOSE")) == 0)
    {
      if(door_position == dpOpen){
        Logging::get()->log(F("IOHandler"),F("Action: Door is open, Close"));
        // start_move_door_position = door_position;
        door_position = dpOpenToClosed;
        toggleRelay();
      }else{
        redMillisFlash = millis() + 1000;
        ledRed(true);
      }
    }

    DataStore::get()->updateDoorPosition(current_door_position, door_position);
  }else{
    Logging::get()->log(F("IOHandler"),F("door Locked"));
  }
}


void IOHandler::onOpenCloseButtonPress(uint8_t pin, uint32_t msPressed){
  Logging::get()->log(F("IOHandler"),F("onOpenCloseButtonPress"));
  if(!DataStore::get()->is_locked && msPressed < 1000){
    toggleRelay();
  }else if(msPressed > 3000){
    Logging::get()->log(F("IOHandler"),F("onOpenCloseButtonPress Long"));     
    DataStore::get()->toggleLocked();
  }else{
    Logging::get()->log(F("IOHandler"),F("onOpenCloseButtonPress door Locked"));
  }
}

void IOHandler::toggleRelay(){
  Logging::get()->log(F("IOHandler"),F("Toggling garage door"));
  ledRed(true);
  digitalWrite(RELAY,RELAY_CLOSED);
  delay(500);
  ledRed(false);
  digitalWrite(RELAY,RELAY_OPEN);

}