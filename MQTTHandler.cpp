#include <Arduino.h>
// #include <AdafruitIO_Feed.h>
// #include <AdafruitIO_WiFi.h>
// #include <EEPROM.h>

#include <RemoteDebug.h>
//extern RemoteDebug Debug;
#define Debug Serial

#include "MQTTHandler.h"

#define AIO_SERVER "io.adafruit.com"
#define AIO_SERVERPORT 8883

MQTTHandler *MQTTHandler::m_instance;

static const char door_position_strings[][20] PROGMEM = {"STARTUP", "UNKNOWN", "OPEN", "CLOSED", "OPEN_CLOSED", "CLOSED_OPEN", "MANUAL_OPEN_CLOSED", "MANUAL_CLOSED_OPEN"};

void handleInHomeAreaMessage(AdafruitIO_Data *data)
{
  MQTTHandler::_getInstance()->inHomeMessage(data);
}

void handleDoorActionMessage(AdafruitIO_Data *data)
{
  MQTTHandler::_getInstance()->doorAction(data);
}

MQTTHandler::MQTTHandler(SettingsFile *settingsFile, ConfigFile *configFile) : 
  settingsFile(settingsFile),
  configFile(configFile),
  door_position(dpStartup),
  // start_move_door_position(dpStartup),
  door_moving(false),
  io(NULL)
{
  m_instance = this;
}

// MQTTHandler *MQTTHandler::init()
// {
//   m_instance = new MQTTHandler();
//   return m_instance;
// }

// MQTTHandler *MQTTHandler::get()
// {
//   return m_instance;
// }

void MQTTHandler::initFeeds()
{
  Debug.println(F("Init Feeds"));

  if (configFile->io_feed_username.length() > 0 && configFile->io_feed_key.length() > 0)
  {
    Debug.println(F("Starting Adafruit IO"));
    io = new AdafruitIO_Custom(configFile->io_feed_username.c_str(), configFile->io_feed_key.c_str());

    io_door_action = io->feed(configFile->io_feed_door_action.c_str());
    io_door_position = io->feed(configFile->io_feed_position.c_str());
    io_in_home_area = io->feed(configFile->io_feed_in_home_area.c_str());
    
    connect();

  }
  else
  {
    Debug.println(F("Adafruit IO Not Configured"));
    if (io)
    {
      delete io;
      delete io_door_action;
      delete io_door_position;
      delete io_in_home_area;
      io = NULL;
      io_door_action = NULL;
      io_door_position = NULL;
      io_in_home_area = NULL;
    }
  }
}


void MQTTHandler::update()
{
  if(io){
    io->run();
  }
  
}

void MQTTHandler::connect()
{
  int8_t ret;

  // Stop if already connected.
  if (!io || io->mqttStatus() < AIO_CONNECTED)
  {
    return;
  }


  Debug.println(F("Connecting to Adafruit IO... "));
  io->connect();

  uint8_t retries = 5;
  while ((ret = io->mqttStatus()) < AIO_CONNECTED)
  { // connect will return 0 for connected
    Debug.println(io->statusText());
    Debug.println(F("Retrying Adafruit IO connection in 5 seconds..."));
    delay(500); // wait 5 seconds
    retries--;
    if (retries == 0)
    {
      // basically die and wait for WDT to reset me
      while (1)
        ;
    }
  }

  Debug.println(F("Adafruit IO Connected!"));

  io_in_home_area->onMessage(handleInHomeAreaMessage);
  io_door_action->onMessage(handleDoorActionMessage);

  io_door_action->get();
  io_door_position->get();
}


void MQTTHandler::inHomeMessage(AdafruitIO_Data *data)
{
  Debug.print(F("Action: Received in home area -> "));
  Debug.print(data->value());

  settingsFile->is_in_home_area = strcmp_P(data->value(), PSTR("entered")) == 0;
  Debug.print(F("inHomeMessage "));
  Debug.println(settingsFile->is_in_home_area ? F(" inside geofence") : F(" outside geofence"));
  
}


void MQTTHandler::doorAction(AdafruitIO_Data *data)
{
  settingsFile->last_door_action = data->value();
  Debug.print(F("Action: Received door action -> "));
  Debug.println(settingsFile->last_door_action);

  if (!settingsFile->is_in_home_area)
  {
    Debug.println(F("IOHandler Not near home, won't action door!"));
  }else if(doorActionCallback){
    Debug.println("doorActionCallback");
    doorActionCallback(settingsFile->last_door_action);
  }
}

void MQTTHandler::updateDoorPosition(doorPositions current_door_position, doorPositions _door_position)
{
  if (current_door_position != _door_position)
  {
    door_position = _door_position;
    Debug.print(F("Action: Update Door Position:"));
    Debug.print(FPSTR(door_position_strings[current_door_position]));
    Debug.print(F(" --> "));
    settingsFile->current_door_position = String(FPSTR(door_position_strings[door_position]));
    Debug.println(settingsFile->current_door_position);
    if(io_door_position){
      io_door_position->save(settingsFile->current_door_position);
    }
  }
}

void MQTTHandler::updateDoorPosition(doorPositions current_door_position, doorPositions _door_position, bool _door_moving)
{
  door_moving = _door_moving;
  updateDoorPosition(current_door_position, _door_position);
}


void MQTTHandler::toggleLocked()
{
  setLocked(!settingsFile->is_locked);
}

void MQTTHandler::setLocked(bool locked)
{
  if (locked)
  {
    Debug.println(F("IOHandler Setting lock to LOCKED"));
  }
  else
  {
    Debug.println(F("IOHandler Setting lock to UNLOCKED"));
  }
  settingsFile->is_locked = locked;
  
}