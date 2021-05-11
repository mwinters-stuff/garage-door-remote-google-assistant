#include "JSONFileBase.h"
#include "StringConstants.h"
#include <LittleFS.h>

JSONFileBase::JSONFileBase(const String& fileName):
      fileName(fileName){
}

bool JSONFileBase::init(){
  if(!readFile()){
    Serial.printf(String(FAILED_TO_READ).c_str(), fileName.c_str());
    return false;
  }
  return true;
}

bool JSONFileBase::readFile()
{
  bool result = false;
  Serial.println(F("mounted file system"));
  if (LittleFS.exists(fileName))
  {
    //file exists, reading and loading
    Serial.print(F("reading file "));
    Serial.println(fileName);
    File configFile = LittleFS.open(fileName, "r");
    if (configFile)
    {
      DynamicJsonDocument doc(1500);
      auto error = deserializeJson(doc, configFile);

      if (!error)
      {
        serializeJsonPretty(doc, Serial);
        Serial.println();
        setJson(doc);
        Serial.println(F("Set"));
        result = true;
      }
      else
      {
        Serial.println(F("failed to load json "));
        Serial.println(error.c_str());
      }
    }
    configFile.close();
  }
  return result;
}

void JSONFileBase::saveFile(){
  Serial.print(F("saving file "));
  Serial.println(fileName);
  DynamicJsonDocument doc(1500);

  getJson(doc);

  File configFile = LittleFS.open(fileName, "w");
  if (!configFile) {
    Serial.println(F("failed to open config file for writing"));
    return;
  }

  serializeJson(doc, configFile);
  
  configFile.close();
}
