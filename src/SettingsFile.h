#ifndef _SETTINGS_H
#define _SETTINGS_H
#include "JSONFileBase.h"

class SettingsFile: public JSONFileBase{
  public:

    SettingsFile();

    void getJson(JsonDocument & root) override;
    void setJson(const JsonDocument &object) override;
    void setClosed(bool is_closed) {
        this->is_closed = is_closed; 
        saveFile();
    };
    bool isClosed(){return is_closed;};
    bool isOpen(){return !is_closed;};

    bool isLocked(){return is_locked;};
    void setLocked(){ 
      is_locked = true;
      saveFile();
    };
    void setUnLocked(){ 
      is_locked = false;
      saveFile();
    };
    
    double getTemperature(){ return temperature;};
    void setTemperature(double temperature){ this->temperature = temperature;};
  private:
    bool   is_locked;
    bool   is_closed;
    double temperature;
    

};


#endif
