#pragma once
#include <queue>
#include "FTAction.h"
#include <freertos/task.h>

#ifdef ARDUINO_TWATCH_BASE
#include "axp20x.h"
    #define CUSTOM_TOUCH_SDA 23
    #define CUSTOM_TOUCH_SCL 32
    #define USECAPTOUCH
    #define touchInterruptPin GPIO_NUM_38
    #define TFT_BL   12
    #define SCREEN_WIDTH 240
    #define SCREEN_HEIGHT 240
    #define SCREEN_ROTATION 2
#else 
// ------- Uncomment the define below if you want to use SLEEP and wake up on touch -------
// The pin where the IRQ from the touch screen is connected uses ESP-style GPIO_NUM_* instead of just pinnumber
#define touchInterruptPin GPIO_NUM_27
#define SCREEN_ROTATION 1


// ------- Uncomment the define below if you want to use a piezo buzzer and specify the pin where the speaker is connected -------
//#define speakerPin 26


#endif
#ifndef touchInterruptPin
#define touchInterruptPin -1
#endif
#ifndef speakerPin
#define speakerPin -1
#endif
//Struct to hold the general config like colours.
struct Config
{
  uint16_t menuButtonColour;
  uint16_t functionButtonColour;
  uint16_t backgroundColour;
  uint16_t latchedColour;
  bool sleepenable;
  uint16_t sleeptimer;
  bool beep;
  uint8_t modifier1;
  uint8_t modifier2;
  uint8_t modifier3;
  uint16_t helperdelay;
};


// Define the storage to be used. For now just SPIFFS.
#define FILESYSTEM SPIFFS

// Text Button Label Font
#define LABEL_FONT &FreeSansBold12pt7b

// Font size multiplier
#define KEY_TEXTSIZE 1

enum class Sounds 
{
  GOING_TO_SLEEP,
  BEEP,
  STARTUP
};

#define LED_BRIGHTNESS_INCREMENT 25

extern void HandleAudio(Sounds sound);
extern Config generalconfig;
extern void drawErrorMessage(String message);
extern void drawErrorMessageChar(String message);
void drawErrorMessage(bool stop, const char * module, const char * fmt,...);