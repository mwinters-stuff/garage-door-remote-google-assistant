#ifndef _JSONFILEBASE_H
#define _JSONFILEBASE_H
#include <Arduino.h>
#include <ArduinoJson.h>

class JSONFileBase{
  protected:
    String fileName;
  public:
    JSONFileBase(const String& fileName);
    virtual ~JSONFileBase(){};
    bool init();
    bool readFile();
    void saveFile();
    virtual void getJson(JsonDocument & root) = 0;
    virtual void setJson(const JsonDocument &object) = 0;

};


#endif
