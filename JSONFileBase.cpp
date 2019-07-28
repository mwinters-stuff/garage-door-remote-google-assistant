#include "JSONFileBase.h"
#include <FS.h>

JSONFileBase::JSONFileBase(const String& fileName):
      fileName(fileName){
}


bool JSONFileBase::readFile()
{
  bool result = false;
  if (SPIFFS.begin())
  {
    Serial.println(F("mounted file system"));
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

        if (json.success())
        {
          json.printTo(Serial);
          Serial.println();

          setJson(json);

          result = true;
        }
        else
        {
          Serial.println(F("failed to load json"));
        }
      }
      configFile.close();
    }else{
      Serial.println(F("File doesnt exist"));
    }
  }else{
    Serial.println(F("SPIFFS.begin failed"));
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
