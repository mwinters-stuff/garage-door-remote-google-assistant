#include "IOHandler.h"

#include <Arduino.h>
#include <ESP8266DebounceSwitch.h>
#include <RemoteDebug.h>

#include "MQTTHandler.h"
#include "StringConstants.h"
extern RemoteDebug Debug;

IOHandler *IOHandler::m_instance;
extern void log_printf(PGM_P fmt_P, ...);

#define SONIC_NO_READING 38000.0

IOHandler::IOHandler(MQTTHandler *mqttHandler, SettingsFile *settingsFile, ConfigFile *configFile)
    : mqttHandler(mqttHandler),
      settingsFile(settingsFile),
      configFile(configFile),
      greenMillisFlash(0),
      redMillisFlash(0),
#ifdef ONE_WIRE_PIN
      oneWire(new OneWireNg_CurrentPlatform(ONE_WIRE_PIN, false)),
      dsTherm(new DSTherm(*oneWire)),
#endif
      sonic(SONAR_TRIGGER, SONAR_ECHO, 20, 100),
      sonarReadMillis(0),
      sonic_reading_commanded(false),
      sonic_read_interval(SONAR_READ_INTERVAL_WAITING) {
  m_instance = this;
}

void IOHandler::init() {
  Debug.println(F("IOHandler init"));
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(RELAY, OUTPUT);

  digitalWrite(LED_RED, LED_ON);
  digitalWrite(LED_GREEN, LED_ON);
  digitalWrite(LED_BUILTIN, HIGH);

#ifdef OPEN_CLOSE_BUTTON
  switches.addButtonPin(
      OPEN_CLOSE_BUTTON, [&](uint8_t pin, uint32_t msPressed) { onOpenCloseButtonPress(pin, msPressed); }, true);
#endif

  mqttHandler->doorCommandCallback = [&](String command) { doorCommand(command); };
#ifdef ONE_WIRE_PIN
  initOneWire();
#else
  Debug.println(F("No DS18B20 Pin Configured"));
#endif
  sonic.begin();
  temperatureLastRead = millis();
  Debug.println(F("IOHandler init Done"));
}

static bool printId(const OneWireNg::Id &id) {
  const char *name = DSTherm::getFamilyName(id);

  Debug.print(id[0], HEX);
  for (size_t i = 1; i < sizeof(OneWireNg::Id); i++) {
    Debug.print(':');
    Debug.print(id[i], HEX);
  }
  if (name) {
    Debug.print(" -> ");
    Debug.print(name);
  }
  Debug.println();

  return (name != NULL);
}

void IOHandler::initOneWire() {
  Debug.println(F("Searching One Wire"));

  OneWireNg::Id id;
  OneWireNg::ErrorCode ec;

  do {
    ec = oneWire->search(id);
    if (ec == OneWireNg::EC_MORE || ec == OneWireNg::EC_DONE) {
      Debug.print(F("Found One Wire Device "));
      printId(id);
      if (id[0] == DSTherm::DS18B20) {
        memcpy(dsThermId, id, sizeof(OneWireNg::Id));
        break;
      }
      // `id' contains 1-wire address of a connected slave
    }
  } while (ec == OneWireNg::EC_MORE);
}

void IOHandler::ledGreen(bool on) {
  pinMode(LED_GREEN, OUTPUT);
  digitalWrite(LED_GREEN, on ? LED_ON : LED_OFF);
}

void IOHandler::ledGreenToggle() {
  pinMode(LED_GREEN, OUTPUT);
  digitalWrite(LED_GREEN, digitalRead(LED_GREEN) == LED_OFF ? LED_ON : LED_OFF);
}

void IOHandler::ledRed(bool on) {
  pinMode(LED_RED, OUTPUT);
  digitalWrite(LED_RED, on ? LED_ON : LED_OFF);
}

void IOHandler::ledRedToggle() {
  pinMode(LED_RED, OUTPUT);
  digitalWrite(LED_RED, digitalRead(LED_RED) == LED_OFF ? LED_ON : LED_OFF);
}

void IOHandler::update() {
  if (!mqttHandler->isMQTTConnected()) {
    return;
  }

#ifdef ONE_WIRE_PIN
  readTemperature();
#endif
  switches.update();
#ifdef TEST
  sonic_read_interval = 2000;
#endif

  readSonar();
}

