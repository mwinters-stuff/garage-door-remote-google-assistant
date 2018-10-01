#include "JSONFileBase.h"
#include <FS.h>

JSONFileBase::JSONFileBase(const String& fileName):
      fileName(fileName){

  if(!readFile()){
    Serial.print(F("Failed to read "));
    Serial.println(fileName);
    while(true){
      delay(1000);
    }
  }
}


bool JSONFileBase::readFile()
{
  bool result = false;
  if (SPIFFS.begin())
  {
    Serial.println("mounted file system");
    if (SPIFFS.exists(fileName))
    {
      //file exists, reading and loading
      Serial.print(F("reading file"));
      Serial.println(fileName);
      File configFile = SPIFFS.open(fileName, "r");
      if (configFile)
      {
        DynamicJsonBuffer jsonBuffer;
        JsonObject &json = jsonBuffer.parseObject(configFile);
        json.printTo(Serial);

        if (json.success())
        {
          setJson(json);
          result = true;
        }
        else
        {
          Serial.println(F("failed to load json"));
        }
      }
      configFile.close();
    }
  }
  return result;
}

void JSONFileBase::saveFile(){
  Serial.print(F("saving file "));
  Serial.println(fileName);
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();

  getJson(json);

  File configFile = SPIFFS.open(fileName, "w");
  if (!configFile) {
    Serial.println(F("failed to open config file for writing"));
    return;
  }

  json.printTo(configFile);
  configFile.close();
}
