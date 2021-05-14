
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

#include "UserConfig.h"

#ifdef ARDUINO_TWATCH_BASE
AXP20X_Class *power = new AXP20X_Class();
#endif

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
#include <FS.h>       // Filesystem support header
#include <SPIFFS.h>   // Filesystem support header

#include <TFT_eSPI.h> // The TFT_eSPI library

#include <BleKeyboard.h> // BleKeyboard is used to communicate over BLE
#include "BLEDevice.h"   // Additional BLE functionaity
#include "BLEUtils.h"    // Additional BLE functionaity
#include "BLEBeacon.h"   // Additional BLE functionaity
#include "esp_sleep.h"   // Additional BLE functionaity

#include "esp_bt_main.h"   // Additional BLE functionaity
#include "esp_bt_device.h" // Additional BLE functionaity

#include <ArduinoJson.h> // Using ArduinoJson to read and write config files

#include <WiFi.h> // Wifi support

#include <AsyncTCP.h>          //Async Webserver support header
#include <ESPAsyncWebServer.h> //Async Webserver support header

#include <ESPmDNS.h> // DNS functionality
#include <map>
#include <list>
#include <queue>
#include <functional>
#ifdef USECAPTOUCH
#include <Wire.h>
#include <FT6236.h>
FT6236 ts = FT6236();
#endif
#include "Menu.h"

BleKeyboard bleKeyboard("FreeTouchDeck", "Made by me");

AsyncWebServer webserver(80);

TFT_eSPI tft = TFT_eSPI();

// This is the file name used to store the calibration data
// You can change this to create new calibration files.
// The FILESYSTEM file name must start with "/".
#define CALIBRATION_FILE "/TouchCalData"

// Set REPEAT_CAL to true instead of false to run calibration
// again, otherwise it will only be done once.
// Repeat calibration if you change the screen rotation.
#define REPEAT_CAL false

// placeholder for the pagenumber we are on (0 indicates home)
int pageNum = 0;

// Initial LED brightness
int ledBrightness = 255;
//path to the directory the logo are in ! including leading AND trailing / !
char logopath[64] = "/logos/";

struct Wificonfig
{
  char *ssid;
  char *password;
  char *wifimode;
  char *hostname;
  uint8_t attempts;
  uint16_t attemptdelay;
};

// Create instances of the structs
Wificonfig wificonfig = {.ssid = NULL, .password = NULL, .wifimode = NULL, .hostname = NULL};

Config generalconfig;

unsigned long previousMillis = 0;
unsigned long Interval = 0;
bool displayinginfo;
char *jsonfilefail = "";



TaskHandle_t xScreenTask = NULL;
void ScreenHandleTask(void *pvParameters);
TaskHandle_t xActionTask = NULL;
void ActionTask(void *pvParameters);

SemaphoreHandle_t xQueueSemaphore = xSemaphoreCreateMutex();
std::queue<FreeTouchDeck::FTAction *> Queue;
bool QueueLock(TickType_t xTicksToWait)
{
  //    ESP_LOGV(TAG, "Locking config json object");
  if (xSemaphoreTake(xQueueSemaphore, xTicksToWait) == pdTRUE)
  {
    //      ESP_LOGV(TAG, "config Json object locked!");
    return true;
  }
  else
  {
    Serial.println("[ERROR]: Unable to lock the Action queue object");
    return false;
  }
}

void QueueUnlock()
{
  xSemaphoreGive(xQueueSemaphore);
}
bool QueueAction(FreeTouchDeck::FTAction *action)
{
  if (!QueueLock(100 / portTICK_PERIOD_MS))
  {
    Serial.println("[ERROR]: Unable to queue new action ");
    return false;
  }
  Queue.push(action);
  QueueUnlock();
  return true;
}



// Checking for BLE Keyboard version
#ifndef BLE_KEYBOARD_VERSION
#warning Old BLE Keyboard version detected. Please update.
#define BLE_KEYBOARD_VERSION "Outdated"
#endif

//--------- Internal references ------------
// (this needs to be below all structs etc..)
#include "ScreenHelper.h"
#include "ConfigLoad.h"
#include "DrawHelper.h"
#include "ConfigHelper.h"
#include "Webserver.h"
#include "MenuNavigation.h"
#include "BMPImage.h"
#ifndef USECAPTOUCH
#include "Touch.h"
#endif



