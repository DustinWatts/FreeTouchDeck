
/*
  Author: Dustin Watts
  Date: 27-08-2020

  My thanks goes out to Brian Lough, Colin Hickey, and the people on my Discord server
  for helping me a lot with the code and troubleshooting! https://discord.gg/RE3XevS

  FreeTouchDeck is based on the FreeDeck idea by Koriwi. It uses the TFT_eSPI library 
  by Bodmer for the display and touch functionality and it uses an ESP32-BLE-Keyboard fork 
  with a few modifications. For saving and loading configuration it uses ArduinoJson V6.

  FreeTouchDeck uses some libraries from other sources. These must be installed for 
  FreeTouchDeck to compile and run. 
  
  These are those libraries:

      !----------------------------- Library Dependencies --------------------------- !
      - Adafruit-GFX-Library (version 1.10.0 or above), available through Library Manager
      - TFT_eSPI (version 2.2.14 or above), available through Library Manager
      - ESP32-BLE-Keyboard (forked) (latest version) download from: https://github.com/DustinWatts/ESP32-BLE-Keyboard
      - ESPAsyncWebserver (latest version) download from: https://github.com/me-no-dev/ESPAsyncWebServer
      - AsyncTCP (latest version) download from: https://github.com/me-no-dev/AsyncTCP
      - ArduinoJson (version 6.16.1 or above), available through Library Manager

      --- If you use Capacitive touch ---
      - Dustin Watts FT6236 Library (version 1.0.1), https://github.com/DustinWatts/FT6236
      
  The FILESYSTEM (SPI FLASH filing system) is used to hold touch screen calibration data.
  It has to be runs at least once when using resistive touch. After that you can set 
  REPEAT_CAL to false (default).

  !-- Make sure you have setup your TFT display and ESP setup correctly in TFT_eSPI/user_setup.h --!
        
        Select the right screen driver and the board (ESP32 is the only one tested) you are
        using. Also make sure TOUCH_CS is defined correctly. TFT_BL is also be needed!

        You can find examples of User_Setup.h in the "user_setup.h Examples" folder.
  
*/

// ------- Uncomment the next line if you use capacitive touch -------
// (THE ESP32 TOUCHDOWN USES THIS!)
//#define USECAPTOUCH

// ------- Uncomment and populate the following if your cap touch uses custom i2c pins -------
//#define CUSTOM_TOUCH_SDA 26
//#define CUSTOM_TOUCH_SCL 27

// PAY ATTENTION! Even if resistive touch is not used, the TOUCH pin has to be defined!
// It can be a random unused pin.
// TODO: Find a way around this!
#define ACTIONS_IN_TASKS
#include "UserConfig.h"

#ifdef ARDUINO_TWATCH_BASE
AXP20X_Class *power = new AXP20X_Class();
#endif
static const char *module = "FreeTouchDeck";
const char *versionnumber = "0.9.11";

/* Version 0.9.11.
     *  
     * Fix: F21 in the configurator was F22 as action and F22 was missing from configurator.
     * Fix: "[WARNING]: SPIFFS initialisation failed!" is now "[ERROR]: SPIFFS initialisation failed!"
     * Fix: "Back home" and "Settings" buttons now also draw custom logos correctly.
     * Fix: Some function descriptions and general typos.
     * Fix: default.json was ignored by .gitignore and therefore not previously uploaded.
     * 
     * Change: Moved TFT init before SPIFFS init, so we can draw an error message on screen if SPIFFS
     *         init fails.
     * Change: Moved to a wificonfig.json just for wifi settings and a general.json for all other general
     *         config.
     * 
     * Added: Helpers are configurable so that it only takes 1 action. Case 10 Action.h.
     *        Settings for this added to "Settings" in the configurator
     * Added: . (full stop) , (comma) and - (minus) added to the configurator under "Special chars"
     *        
     * 
     * Important! This version changes json structure and HTML files. Config/data files changed:
     * 
     * - wificonfig.json (there are some settings added)
     * - general.json (colors, sleep, and beep are moved here)
     * - index.html
     * 
     * Make sure to check if you use your old config files that they match the structure of the new ones!
    */


#include <pgmspace.h> // PROGMEM support header
#include "Arduino.h"
#include <SPI.h>
// 
#include "SPIFFS.h"
#ifdef SDDAT3
#include "SD.h"
#endif

