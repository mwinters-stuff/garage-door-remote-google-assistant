#include "iohandler.h"
#include "datastore.h"
#include "logging.h"

std::shared_ptr<IOHandler> IOHandler::m_instance;

IOHandler::IOHandler():
  buttonOpenClose(OPEN_CLOSE_BUTTON),
  buttonOpenSwitch(SWITCH_OPEN),
  buttonClosedSwitch(SWITCH_CLOSED),
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

void _onButtonRelease(Button& btn, uint16_t duration){
  IOHandler::get()->onButtonRelease(btn, duration);
}

void _onButtonPress(Button& btn){
  IOHandler::get()->onButtonPress(btn);
}

void _onButtonHeld(Button& btn, uint16_t duration){
  IOHandler::get()->onOpenCloseButtonHeld(btn, duration);
}

void IOHandler::setup(){
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(RELAY, OUTPUT);
  pinMode(OPEN_CLOSE_BUTTON,INPUT_PULLUP);
  pinMode(SWITCH_OPEN,INPUT_PULLUP);
  pinMode(SWITCH_CLOSED,INPUT_PULLUP);

  digitalWrite(LED_RED,LED_ON);
  digitalWrite(LED_GREEN,LED_ON);
  digitalWrite(LED_BUILTIN,HIGH);

  buttonOpenClose.onRelease(_onButtonRelease);
  buttonOpenClose.onHold(5000, _onButtonHeld);

  buttonOpenSwitch.onPress(_onButtonPress);
  buttonOpenSwitch.onRelease(_onButtonRelease);

  buttonClosedSwitch.onPress(_onButtonPress);
  buttonClosedSwitch.onRelease(_onButtonRelease);
}

void IOHandler::ledGreen(bool on){
  digitalWrite(LED_GREEN,on ? LED_ON : LED_OFF);
}

void IOHandler::ledRed(bool on){
  digitalWrite(LED_RED,on ? LED_ON : LED_OFF);
}

void IOHandler::onButtonPress(Button &btn){
  if(btn.is(buttonOpenClose)){
  } else if(btn.is(buttonOpenSwitch)){
    onOpenSwitchPress();
  } else if(btn.is(buttonClosedSwitch)){
    onClosedSwitchPress();
  }
}

void IOHandler::onButtonRelease(Button &btn, uint16_t duration){
  if(btn.is(buttonOpenClose)){
    onOpenCloseButtonRelease(duration);
  } else if(btn.is(buttonOpenSwitch)){
    onOpenSwitchRelease();
  } else if(btn.is(buttonClosedSwitch)){
    onClosedSwitchRelease();
  }
}

void IOHandler::update(){
  buttonOpenClose.update();
  buttonOpenSwitch.update();
  buttonClosedSwitch.update();
//  readSwitchs();

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

void IOHandler::onOpenSwitchPress(){
  Logging::get()->log(F("IOHandler"),F("onOpenSwitchPress"));
  processSwitchs();
}

void IOHandler::onOpenSwitchRelease(){
  Logging::get()->log(F("IOHandler"),F("onOpenSwitchRelease"));
  processSwitchs();
}

void IOHandler::onClosedSwitchPress(){
  Logging::get()->log(F("IOHandler"),F("onClosedSwitchPress"));
  processSwitchs();
}

void IOHandler::onClosedSwitchRelease(){
  Logging::get()->log(F("IOHandler"),F("onClosedSwitchRelease"));
  processSwitchs();
}

void IOHandler::processSwitchs(){
  bool s_open = buttonOpenSwitch.isPressed();
  bool s_closed = buttonClosedSwitch.isPressed();

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


void IOHandler::onOpenCloseButtonRelease(uint16_t duration){
  Logging::get()->log(F("IOHandler"),F("onOpenCloseButtonRelease"));
  if(duration < 1000){
    if(!DataStore::get()->is_locked){
      toggleRelay();
    }else{
      Logging::get()->log(F("IOHandler"),F("door Locked"));
    }
  }
}

void IOHandler::onOpenCloseButtonHeld(Button& btn, uint16_t duration){
  Logging::get()->log(F("IOHandler"),F("onOpenCloseButtonHeld"));
  DataStore::get()->toggleLocked();
}


void IOHandler::toggleRelay(){
  Logging::get()->log(F("IOHandler"),F("Toggling garage door"));
  ledRed(true);
  digitalWrite(RELAY,RELAY_CLOSED);
  delay(500);
  ledRed(false);
  digitalWrite(RELAY,RELAY_OPEN);

}