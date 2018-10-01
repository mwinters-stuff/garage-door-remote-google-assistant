#ifndef _JSONFILEBASE_H
#define _JSONFILEBASE_H
#include <Arduino.h>
#include <ArduinoJson.h>

class JSONFileBase{
  private:
    String fileName;
  public:
    JSONFileBase(const String& fileName);
    virtual ~JSONFileBase(){};

    bool readFile();
    void saveFile();
    virtual void getJson(JsonObject & root) = 0;
    virtual void setJson(const JsonObject &object) = 0;

};


#endif
