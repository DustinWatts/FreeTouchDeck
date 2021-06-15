#pragma once

/* ------------------------------------------------------------------------ */
/* Select the board that you are using below. Make sure to only select one! */
/* ------------------------------------------------------------------------ */

//#define MAKERFABTOUCH
#define ESP32TOUCHDOWN
//#define ESP32DEVKIT
//#define ARDUINO_TWATCH

/* ------------------------------------------------------------------------ */
/* Board specific config. No need to touch if you selected the right board  */
/* ------------------------------------------------------------------------ */

/* MakerFab's Touchscreen configuration */
#ifdef MAKERFABTOUCH
#define CUSTOM_TOUCH_SDA 26
#define CUSTOM_TOUCH_SCL 27
#define USECAPTOUCH
#define touchInterruptPin GPIO_NUM_0
#define SCREEN_ROTATION 1
#define INVERSE_Y_TOUCH
#define FLIP_TOUCH_AXIS

/* Arduino TWatch Configuration */
#elif defined(ARDUINO_TWATCH_BASE) && defined(ARDUINO_TWATCH)
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

/* ESP32 TouchDown Configuration */
#elif defined(ESP32TOUCHDOWN)
#define touchInterruptPin GPIO_NUM_27
#define speakerPin GPIO_NUM_26
#define SCREEN_ROTATION 1
#define FLIP_TOUCH_AXIS
#define INVERSE_Y_TOUCH
#define USECAPTOUCH
#define SDDAT3 25 
/* Using the ESP32 DevKit with a screen module */
#elif defined(ESP32DEVKIT)
#define SCREEN_ROTATION 1
#define FLIP_TOUCH_AXIS
#define INVERSE_Y_TOUCH
#define touchInterruptPin GPIO_NUM_27
#else 
#error("Unsupported platform")
#endif

/* --- END OF USER CONFIG --- */

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



// Text Button Label Font
#define LABEL_FONT &FreeSansBold12pt7b

// Font size multiplier
#define KEY_TEXTSIZE 1

// This is the file name used to store the calibration data
// You can change this to create new calibration files.
// The FILESYSTEM file name must start with "/".
#define CALIBRATION_FILE "/TouchCalData"

// Set REPEAT_CAL to true instead of false to run calibration
// again, otherwise it will only be done once.
// Repeat calibration if you change the screen rotation.
#define REPEAT_CAL false

#define LED_BRIGHTNESS_INCREMENT 25

// percentage of free memory that the system will use as
// a temporary buffer when drawing images. Warning! 
// too much buffer will lead to system instabilities. 
#define BITMAP_BUFFER_FREE_RAM_PCT 0.30
