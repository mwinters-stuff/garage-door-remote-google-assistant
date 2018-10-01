#ifndef _IOHANDLER_H
#define _IOHANDLER_H

#include <Arduino.h>
#include <ESP8266DebounceSwitch.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#include "ConfigFile.h"
#include "SettingsFile.h"
#include "HardwareConfig.h"
#include "MQTTHandler.h"
#include <memory>

#define FLASH_TIME_ON 50
#define MOVING_FLASH_TIME_OFF 450
#define MANUAL_FLASH_TIME_OFF 150
#define WAITING_TIME_OFF 4950

class IOHandler{
  public:
    IOHandler(MQTTHandler *mqttHandler, SettingsFile *settingsFile, ConfigFile *configFile);

    static IOHandler* _getInstance(){return m_instance;};
    // static IOHandler* init();

    void ledGreen(bool on);
    void ledRed(bool on);

    void update();
    void actionDoor(String position);

    static void _switchCallback(uint8_t button, bool closed);
  private:
    SettingsFile *settingsFile;
    ConfigFile *configFile;
    MQTTHandler *mqttHandler;
    OneWire oneWire;
    DallasTemperature DS18B20;
    uint32_t temperatureLastRead;
    
    bool switch_open_closed;
    bool switch_closed_closed;

    void onOpenCloseButtonPress(uint8_t pin, uint32_t msPressed);
    void onOpenSwitchCallback(bool closed);
    void onClosedSwitchCallback(bool closed);
    void readTemperature();

    ESP8266DebounceSwitch switches;
    uint32_t greenMillisFlash;
    uint32_t redMillisFlash;

    void processSwitchs();
    void toggleRelay();


    static IOHandler* m_instance;


};

#endif