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
#ifdef ESP8266

#include "AdafruitIO_Custom.h"

AdafruitIO_Custom::AdafruitIO_Custom(const char *user, const char *key):AdafruitIO(user, key)
{
  _client = new WiFiClientSecure;
  _mqtt = new Adafruit_MQTT_Client(_client, _host, _mqtt_port);
  _http = new HttpClient(*_client, _host, _http_port);
}

AdafruitIO_Custom::~AdafruitIO_Custom()
{
  if(_client)
    delete _client;
  if(_mqtt)
    delete _mqtt;
}

void AdafruitIO_Custom::_connect()
{

  _status = networkStatus();

}

aio_status_t AdafruitIO_Custom::networkStatus()
{

  switch(WiFi.status()) {
    case WL_CONNECTED:
      return AIO_NET_CONNECTED;
    case WL_CONNECT_FAILED:
      return AIO_NET_CONNECT_FAILED;
    case WL_IDLE_STATUS:
      return AIO_IDLE;
    default:
      return AIO_NET_DISCONNECTED;
  }

}

const char* AdafruitIO_Custom::connectionType()
{
  return "wifiCustom";
}

#endif // ESP8266
