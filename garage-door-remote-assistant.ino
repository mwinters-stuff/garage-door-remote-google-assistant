
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <AdafruitIO_WiFi.h>

#include <AdafruitIO_Feed.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#include "config.h"


AdafruitIO_WiFi io(IO_USERNAME, IO_KEY, WIFI_SSID, WIFI_PASS);

AdafruitIO_Feed *io_door_action = io.feed("garagedooraction");
AdafruitIO_Feed *io_door_position = io.feed("garagedoorposition");
AdafruitIO_Feed *io_garage_temperature = io.feed("garagetemperature");
AdafruitIO_Feed *io_reset_reason = io.feed("resetreason");

OneWire oneWire(ONE_WIRE_PIN);
DallasTemperature DS18B20(&oneWire);


uint32_t greenMillisFlash = 0;
uint32_t redMillisFlash = 0;
uint32_t temperatureLastRead = 0;
#define FLASH_TIME_ON 50
#define MOVING_FLASH_TIME_OFF 450
#define MANUAL_FLASH_TIME_OFF 150
#define WAITING_TIME_OFF 4950

enum doorPositions{
  dpStartup,
  dpUnknown,
  dpOpen,
  dpClosed,
  dpOpenToClosed,
  dpClosedToOpen,
  dpManualOpenToClosed,
  dpManualClosedToOpen
};

char door_position_strings [][20]= {"STARTUP","UNKNOWN","OPEN","CLOSED","OPEN_CLOSED","CLOSED_OPEN","MANUAL_OPEN_CLOSED","MANUAL_CLOSED_OPEN"};

doorPositions door_position = dpStartup;
doorPositions start_move_door_position = dpStartup;
bool door_moving = false;


void handleDoorActionMessage(AdafruitIO_Data *data) {

  Serial.print(F("Action: Received door action -> "));
  Serial.println(data->value());

  doorPositions current_door_position = door_position;

  if (strcmp(data->value(), "OPEN") == 0)
  {
    if(door_position == dpClosed){
      Serial.println(F("Action: Door is closed, Open"));
      start_move_door_position = door_position;
      door_position = dpClosedToOpen;
      digitalWrite(LED_RED,LED_ON);
      digitalWrite(RELAY,RELAY_CLOSED);
      delay(500);
      digitalWrite(LED_RED,LED_OFF);
      digitalWrite(RELAY,RELAY_OPEN);
    }else{
      redMillisFlash = millis() + 1000;
      digitalWrite(LED_RED,LED_ON);
    }
  }
  if (strcmp(data->value(), "CLOSE") == 0)
  {
    if(door_position == dpOpen){
      Serial.println(F("Action: Door is open, Close"));
      start_move_door_position = door_position;
      door_position = dpOpenToClosed;
      digitalWrite(LED_RED,LED_ON);
      digitalWrite(RELAY,RELAY_CLOSED);
      delay(500);
      digitalWrite(LED_RED,LED_OFF);
      digitalWrite(RELAY,RELAY_OPEN);
    }else{
      redMillisFlash = millis() + 1000;
      digitalWrite(LED_RED,LED_ON);
    }
  }

  if (current_door_position != door_position){
    Serial.print(F("Action: Update Door Position:"));
    Serial.print(door_position_strings[current_door_position]);
    Serial.print(F(" --> "));
    Serial.println(door_position_strings[door_position]);
    io_door_position->save(door_position_strings[door_position]);
  }
}

void setup() {
  Serial.begin(115200);
  for(int i = 0; i < 10; i++){
    Serial.println();
  }

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(RELAY, OUTPUT);
  pinMode(SWITCH_OPEN,INPUT_PULLUP);
  pinMode(SWITCH_CLOSED,INPUT_PULLUP);

  digitalWrite(LED_RED,LED_ON);
  digitalWrite(LED_GREEN,LED_ON);
  digitalWrite(LED_BUILTIN,HIGH);

  Serial.println(F("Connecting to Adafruit IO"));
  WiFi.setAutoReconnect(true);
  WiFi.mode(WIFI_STA);

  io.connect();
  io_door_action->onMessage(handleDoorActionMessage);

  while(io.status() < AIO_NET_CONNECTED){
    Serial.print("Net Status ");
    Serial.println(io.statusText());
    WiFi.printDiag(Serial);
    delay(500);
  }

  Serial.println();
  Serial.println(io.statusText());
  digitalWrite(LED_GREEN,LED_OFF);
  digitalWrite(LED_RED,LED_OFF);

  io_reset_reason->save(ESP.getResetReason());

}

