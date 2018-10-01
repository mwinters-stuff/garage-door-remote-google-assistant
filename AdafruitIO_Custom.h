//
// Adafruit invests time and resources providing this open source code.
// Please support Adafruit and open source hardware by purchasing
// products from Adafruit!
//
// Copyright (c) 2015-2016 Adafruit Industries
// Authors: Tony DiCola, Todd Treece
// Licensed under the MIT license.
//
// All text above must be included in any redistribution.
//
#ifndef ADAFRUITIO_CUSTOM_H
#define ADAFRUITIO_CUSTOM_H

#ifdef ESP8266

#include <Arduino.h>
#include <AdafruitIO.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>

class AdafruitIO_Custom : public AdafruitIO {

  public:
    AdafruitIO_Custom(const char *user, const char *key);
    ~AdafruitIO_Custom();

    aio_status_t networkStatus();
    const char* connectionType();

  protected:
    void _connect();

    WiFiClientSecure *_client;

};

#endif //ESP8266
#endif // ADAFRUITIO_ESP8266_H