void DrawSplash()
{
  FreeTouchDeck::BMPImage* splash=FreeTouchDeck::GetImage("freetouchdeck_logo.bmp");
  if(splash)
  {
//    splash->Draw(&tft,0,0,false);
  }
  else 
  {
    Serial.println("[ERROR]: Unable to draw the splash screen.");
  }
  
}

//-------------------------------- SETUP --------------------------------------------------------------

void setup()
{

  // Use serial port
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.printf("Main Task size is %d\r", CONFIG_ARDUINO_LOOP_STACK_SIZE);
#ifdef ARDUINO_TWATCH_BASE
  Serial.println("[INFO]: Enabling power ");
  Wire1.begin(21, 22);
  int ret = power->begin(Wire1, AXP202_SLAVE_ADDRESS, false);
  if (ret == AXP_FAIL)
  {
    Serial.println("[ERROR]: AXP Power begin failed");
  }
  else
  {
    // todo: implement better power management for
    // watch
    //Change the shutdown time to 4 seconds
    //power->setShutdownTime(AXP_POWER_OFF_TIME_4S);
    // Turn off the charging instructions, there should be no
    //power->setChgLEDMode(AXP20X_LED_OFF);
    // Turn off external enable
    //power->setPowerOutPut(AXP202_EXTEN, false);
    //axp202 allows maximum charging current of 1800mA, minimum 300mA
    power->setChargeControlCur(300);
  }
  power->setPowerOutPut(AXP202_LDO2, AXP202_ON);
  Serial.println("[INFO]: Setting up Display Back light");
#endif
#ifdef USECAPTOUCH
#ifdef CUSTOM_TOUCH_SDA
  if (!ts.begin(40, CUSTOM_TOUCH_SDA, CUSTOM_TOUCH_SCL))
#else
  if (!ts.begin(40))
#endif
  {
    Serial.println("[WARNING]: Unable to start the capacitive touchscreen.");
  }
  else
  {
    Serial.println("[INFO]: Capacitive touch started!");
  }
#endif

  // Setup PWM channel and attach pin 32
  ledcSetup(0, 5000, 8);
#ifdef TFT_BL
  ledcAttachPin(TFT_BL, 0);
#else
  ledcAttachPin(32, 0);
#endif
  ledcWrite(0, ledBrightness); // Start @ initial Brightness

  // --------------- Init Display -------------------------

  // Initialise the TFT screen
  tft.init();

  // Set the rotation before we calibrate
  tft.setRotation(SCREEN_ROTATION);

  // Clear the screen
  tft.fillScreen(TFT_BLACK);
  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();

  // -------------- Start filesystem ----------------------

  if (!FILESYSTEM.begin())
  {
    Serial.println("[ERROR]: SPIFFS initialisation failed!");
    drawErrorMessage("Failed to init SPIFFS! Did you upload the data folder?");
    while (1)
      yield(); // We stop here
  }
  Serial.println("[INFO]: SPIFFS initialised.");

  // Check for free space

  Serial.print("[INFO]: Free Space: ");
  Serial.println(SPIFFS.totalBytes() - SPIFFS.usedBytes());

  // ------------------- Splash screen ------------------

  // If we are woken up we do not need the splash screen
  if (wakeup_reason > 0)
  {
    // But we do draw something to indicate we are waking up
    tft.setTextFont(2);
    tft.println(" Waking up...");
  }
  else
  {

    // Draw a splash screen
    DrawSplash();
    tft.setCursor(1, 3);
    tft.setTextFont(2);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.printf("Loading version %s\n", versionnumber);
    Serial.printf("[INFO]: Loading version %s\n", versionnumber);
  }

  //------------------ Load Wifi Config ----------------------------------------------

  Serial.println("[INFO]: Loading Wifi Config");
  if (!loadMainConfig())
  {
    Serial.println("[WARNING]: Failed to load WiFi Credentials!");
  }
  else
  {
    Serial.println("[INFO]: WiFi Credentials Loaded");
  }

  // ----------------- Load webserver ---------------------

  handlerSetup();


  Serial.printf("[INFO]: Screen size is %dx%d\n", tft.width(), tft.height());

// Calibrate the touch screen and retrieve the scaling factors
#ifndef USECAPTOUCH
  touch_calibrate();
#endif

  // Let's first check if all the files we need exist
  if (!checkfile("/config/general.json"))
  {
    Serial.println("[ERROR]: /config/general.json not found!");
    while (1)
      yield(); // Stop!
  }

  // After checking the config files exist, actually load them
  if (!loadConfig())
  {
    Serial.println("[WARNING]: general.json seems to be corrupted!");
    Serial.println("[WARNING]: To reset to default type 'reset general'.");
    jsonfilefail = "general";
    pageNum = 10;
  }

  // Setup PWM channel for Piezo speaker

  ledcSetup(2, 500, 8);
  HandleAudio(Sounds::STARTUP);
  // Load menu definitions
  LoadAllMenus(tft);

  Serial.println("[INFO]: All configs loaded");
  // Now add the system config menu
  
  // Setup the Font used for plain text
  tft.setFreeFont(LABEL_FONT);

  //------------------BLE Initialization ------------------------------------------------------------------------

  Serial.println("[INFO]: Starting BLE");
  bleKeyboard.begin();

  // ---------------- Printing version numbers -----------------------------------------------
  Serial.print("[INFO]: BLE Keyboard version: ");
  Serial.println(BLE_KEYBOARD_VERSION);
  Serial.print("[INFO]: ArduinoJson version: ");
  Serial.println(ARDUINOJSON_VERSION);
  Serial.print("[INFO]: TFT_eSPI version: ");
  Serial.println(TFT_ESPI_VERSION);

  // ---------------- Start the first keypad -------------

  // Draw background
  tft.fillScreen(generalconfig.backgroundColour);
  SetActiveScreen("homescreen");

  if (generalconfig.sleepenable && touchInterruptPin >= 0)
  {
    // todo: implement sleep logic
    pinMode(touchInterruptPin, INPUT_PULLUP);
    Interval = generalconfig.sleeptimer * 60000;
    Serial.println("[INFO]: Sleep enabled.");
    Serial.print("[INFO]: Sleep timer = ");
    Serial.print(generalconfig.sleeptimer);
    Serial.println(" minutes");
    // islatched[28] = 1;
  }

  xTaskCreate(ScreenHandleTask, "Screen", 4096, NULL, tskIDLE_PRIORITY + 5, &xScreenTask);
  xTaskCreate(ActionTask, "Action", 4096, NULL, tskIDLE_PRIORITY + 5, &xActionTask);
}

