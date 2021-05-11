#ifndef _IOHANDLER_H
#define _IOHANDLER_H

#include <Arduino.h>
#include <ESP8266DebounceSwitch.h>
#include <SPI.h>
#include <functional>
#include <OneWireNg_CurrentPlatform.h>
#include <drivers/DSTherm.h>

#include "ConfigFile.h"
#include "SettingsFile.h"
#include "HardwareConfig.h"
#include "MQTTHandler.h"
#include <HCSR04.h>
#include <memory>

// #define FLASH_TIME_ON 50
// #define MOVING_FLASH_TIME_OFF 450
// #define REQUEST_FLASH_TIME_OFF 100
// #define REQUEST_FLASH_TIME_ON 100
// #define WAITING_TIME_OFF 4950
// #define REQUEST_MUST_OPEN 5000
// #define REQUEST_RETRIES 3

class IOHandler{
  public:
    IOHandler(MQTTHandler *mqttHandler, SettingsFile *settingsFile, ConfigFile *configFile);

    static IOHandler* _getInstance(){return m_instance;};
    // static IOHandler* init();

    static void ledGreen(bool on);
    static void ledRed(bool on);
    static void ledGreenToggle();
    static void ledRedToggle();

    void init();
    void update();
    void doorCommand(String position);
    double sonicLastDistance(){
      return sonic_last_distance;
    };

  private:
    MQTTHandler *mqttHandler;
    SettingsFile *settingsFile;
    ConfigFile *configFile;
    uint32_t greenMillisFlash;
    uint32_t redMillisFlash;
#ifdef ONE_WIRE_PIN
    OneWireNg *oneWire;
    DSTherm *dsTherm;
    OneWireNg::Id dsThermId;
#endif
    HCSR04 sonic;
    uint32_t temperatureLastRead;
    uint32_t sonic_read_commanded_start;
    double sonic_last_distance;
    
    
#ifdef OPEN_CLOSE_BUTTON
    void onOpenCloseButtonPress(uint8_t pin, uint32_t msPressed);
#endif    
#ifdef ONE_WIRE_PIN
    void readTemperature();
#endif

    ESP8266DebounceSwitch switches;
    uint32_t sonarReadMillis;
    uint8_t requestRetries;
    bool sonic_reading_commanded;
    uint16_t sonic_read_interval;

    stringCallback doorCommandCallback;

    void toggleRelay();
    void readSonar();
    void initOneWire();

    static IOHandler* m_instance;


};

#endif