
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <AdafruitIO_WiFi.h>

#include <AdafruitIO_Feed.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>
#include <EEPROM.h>
#include <FS.h>

#include "config.h"

#define EEPROM_SIZE 16
#define  EEPROM_IN_GEOFENCE_ADDR 0

ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;

AdafruitIO_WiFi io(IO_USERNAME, IO_KEY, WIFI_SSID, WIFI_PASS);
AdafruitIO_Feed* io_door_action = io.feed(IO_FEED_DOOR_ACTION);
AdafruitIO_Feed* io_door_position = io.feed(IO_FEED_POSITION);
AdafruitIO_Feed* io_garage_temperature = io.feed(IO_FEED_TEMPERATURE);
AdafruitIO_Feed* io_reset_reason = io.feed(IO_FEED_RESET_REASON);
#ifdef GEOFENCE
AdafruitIO_Feed* io_in_home_area = io.feed(IO_FEED_IN_HOME_AREA);
#endif


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

static const char door_position_strings [][20] PROGMEM = {"STARTUP","UNKNOWN","OPEN","CLOSED","OPEN_CLOSED","CLOSED_OPEN","MANUAL_OPEN_CLOSED","MANUAL_CLOSED_OPEN"};

doorPositions door_position = dpStartup;
doorPositions start_move_door_position = dpStartup;
bool door_moving = false;


void handleDoorActionMessage(AdafruitIO_Data *data);
String doDoorAction(String action);
void actionDoor(String position);

#ifdef GEOFENCE
bool in_geofence = false;
void handleInHomeAreaMessage(AdafruitIO_Data *data);
String doInHomeArea(String action);
#endif

bool handleFileRead(String path){
  Serial.println("handleFileRead: " + path);
  if(path.endsWith("/")) path += "index.html";
  String contentType = "text/html";
  String pathWithGz = path + ".gz";
  if(SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)){
    if(SPIFFS.exists(pathWithGz))
      path += ".gz";
    File file = SPIFFS.open(path, "r");
    httpServer.streamFile(file, contentType);
    file.close();
    return true;
  }
  return false;
}

void setup() {
  Serial.begin(115200);
  for(int i = 0; i < 10; i++){
    Serial.println();
  }
  SPIFFS.begin();

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
  WiFi.hostname(HOSTNAME);
  WiFi.setAutoReconnect(true);
  WiFi.mode(WIFI_STA);

  io.connect();
  io_door_action->onMessage(handleDoorActionMessage);
#ifdef GEOFENCE  
  io_in_home_area->onMessage(handleInHomeAreaMessage);
  EEPROM.begin(EEPROM_SIZE);
  in_geofence = EEPROM.read(EEPROM_IN_GEOFENCE_ADDR) == 1;
  Serial.println(in_geofence ? F("restored inside geofence") : F("restored outside geofence"));
  EEPROM.end();
#endif  

  while(io.status() < AIO_NET_CONNECTED){
    Serial.print(F("Net Status "));
    Serial.println(io.statusText());
    // WiFi.printDiag(Serial);
    delay(500);
  }

  Serial.println();
  Serial.println(io.statusText());
  digitalWrite(LED_GREEN,LED_OFF);
  digitalWrite(LED_RED,LED_OFF);

  io_reset_reason->save(ESP.getResetReason());
  Serial.println(io_door_action->lastValue()->toString());


  MDNS.begin(HOSTNAME);

  httpUpdater.setup(&httpServer);
  
  httpServer.on("/all", HTTP_GET, [](){
    httpServer.sendHeader("Access-Control-Allow-Origin","*");
    String json = "{";
    json += "\"heap\": " + String(ESP.getFreeHeap());
    json += ", \"door_action\": \"" + io_door_action->data->toString() + "\"";
    json += ", \"door_position\": \"" + io_door_position->data->toString() + "\"";
#ifdef GEOFENCE
    json += ", \"in_home_area\": ";
    if(in_geofence == 1){
      json += "true";
    } else{
      json += "false";
    } 
#endif
    json += "}";
    httpServer.send(200, "text/json", json);
    json = String();
  });

  httpServer.on("/action", HTTP_GET,[](){
    httpServer.sendHeader("Access-Control-Allow-Origin","*");
    if(httpServer.args() == 0) 
      return httpServer.send(500, "text/plain", "BAD ARGS");
    String name= httpServer.argName(0);
    String action = httpServer.arg(0);

    Serial.print(F("Action: "));
    Serial.print(name);
    Serial.print(" = ");
    Serial.println(action);
    String result = "ERROR: " + name + ": " + action + " is an unknown action";

    if(name.equals("door_action")){
      result = doDoorAction(action);
    }

#ifdef GEOFENCE
    if(name.equals("in_home_area")){
      result = doInHomeArea(action);
    }
#endif
    httpServer.send(200, "text/plain", result);

  });

  //called when the url is not defined here
  //use it to load content from SPIFFS
  httpServer.onNotFound([](){
    if(!handleFileRead(httpServer.uri()))
      httpServer.send(404, "text/plain", "FileNotFound");
  });

  httpServer.begin();

  MDNS.addService("http", "tcp", 80);
  Serial.printf("HTTPUpdateServer ready! Open http://%s/update in your browser\n", HOSTNAME);

}

#ifdef GEOFENCE

void handleInHomeAreaMessage(AdafruitIO_Data *data) {

  Serial.print(F("Action: Received in home area -> "));
  Serial.print(data->value());

  in_geofence = strcmp_P(data->value(), PSTR("entered")) == 0;
  Serial.println(in_geofence ? F(" inside geofence") : F(" outside geofence"));

  EEPROM.begin(EEPROM_SIZE);
  EEPROM.write(EEPROM_IN_GEOFENCE_ADDR, in_geofence ? 1 : 0);
  EEPROM.end();

}
#endif

void handleDoorActionMessage(AdafruitIO_Data *data) {

  Serial.print(F("Action: Received door action -> "));
  Serial.println(data->value());

#ifdef GEOFENCE
  if(!in_geofence){
    Serial.println(F("Not near home, won't action door!"));
    return;
  }
#endif

  actionDoor(data->value());
}

void actionDoor(String position){
  doorPositions current_door_position = door_position;

  if (position.compareTo(F("OPEN")) == 0)
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
  if (position.compareTo(F("CLOSE")) == 0)
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
    Serial.print(FPSTR(door_position_strings[current_door_position]));
    Serial.print(F(" --> "));
    Serial.println(FPSTR(door_position_strings[door_position]));
    io_door_position->save(String(FPSTR(door_position_strings[door_position])));
  }
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
    Serial.print(FPSTR(door_position_strings[current_door_position]));
    Serial.print(F(" --> "));
    Serial.println(FPSTR(door_position_strings[door_position]));
    io_door_position->save(String(FPSTR(door_position_strings[door_position])));
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
      Serial.println(F("Temperature Reading Failed"));
      delay(1000);
    }
    temperatureLastRead = millis();
    ESP.wdtEnable((uint32_t)0);
  }
}

void loop() {
  io.run();
  httpServer.handleClient();
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

String doDoorAction(String action){
  if(action.compareTo(F("OPEN")) == 0 || action.compareTo(F("CLOSE"))==0){
    io_door_action->save(action);
    actionDoor(action);
    return "OK";
  }

  return "ERROR: " + action + " is not a door action";
}
#ifdef GEOFENCE
String doInHomeArea(String action){
  return "ERROR: " + action + " is not in home action";
}
#endif