void processSerial()
{
  // Check if there is data available on the serial input that needs to be handled.

  if (Serial.available())
  {

    String command = Serial.readStringUntil(' ');

    if (command == "cal")
    {
      FILESYSTEM.remove(CALIBRATION_FILE);
      ESP.restart();
    }
    else if (command == "setssid")
    {

      String value = Serial.readString();
      if (saveWifiSSID(value))
      {
        Serial.printf("[INFO]: Saved new SSID: %s\n", value.c_str());
        loadMainConfig();
        Serial.println("[INFO]: New configuration loaded");
      }
    }
    else if (command == "setpassword")
    {
      String value = Serial.readString();
      if (saveWifiPW(value))
      {
        Serial.printf("[INFO]: Saved new Password: %s\n", value.c_str());
        loadMainConfig();
        Serial.println("[INFO]: New configuration loaded");
      }
    }
    else if (command == "setwifimode")
    {
      String value = Serial.readString();
      if (saveWifiMode(value))
      {
        Serial.printf("[INFO]: Saved new WiFi Mode: %s\n", value.c_str());
        loadMainConfig();
        Serial.println("[INFO]: New configuration loaded");
      }
    }
    else if (command == "restart")
    {
      Serial.println("[WARNING]: Restarting");
      ESP.restart();
    }

    else if (command == "reset")
    {
      String file = Serial.readString();
      Serial.printf("[INFO]: Resetting %s.json now\n", file.c_str());
      resetconfig(file);
    }
  }
}
void AudioChipTune(Sounds sound)
{
  switch (sound)
  {
  case Sounds::GOING_TO_SLEEP:
    if (generalconfig.beep)
    {
      ledcAttachPin(speakerPin, 2);
      ledcWriteTone(2, 1200);
      delay(150);
      ledcDetachPin(speakerPin);
      ledcWrite(2, 0);

      ledcAttachPin(speakerPin, 2);
      ledcWriteTone(2, 800);
      delay(150);
      ledcDetachPin(speakerPin);
      ledcWrite(2, 0);

      ledcAttachPin(speakerPin, 2);
      ledcWriteTone(2, 600);
      delay(150);
      ledcDetachPin(speakerPin);
      ledcWrite(2, 0);
    }
    break;
  case Sounds::BEEP:
    if (generalconfig.beep)
    {
      ledcAttachPin(speakerPin, 2);
      ledcWriteTone(2, 600);
      delay(50);
      ledcDetachPin(speakerPin);
      ledcWrite(2, 0);
    }
    break;
  case Sounds::STARTUP:
    if (generalconfig.beep)
    {
      ledcAttachPin(speakerPin, 2);
      ledcWriteTone(2, 600);
      delay(150);
      ledcDetachPin(speakerPin);
      ledcWrite(2, 0);

      ledcAttachPin(speakerPin, 2);
      ledcWriteTone(2, 800);
      delay(150);
      ledcDetachPin(speakerPin);
      ledcWrite(2, 0);

      ledcAttachPin(speakerPin, 2);
      ledcWriteTone(2, 1200);
      delay(150);
      ledcDetachPin(speakerPin);
      ledcWrite(2, 0);
    }
    break;
  default:
    break;
  }
}
void HandleAudio(Sounds sound)
{
  if (speakerPin >= 0)
  {
    AudioChipTune(sound);
  }
  // todo:  add support for i2s audio
}

