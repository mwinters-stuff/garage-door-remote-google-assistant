#ifndef _IOHANDLER_H
#define _IOHANDLER_H

#include <Arduino.h>
#include <ESP8266DebounceSwitch.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <SPI.h>
#include <functional>
#include <Ticker.h>

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
    IOHandler(std::shared_ptr<MQTTHandler> mqttHandler, std::shared_ptr<SettingsFile> settingsFile, std::shared_ptr<ConfigFile> configFile);

    // static std::shared_ptr<IOHandler> _getInstance(){return m_instance;};
    // static IOHandler* init();

    void init();
    void ledGreen(bool on);
    void ledRed(bool on);

    void update();
    
    void setupTimers();

    void doorCommand(String position);
    double sonicLastDistance(){
      return sonic_last_distance;
    };

  private:
    std::shared_ptr<MQTTHandler> mqttHandler;
    std::shared_ptr<SettingsFile> settingsFile;
    std::shared_ptr<ConfigFile> configFile;
    OneWire oneWire;
    DallasTemperature DS18B20;
    HCSR04 sonic;
    uint32_t sonic_read_commanded_start;
    double sonic_last_distance;
    bool doInit;
    Ticker ticker_relay;
    Ticker ticker_read_sonic;
    Ticker ticker_read_temperature;
    
    
#ifdef OPEN_CLOSE_BUTTON
    void onOpenCloseButtonPress(uint8_t pin, uint32_t msPressed);
#endif    
    void readTemperature();

    ESP8266DebounceSwitch switches;
    uint32_t sonarReadMillis;
    uint8_t requestRetries;
    bool sonic_reading_commanded;
    uint16_t sonic_read_interval;

    stringCallback doorCommandCallback;

    void toggleRelay();
    void readSonar();


    // static std::shared_ptr<IOHandler> m_instance;


};

#endif