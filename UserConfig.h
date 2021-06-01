#pragma once
#include <queue>
#include "FTAction.h"
#include <freertos/task.h>
//#define MAKERFABTOUCH
#ifdef MAKERFABTOUCH

#define CUSTOM_TOUCH_SDA 26
#define CUSTOM_TOUCH_SCL 27
#define USECAPTOUCH
#define touchInterruptPin GPIO_NUM_0
#define SCREEN_ROTATION 1
#define INVERSE_Y_TOUCH
#define FLIP_TOUCH_AXIS

#elif defined(ARDUINO_TWATCH_BASE)
#include "axp20x.h"
#define CUSTOM_TOUCH_SDA 23
#define CUSTOM_TOUCH_SCL 32
#define USECAPTOUCH
#define touchInterruptPin GPIO_NUM_38
#define TFT_BL 12
#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 240
#define SCREEN_ROTATION 2
#define ILI9341_DRIVER
#else
// ------- Uncomment the define below if you want to use SLEEP and wake up on touch -------
// The pin where the IRQ from the touch screen is connected uses ESP-style GPIO_NUM_* instead of just pinnumber
#define touchInterruptPin GPIO_NUM_27
#define speakerPin GPIO_NUM_26
#define SCREEN_ROTATION 1
#define TFT_BL 32
#define TFT_BACKLIGHT_ON
#define USECAPTOUCH

#define ILI9341_DRIVER


// ------- Uncomment the define below if you want to use a piezo buzzer and specify the pin where the speaker is connected -------
//#define speakerPin 26
#endif
#ifndef touchInterruptPin
#define touchInterruptPin -1
#endif
#ifndef speakerPin
#define speakerPin -1
#endif
#if defined(ST7789_DRIVER) || defined(ST7735_DRIVER) || defined(ILI9163_DRIVER)
#define TFT_HEIGHT SCREEN_HEIGHT
#define TFT_WIDTH SCREEN_WIDTH
#endif
#ifndef SCREEN_ROTATION 
#define SCREEN_ROTATION 1
#endif
#ifndef INVERSE_Y_TOUCH
#define INVERSE_Y_TOUCH false
#else 
#undef INVERSE_Y_TOUCH
#define INVERSE_Y_TOUCH true
#endif

#ifndef INVERSE_X_TOUCH
#define INVERSE_X_TOUCH false
#else 
#undef INVERSE_X_TOUCH
#define INVERSE_X_TOUCH true
#endif

#ifndef FLIP_TOUCH_AXIS
#define FLIP_TOUCH_AXIS false
#else 
#undef FLIP_TOUCH_AXIS
#define FLIP_TOUCH_AXIS true
#endif



// the following can be uncommented in case the coordinates of the
// X or Y axis touch panel are reversed
// #define INVERSE_X_TOUCH
// #define INVERSE_Y_TOUCH

//Struct to hold the general config like colours.
struct Config
{
  uint16_t menuButtonColour;
  uint16_t functionButtonColour;
  uint16_t backgroundColour;
  uint16_t latchedColour;
  uint8_t colscount;
  uint8_t rowscount;
  bool sleepenable;
  uint16_t sleeptimer;
  bool beep;
  bool flip_touch_axis;
  bool reverse_x_touch;
  bool reverse_y_touch;
  uint8_t screenrotation;
  uint8_t modifier1;
  uint8_t modifier2;
  uint8_t modifier3;
  uint16_t helperdelay;
  uint16_t DefaultOutline;
  uint8_t DefaultTextSize;
  uint16_t DefaultTextColor; 
  bool moreLogs; 
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
void drawErrorMessage(bool stop, const char *module, const char *fmt, ...);