void ChangeBrightness(Direction direction)
{
  if (direction == Direction::UP)
  {
    ledBrightness = min(ledBrightness + LED_BRIGHTNESS_INCREMENT, 255);
  }
  else
  {
    ledBrightness = max(ledBrightness - LED_BRIGHTNESS_INCREMENT, 0);
  }
  ledcWrite(0, ledBrightness);
}

void ResetSleep()
{
  previousMillis = millis();
}
//--------------------- LOOP ---------------------------------------------------------------------
#ifdef USECAPTOUCH
bool getTouch(uint16_t *t_x, uint16_t *t_y)
{
  if (ts.touched())
  {
    // Retrieve a point
    TS_Point p = ts.getPoint();
#ifndef ARDUINO_TWATCH_BASE
    //Flip things around so it matches our screen rotation
    p.x = map(p.x, 0, 320, 320, 0);
    *t_y = p.x;
    *t_x = p.y;
#else
    *t_y = p.y;
    *t_x = p.x;
#endif
    ResetSleep();
    return true;
  }
  return false;
}
#else
bool getTouch(uint16_t *t_x, uint16_t *t_y)
{
  if (tft.getTouch(t_x, t_y))
  {
    ResetSleep();
    return true;
  }
  return false;
}
#endif

void handleDisplay(bool pressed, uint16_t t_x, uint16_t t_y)
{
  auto Active = GetActiveScreen();
  if (Active)
  {
    if (pressed)
    {
      Active->Touch(t_x, t_y);
    }
    else
    {
      Active->ReleaseAll();
    }
    Active->Draw();
  }
  else
  {
    Serial.println("No active display!");
  }
}

void loop(void)
{

  processSerial();

  if (generalconfig.sleepenable && touchInterruptPin >= 0)
  {
    if (millis() > previousMillis + Interval)
    {

      // The timer has ended and we are going to sleep  .
      tft.fillScreen(TFT_BLACK);
      Serial.println("[INFO]: Going to sleep.");
      HandleAudio(Sounds::GOING_TO_SLEEP);
      //todo better power management for TWATCH
      //       power->setPowerOutPut(AXP202_LDO2, AXP202_OFF);
      esp_sleep_enable_ext0_wakeup(touchInterruptPin, 0);
      esp_deep_sleep_start();
    }
  }

  delay(10);
}

void ScreenHandleTask(void *pvParameters)
{
  uint16_t t_x = 0;
  uint16_t t_y = 0;
  for (;;)
  {
    bool pressed = getTouch(&t_x, &t_y);
    handleDisplay(pressed, t_x, t_y);
    delay(10);
  }
}

FreeTouchDeck::FTAction *PopQueue()
{
  FreeTouchDeck::FTAction *Action = NULL;

  if (QueueLock(portMAX_DELAY / portTICK_PERIOD_MS))
  {
    if (!Queue.empty())
    {
      Action = Queue.front();
      Queue.pop();
    }
    QueueUnlock();
  }
  else
  {
    Serial.println("[ERROR]: Unable to lock Action queue");
  }
  return Action;
}
void ActionTask(void *pvParameters)
{
  for (;;)
  {
    auto Action = PopQueue();
    if (Action)
    {
      ResetSleep();
      Action->Execute();
    }
    else
    {
      delay(50);
    }
  }
}