void IOHandler::readSonar() {
  if (millis() - sonarReadMillis > sonic_read_interval) {
    sonic.setTemperature(settingsFile->getTemperature());
    noInterrupts();
    sonic_last_distance = SONIC_NO_READING;
    float readingssum = 0.0f;
    float readings_count = 0;
    for (int i = 0; i < 5; i++) {
      float r = sonic.getDistance();
      // Debug.print("R: ");
      // Debug.print(r);
      // Debug.println();

      if (r != SONIC_NO_READING) {
        readingssum += r;
        ++readings_count;
      }
    }
    interrupts();
    if (readings_count > 0) {
      sonic_last_distance = readingssum / readings_count;
    }
    mqttHandler->setSonicReading(sonic_last_distance == SONIC_NO_READING ? 0 : sonic_last_distance);

    Debug.print(F("Distance CM "));
    Debug.print(sonic_last_distance);
    Debug.print(F(" Min "));
    Debug.print(configFile->open_distance_min);
    Debug.print(F(" Max "));
    Debug.print(configFile->open_distance_max);

    String log = String(F("Sonic CM ")) + String(sonic_last_distance);
    Debug.print(F(" Open "));
    bool within_open = (sonic_last_distance >= configFile->open_distance_min && sonic_last_distance <= configFile->open_distance_max);
    Debug.print(within_open ? F("TRUE") : F("FALSE"));
    bool is_closed = sonic_last_distance == SONIC_NO_READING || !within_open;
    Debug.print(F(" Closed "));
    Debug.println(is_closed ? F("TRUE") : F("FALSE"));
    if (sonarReadMillis == 0) {
      // first run update mqtt.
      ledGreen(false);
      ledRed(true);
      mqttHandler->setClosed(is_closed);
    }

    if (is_closed) {
      if (!settingsFile->isClosed()) {
        log += F(" setting CLOSED");
        ledGreen(false);
        ledRed(true);
        mqttHandler->setClosed(true);
      }
    } else {
      if (!settingsFile->isOpen()) {
        log += F(" setting OPEN");
        ledGreen(true);
        ledRed(false);
        mqttHandler->setClosed(false);
      }
    }
    log_printf(log.c_str());
    sonarReadMillis = millis();
#ifndef TEST
    if (sonic_reading_commanded) {
      if (millis() - sonic_read_commanded_start > SONAR_READ_COMMANDED_FOR) {
        log_printf(PSTR("No Longer reading sonic for commanded interval"));
        sonic_reading_commanded = false;
        sonic_read_interval = SONAR_READ_INTERVAL_WAITING;
      }
    }
#endif
  }
}

void IOHandler::doorCommand(String command) {
  String logString = String(F("DoorCommand ")) + command;
  if (!settingsFile->isLocked()) {
    if (command.compareTo(FORCE) == 0) {
      logString += F(": Force Door Requested");
      redMillisFlash = millis();
      toggleRelay();
    }

    if (command.compareTo(OPEN) == 0) {
      if (settingsFile->isClosed()) {
        logString += F(": Open Requested");
        greenMillisFlash = 10000;
        ledGreen(true);
        toggleRelay();
      } else {
        logString += F(": Already Open");
        redMillisFlash = millis() + 1000;
        ledRed(true);
      }
    }
    if (command.compareTo(CLOSE) == 0) {
      if (settingsFile->isOpen()) {
        logString += F(": Close Requested");
        greenMillisFlash = 10000;
        ledGreen(true);
        toggleRelay();
      } else {
        logString += F(": Already Closed");
        redMillisFlash = millis() + 1000;
        ledRed(true);
      }
    }
  } else {
    ledRed(true);
    logString += LOCK;
  }
  log_printf(logString.c_str());
}

#ifdef OPEN_CLOSE_BUTTON
void IOHandler::onOpenCloseButtonPress(uint8_t pin, uint32_t msPressed) {
  String logString;
  if (!settingsFile->isLocked() && msPressed < 1000) {
    logString += F("Toggle ");
    greenMillisFlash = 10000;
    ledGreen(true);
    toggleRelay();
  } else if (msPressed > 5000) {
    logString += F("Changing to ");
    logString += (settingsFile->isLocked() ? LOCK : UNLOCK);
    mqttHandler->toggleLocked();
    ledRed(settingsFile->isLocked());
  } else {
    logString += F("IS ");
    logString += (settingsFile->isLocked() ? LOCK : UNLOCK);
  }
  log_printf(PSTR("onOpenCloseButtonPress %d %s"), msPressed, logString.c_str());
}
#endif

void IOHandler::toggleRelay() {
  sonic_reading_commanded = true;
  sonic_read_interval = SONAR_READ_INTERVAL_COMMANDED;
  sonic_read_commanded_start = millis();

  log_printf(PSTR("Toggle Relay"));
  ledRed(true);
  digitalWrite(RELAY, RELAY_CLOSED);
  delay(RELAY_TOGGLE_TIME);
  ledRed(false);
  digitalWrite(RELAY, RELAY_OPEN);
}

#ifdef ONE_WIRE_PIN
void IOHandler::readTemperature() {
  if (temperatureLastRead == 0 || (millis() - temperatureLastRead) >= configFile->update_interval_ms) {
    if (dsTherm->convertTemp(dsThermId) != OneWireNg::EC_SUCCESS) {
      Debug.println(F("DS18B20 convert failed"));
    } else {
      MAKE_SCRATCHPAD(scrpd);
      if (dsTherm->readScratchpad(dsThermId, scrpd) != OneWireNg::EC_SUCCESS) {
        Debug.println(F("DS18B20 Invalid CRC!"));
      } else {
        float temp = scrpd->getTemp() / 1000.0f;
        if (temp != 85.0 && temp != (-127.0)) {
          Debug.print(F("Temperature -> "));
          Debug.println(temp);
          settingsFile->setTemperature(temp);
          mqttHandler->setTemperature(temp);
        } else {
          Debug.println(F("Temperature Reading Failed"));
        }
      }
    }
    temperatureLastRead = millis();
  }
}
#endif