#include <TFT_eSPI.h> // The TFT_eSPI library
#include <BleKeyboard.h>       // BleKeyboard is used to communicate over BLE
#include "BLEDevice.h"         // Additional BLE functionaity
#include "BLEUtils.h"          // Additional BLE functionaity
#include "BLEBeacon.h"         // Additional BLE functionaity
#include "esp_sleep.h"         // Additional BLE functionaity
#include "esp_bt_main.h"       // Additional BLE functionaity
#include "esp_bt_device.h"     // Additional BLE functionaity
#include "globals.hpp"
#include "ConfigHelper.h"
#include "Storage.h"
#include "System.h"
#include "Console.h"


#include "Menu.h"
#include "cJSON.h"
#include "ImageWrapper.h"
using namespace FreeTouchDeck;
// ----------------------------------------------
// It is possible to define you own custom action
// to perform various actions on the ESP32 itself
// (e.g. send the device to sleep, measure temperature
// and send the result out as keypresses, etc.) and assign them
// to buttons!
// See examples under UserActions.h
//
#include "UserActions.h"

BleKeyboard bleKeyboard("FreeTouchDeck", "Made by me");
// Create instances of the structs
Wificonfig wificonfig = {.ssid = NULL, .password = NULL, .wifimode = NULL, .hostname = NULL};

#ifdef ACTIONS_IN_TASKS
//TaskHandle_t xScreenTask = NULL;
//void ScreenHandleTask(void *pvParameters);
TaskHandle_t xActionTask = NULL;
void ActionTask(void *pvParameters);
#endif


// Checking for BLE Keyboard version
#ifndef BLE_KEYBOARD_VERSION
#warning Old BLE Keyboard version detected. Please update.
#define BLE_KEYBOARD_VERSION "Outdated"
#endif

#include "ConfigLoad.h"
#include "DrawHelper.h"
#include "ConfigHelper.h"
#include "MenuNavigation.h"


//-------------------------------- SETUP --------------------------------------------------------------

void setup()
{
  // Use serial port
  Serial.begin(115200);
  PrintMemInfo(__FUNCTION__, __LINE__);
  SetGeneralConfigDefaults();
  Serial.setDebugOutput(true);
  LOC_LOGI(module, "Starting system.");
  PrintMemInfo(__FUNCTION__, __LINE__);
  // Init Touch first. We will use it to restart the system when displaying a hard error
  InitSystem();
  
PrintMemInfo(__FUNCTION__, __LINE__);
  //------------------BLE Initialization ------------------------------------------------------------------------
  LOC_LOGI(module, "Starting BLE Keyboard");
  bleKeyboard.deviceName = generalconfig.deviceName;
  bleKeyboard.deviceManufacturer = generalconfig.manufacturer;
  bleKeyboard.begin();
PrintMemInfo(__FUNCTION__, __LINE__);
  // ---------------- Printing version numbers -----------------------------------------------
  LOC_LOGI(module, "BLE Keyboard version: %s", BLE_KEYBOARD_VERSION);
  LOC_LOGI(module, "ArduinoJson version: %s", ARDUINOJSON_VERSION);
  LOC_LOGI(module, "TFT_eSPI version: %s", TFT_ESPI_VERSION);
  LOC_LOGI(module, "SPI Flash frequency: %s", CONFIG_ESPTOOLPY_FLASHFREQ);

  // ---------------- Start the first keypad -------------

  // Draw background
  if (!GetActiveScreen())
  {
    // only set active screen to home if no screen is already active
    // for example don't active home if drawError was called
    SetActiveScreen("home");
  }
  PrintMemInfo(__FUNCTION__, __LINE__);
  HandleSleepConfig();
  HandleBeepConfig();
#ifdef ACTIONS_IN_TASKS
  // xTaskCreate(ScreenHandleTask, "Screen", 1024 * 3, NULL, tskIDLE_PRIORITY + 8, &xScreenTask);
  // PrintMemInfo(__FUNCTION__, __LINE__);
  // LOC_LOGD(module, "Screen task created");
  xTaskCreate(ActionTask, "Action", 1024 * 4, NULL, tskIDLE_PRIORITY + 5, &xActionTask);
  PrintMemInfo(__FUNCTION__, __LINE__);
  LOC_LOGD(module, "Action task created");
#endif
}

//--------------------- LOOP ---------------------------------------------------------------------



void loop(void)
{

  processSerial();
  processSleep();
HandleScreen();
#ifndef ACTIONS_IN_TASKS
  HandleActions();
#endif
  // screen debounce
  delay(10);
}
#ifdef ACTIONS_IN_TASKS
// void ScreenHandleTask(void *pvParameters)
// {

//   // Load menu definitions

//   for (;;)
//   {
//     HandleScreen();
//     delay(50);
//   }
// }

void ActionTask(void *pvParameters)
{
  for (;;)
  {
    HandleActions();
    delay(50);
  }
}
#endif