void read_switchs(){
  bool s_closed = false;
  bool s_open = false;

  uint8_t state = digitalRead(SWITCH_CLOSED);
  delay(20);
  if(state == digitalRead(SWITCH_CLOSED)){
    // closed switch debounced.
    s_closed = state == LOW;
  }
  
  state = digitalRead(SWITCH_OPEN);
  delay(20);
  if(state == digitalRead(SWITCH_OPEN)){
    // open switch debounced.
    s_open = state == LOW;
  }
  uint8_t current_door_position = door_position;
  
  switch(door_position){
    case dpStartup:
    case dpUnknown:
      if(s_closed){
        door_position = dpClosed;
        Serial.println(F("Startup Door CLOSED"));
        digitalWrite(LED_RED,LED_OFF);
      }else if(s_open){
        door_position = dpOpen;
        digitalWrite(LED_RED,LED_OFF);
        Serial.println(F("Startup Door OPEN"));
      }else{
        Serial.println(F("Startup Door UNKNOWN"));
        digitalWrite(LED_RED,LED_ON);
        delay(1000);
      }
      break;
    case dpClosedToOpen:
      if(!s_closed && ! door_moving){
        Serial.println(F("Switch: ClosedToOpen Door Moving"));
        door_moving = true;
      }
      if(s_open && door_moving){
        Serial.println(F("Switch: ClosedToOpen Door Open"));
        door_position = dpOpen;
        door_moving = false;
      }
      if(s_closed && door_moving){
        Serial.println(F("Switch: ClosedToOpen Door Closed"));
        door_position = dpClosed;
        door_moving = false;
      }
      break;
    case dpOpenToClosed:
      if(!s_open && !door_moving){
        Serial.println(F("Switch: OpenToClosed Door Moving"));
        door_moving = true;
      }
      if(s_open && door_moving){
        Serial.println(F("Switch: OpenToClosed Door Open"));
        door_position = dpOpen;
        door_moving = false;
      }
      if(s_closed && door_moving){
        Serial.println(F("Switch: OpenToClosed Door Closed"));
        door_position = dpClosed;
        door_moving = false;
      }
      break;
    case dpClosed:
      if(!s_closed && !door_moving){
        Serial.println(F("Switch: Closed -> Manual Movement Door Moving"));
        door_moving = true;
        door_position = dpManualClosedToOpen;
      }
      break;
    case dpOpen:
      if(!s_open && !door_moving){
        Serial.println(F("Switch: Open -> Manual Movement Door Moving"));
        door_moving = true;
        door_position = dpManualOpenToClosed;
      }
      break; 
    case dpManualClosedToOpen:
      if(s_open && door_moving){
        Serial.println(F("Switch: ManualClosedToOpen Door Open"));
        door_position = dpOpen;
        door_moving = false;
      }
      if(s_closed && door_moving){
        Serial.println(F("Switch: ManualClosedToOpen Door Closed"));
        door_position = dpClosed;
        door_moving = false;
      }
      break;
    case dpManualOpenToClosed:
      if(s_open && door_moving){
        Serial.println(F("Switch: ManualOpenToClosed Door Open"));
        door_position = dpOpen;
        door_moving = false;
      }
      if(s_closed && door_moving){
        Serial.println(F("Switch: ManualOpenToClosed Door Closed"));
        door_position = dpClosed;
        door_moving = false;
      }
      break;
  }
  if (current_door_position != door_position){
    Serial.print(F("Switch: Update Door Position:"));
    Serial.print(door_position_strings[current_door_position]);
    Serial.print(F(" --> "));
    Serial.println(door_position_strings[door_position]);
    io_door_position->save(door_position_strings[door_position]);
  }
  
  
}

void read_temperature(){
  if(temperatureLastRead == 0 || (millis() - temperatureLastRead) >= TEMPERATURE_DELAY ){
    ESP.wdtDisable();
    for(int d = 0; d < 5; d++) {
      DS18B20.requestTemperatures(); 
      float temp = DS18B20.getTempCByIndex(0);
      if(temp != 85.0 && temp != (-127.0)){
        Serial.print(F("Temperature: "));
        Serial.println(temp);
        io_garage_temperature->save(temp);
        break;
      }
      Serial.print(F("Temperature Reading Failed"));
      delay(1000);
    }
    temperatureLastRead = millis();
    ESP.wdtEnable((uint32_t)0);
  }
}

void loop() {
  io.run();
  read_switchs();
  read_temperature();

  if(door_position == dpOpen || door_position == dpClosed){
    if(millis() - greenMillisFlash >= WAITING_TIME_OFF && digitalRead(LED_GREEN) == LED_OFF){
      digitalWrite(LED_GREEN,LED_ON);
      greenMillisFlash = millis();
    }
    if(millis() - greenMillisFlash >= FLASH_TIME_ON && digitalRead(LED_GREEN) == LED_ON){
      digitalWrite(LED_GREEN,LED_OFF);
      greenMillisFlash = millis();
    }
  }


  if(door_position == dpClosedToOpen || door_position == dpOpenToClosed){
    if(millis() - greenMillisFlash >= MOVING_FLASH_TIME_OFF && digitalRead(LED_GREEN) == LED_OFF){
      digitalWrite(LED_GREEN,LED_ON);
      greenMillisFlash = millis();
    }
    if(millis() - greenMillisFlash >= FLASH_TIME_ON && digitalRead(LED_GREEN) == LED_ON){
      digitalWrite(LED_GREEN,LED_OFF);
      greenMillisFlash = millis();
    }
  }

  if(door_position == dpManualClosedToOpen || door_position == dpManualOpenToClosed){
    if(millis() - greenMillisFlash>= MANUAL_FLASH_TIME_OFF && digitalRead(LED_GREEN) == LED_OFF){
      digitalWrite(LED_GREEN,LED_ON);
      greenMillisFlash = millis();
    }
    if(millis() - greenMillisFlash >= FLASH_TIME_ON && digitalRead(LED_GREEN) == LED_ON){
      digitalWrite(LED_GREEN,LED_OFF);
      greenMillisFlash = millis();
    }
  }

  if(redMillisFlash >= millis()){
    digitalWrite(LED_RED,LED_OFF);
    redMillisFlash = 0;
  }

  delay(10);
}
