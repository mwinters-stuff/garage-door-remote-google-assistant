#ifndef _IOHANDLER_H
#define _IOHANDLER_H

#include <Arduino.h>
#include <Button.h>
#include <ButtonEventCallback.h>
#include <PushButton.h>
#include <Bounce2.h> 

#include "config.h"
#include <memory>

#define FLASH_TIME_ON 50
#define MOVING_FLASH_TIME_OFF 450
#define MANUAL_FLASH_TIME_OFF 150
#define WAITING_TIME_OFF 4950

class IOHandler{
  public:
    IOHandler();

    void setup();

    static std::shared_ptr<IOHandler> get();
    static std::shared_ptr<IOHandler> init();


    void ledGreen(bool on);
    void ledRed(bool on);

    void update();
    void actionDoor(String position);

    void onButtonPress(Button &btn);
    void onButtonRelease(Button &btn, uint16_t duration);
    void onOpenCloseButtonHeld(Button& btn, uint16_t duration);

  private:

    void onOpenCloseButtonRelease(uint16_t duration);

    void onOpenSwitchPress();
    void onOpenSwitchRelease();

    void onClosedSwitchPress();
    void onClosedSwitchRelease();

    PushButton buttonOpenClose;
    PushButton buttonOpenSwitch;
    PushButton buttonClosedSwitch;
    uint32_t greenMillisFlash;
    uint32_t redMillisFlash;

    void processSwitchs(bool s_open, bool s_closed);
    void toggleRelay();


    static std::shared_ptr<IOHandler> m_instance;


};

#endif