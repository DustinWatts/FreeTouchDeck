
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
   
using namespace FreeTouchDeck;
#include <pgmspace.h> // PROGMEM support header
#include <FS.h>       // Filesystem support header
#include <SPIFFS.h>   // Filesystem support header

#include <TFT_eSPI.h> // The TFT_eSPI library

#include <BleKeyboard.h>       // BleKeyboard is used to communicate over BLE
#include "BLEDevice.h"         // Additional BLE functionaity
#include "BLEUtils.h"          // Additional BLE functionaity
#include "BLEBeacon.h"         // Additional BLE functionaity
#include "esp_sleep.h"         // Additional BLE functionaity
#include "esp_bt_main.h"       // Additional BLE functionaity
#include "esp_bt_device.h"     // Additional BLE functionaity
#include <ArduinoJson.h>       // Using ArduinoJson to read and write config files
#include <WiFi.h>              // Wifi support
#include <AsyncTCP.h>          //Async Webserver support header
#include <ESPAsyncWebServer.h> //Async Webserver support header
#include "globals.hpp"
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
#include "cJSON.h"
#include "ImageWrapper.h"

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

// Initial LED brightness

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

RTC_NOINIT_ATTR SystemMode restartReason = SystemMode::STANDARD;
SystemMode RunMode = SystemMode::STANDARD;

void ChangeMode(SystemMode newMode)
{
  restartReason = newMode;
  ESP.restart();
}

volatile unsigned long previousMillis = 0;
unsigned long Interval = 0;
#ifdef ACTIONS_IN_TASKS
TaskHandle_t xScreenTask = NULL;
void ScreenHandleTask(void *pvParameters);
TaskHandle_t xActionTask = NULL;
void ActionTask(void *pvParameters);
#endif
IRAM_ATTR char *ps_strdup(const char *fmt)
{
  // Duplicate string values, calling our own
  // memory allocation so we can decide to use
  // PSRAM or not
  const char *s = NULL;
  if (fmt && strlen(fmt) > 0)
  {
    s = fmt;
  }
  else
  {
    s = "";
  }
  char *o = (char *)malloc_fn(strlen(fmt) + 1);
  if (o)
  {
    memcpy(o, fmt, strlen(fmt));
  }
  return o;
}
IRAM_ATTR void *malloc_fn(size_t sz)
{
  void *ptr = NULL;
#if defined(ESP32) && defined(CONFIG_SPIRAM_SUPPORT)
#if defined(ESP32)
  ptr = heap_caps_malloc(sz, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
#else
  ptr = malloc(sz)
#endif
#else
#endif
  //     bool psramSupported=psramInit()&&psramFound();
  // #else
  //   bool psramSupported=false;
  // #endif

  //   if (psramSupported)
  //   {
  //     ptr = ps_malloc(sz);
  //     if(!ptr)
  //     {
  //       LOC_LOGE("FreeTouchDeck","malloc_fn: PSRAM alloc failed. Allocating RAM instead");
  //       ptr=malloc(sz);
  //     }

  //   }
  //   else
  //   {
  //     ptr=malloc(sz);
  //   }

  if (!ptr)
  {
    Serial.print("free_iram: ");
    Serial.print(heap_caps_get_free_size(MALLOC_CAP_INTERNAL));
    drawErrorMessage(true, module, "Memory allocation failed");
  }
  else
  {
    memset(ptr, 0x00, sz);
  }

  return ptr;
}

void init_cJSON()
{
  static cJSON_Hooks hooks;
  hooks.malloc_fn = &malloc_fn;
  cJSON_InitHooks(&hooks);
}

// Checking for BLE Keyboard version
#ifndef BLE_KEYBOARD_VERSION
#warning Old BLE Keyboard version detected. Please update.
#define BLE_KEYBOARD_VERSION "Outdated"
#endif

//--------- Internal references ------------
// (this needs to be below all structs etc..)
#include "ScreenHelper.h"

//#define DEFAULT_LOG_LEVEL LogLevels::DEBUG

#include "ConfigLoad.h"
#include "DrawHelper.h"
#include "ConfigHelper.h"
#include "MenuNavigation.h"
#ifndef USECAPTOUCH
#include "Touch.h"
#endif

void DrawSplash()
{
  LOC_LOGD(module, "Loading splash screen bitmap.");
  FreeTouchDeck::ImageWrapper *splash = FreeTouchDeck::GetImage("freetouchdeck_logo.bmp");
  if (splash)
  {
    LOC_LOGD(module, "splash screen bitmap loaded. Drawing");
    splash->Draw(0, 0, false);
  }
  else
  {
    LOC_LOGW(module, "Unable to draw the splash screen.");
  }
}
void CacheBitmaps()
{
  // Preload all bitmaps
  File root = SPIFFS.open("/");
  File file = root.openNextFile();
  PrintMemInfo();
  while (file)
  {
    String FileName = file.name();
    if (FileName.endsWith(".bmp"))
    {
      int start = FileName.lastIndexOf("/") + 1;
      FileName = FileName.substring(start);
      LOC_LOGV(module, "Caching bitmap from file %s, with name %s", file.name(), FileName.c_str());
      FreeTouchDeck::ImageWrapper *image = FreeTouchDeck::GetImage(FileName.c_str());
      LOC_LOGV(module, "Adding menu completed. Getting next file");
    }
    file = root.openNextFile();
  }
}
void PrintMemInfo()
{
  static size_t prev_free = 0;
  static size_t prev_min_free = 0;
  if (generalconfig.LogLevel < LogLevels::VERBOSE)
    return;
  LOC_LOGV(module, "free_iram: %d, delta: %d", heap_caps_get_free_size(MALLOC_CAP_INTERNAL), prev_free > 0 ? prev_free - heap_caps_get_free_size(MALLOC_CAP_INTERNAL) : 0);
  LOC_LOGV(module, "min_free_iram: %d, delta: %d", heap_caps_get_minimum_free_size(MALLOC_CAP_INTERNAL), prev_free > 0 ? prev_min_free - heap_caps_get_free_size(MALLOC_CAP_INTERNAL) : 0);
  prev_free = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
  prev_min_free = heap_caps_get_minimum_free_size(MALLOC_CAP_INTERNAL);
}

const char *enum_to_string(SystemMode mode)
{
  switch (mode)
  {

    ENUM_TO_STRING_HELPER(SystemMode, STANDARD);
    ENUM_TO_STRING_HELPER(SystemMode, CONSOLE);
    ENUM_TO_STRING_HELPER(SystemMode, CONFIG);
  default:
    return "unknown";
  }
}
void powerInit()
{
#ifdef ARDUINO_TWATCH_BASE
  LOC_LOGI(module, "Enabling AXP power management chip.");
  Wire1.begin(21, 22);
  int ret = power->begin(Wire1, AXP202_SLAVE_ADDRESS, false);
  if (ret == AXP_FAIL)
  {
    LOC_LOGE(module, "AXP Power begin failed");
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
  LOC_LOGI(module, "Setting up Display Back light");
#endif

  // Setup PWM channel and attach pin 32
  ledcSetup(0, 5000, 8);
#ifdef TFT_BL
  ledcAttachPin(TFT_BL, 0);
#else
  ledcAttachPin(32, 0);
#endif
  ledcWrite(0, generalconfig.ledBrightness); // Start @ initial Brightness
}

void HandleSleepConfig()
{

  if (generalconfig.sleepenable && touchInterruptPin >= 0)
  {
    // todo: implement sleep logic
    pinMode(touchInterruptPin, INPUT_PULLUP);
    Interval = generalconfig.sleeptimer * 60000;
    QueueAction(FreeTouchDeck::sleepSetLatchAction);
    LOC_LOGI(module, "Sleep enabled. Timer = %d minutes", generalconfig.sleeptimer);
  }
  else 
  {
      QueueAction(FreeTouchDeck::sleepClearLatchAction);
  }
}
void HandleBeepConfig()
{
  if (generalconfig.beep)
  {
    QueueAction(FreeTouchDeck::beepSetLatchAction);
  }
  else
  {
    QueueAction(FreeTouchDeck::beepClearLatchAction);
  }
}
void touchInit()
{

#ifdef USECAPTOUCH
#ifdef CUSTOM_TOUCH_SDA
  if (!ts.begin(40, CUSTOM_TOUCH_SDA, CUSTOM_TOUCH_SCL))
#else
  if (!ts.begin(40))
#endif
  {
    LOC_LOGE(module, "Unable to start the capacitive touchscreen.");
  }
  else
  {
    LOC_LOGI(module, "Capacitive touch started");
  }
#endif
}

void displayInit()
{
  static bool initialized = false;
  if (initialized)
    return;
  initialized = true;
  // --------------- Init Display -------------------------
  LOC_LOGI(module, "Initializing display");
  // Initialise the TFT screen
  tft.init();
  // Set the rotation
  tft.setRotation(generalconfig.screenrotation);
  // Clear the screen
  tft.fillScreen(TFT_BLACK);
  // Setup the Font used for plain text
  InitFontsTable();
  SetDefaultFont();
  tft.setCursor(0, tft.fontHeight()+1);
  LOC_LOGI(module, "Screen size is %dx%d", tft.width(), tft.height());
}

void LoadSystemConfig()
{
  // -------------- Start filesystem ----------------------
  if (!FILESYSTEM.begin())
  {
    LOC_LOGE(module, "SPIFFS initialisation failed!");
    drawErrorMessage(true, module, "Failed to init SPIFFS! Did you upload the data folder?");
  }
  LOC_LOGI(module, "SPIFFS initialised. Free Space: %d", SPIFFS.totalBytes() - SPIFFS.usedBytes());

  // if(psramFound())
  // LOC_LOGI(module,"Available PSRAM: %d",heap_caps_get_free_size(MALLOC_CAP_SPIRAM));

  //------------------ Load Wifi Config ----------------------------------------------
  LOC_LOGI(module, "Loading Wifi Config");
  if (!loadWifiConfig())
  {
    LOC_LOGW(module, "Failed to load WiFi Credentials!");
  }
  else
  {
    LOC_LOGI(module, "WiFi Credentials Loaded");
  }
  // Let's first check if all the files we need exist
  if (!checkfile("/config/general.json"))
  {
    // draw an error message and stop
    drawErrorMessage(true, module, "/config/general.json not found. Make sure the data partition was uploaded");
  }

  // After checking the config files exist, actually load them
  if (!loadGeneralConfig())
  {
    drawErrorMessage(false, module, "general.json seems to be corrupted. To reset to default type 'reset general'.");
  }
}
void ShowDir()
{

  File root = SPIFFS.open("/");
  File file = root.openNextFile();
  LOC_LOGI(module, "Name\tSize");
  while (file)
  {
    LOC_LOGI(module, "%s\t%d", file.name(), file.size());
    file = root.openNextFile();
  }
  root.close();
}
void PrintScreenMessage(const char *message)
{
  SetSmallestFont(1);
  tft.setTextSize(1);
  tft.setCursor(0, tft.fontHeight()+1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.println(message);
}

void DumpCJson(cJSON *doc)
{
  if (generalconfig.LogLevel < FreeTouchDeck::LogLevels::VERBOSE)
    return;
  char *d = cJSON_Print(doc);
  if (d)
  {
    LOC_LOGD(module, "%s", d);
  }
  FREE_AND_NULL(d);
}

//-------------------------------- SETUP --------------------------------------------------------------

void setup()
{

  RESET_REASON resetReason = rtc_get_reset_reason(0);
  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
  SetGeneralConfigDefaults();
  // Use serial port
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  LOC_LOGI(module, "Starting system.");
  init_cJSON();
  // We cannot rely on the c++ compiler to initialize our
  // contants, for example the buttons list which is required by
  // other constants.  Initializing them here ensure that
  // primitive maps will exist before we try to access them
  FTAction::InitConstants();
  FTButton::InitConstants();

  LoadSystemConfig();
  displayInit();
  powerInit();
  touchInit();

  if (restartReason != SystemMode::STANDARD && restartReason != SystemMode::CONFIG && restartReason != SystemMode::CONSOLE)
  {
    restartReason = SystemMode::STANDARD;
  }

  // ------------------- Startup actions ------------------
  LOC_LOGI(module, "Found Reset reason: %d", resetReason);
  LOC_LOGI(module, "System mode: %s", enum_to_string(restartReason));

  if ((resetReason == SW_RESET || resetReason == SW_CPU_RESET))
  {
    //reset restartreason for next reboot

    if (restartReason == SystemMode::CONFIG)
    {
      generalconfig.sleepenable = false;
      if (ConfigMode())
      {
        RunMode = SystemMode::CONFIG;
        restartReason = SystemMode::STANDARD;
        LoadAllMenus();
        return;
      }
      else
      {
        drawErrorMessage(true, module, "Unable to start config mode. Please reset device.");
      }
    }
    else if (restartReason == SystemMode::CONSOLE)
    {
      RunMode = SystemMode::CONSOLE;
      restartReason = SystemMode::STANDARD;
      PrintScreenMessage("Console mode active. Press screen to exit");
      LoadAllMenus();
      return;
    }
  }

  if (wakeup_reason > ESP_SLEEP_WAKEUP_UNDEFINED)
  {
    // If we are woken up we do not need the splash screen
    // But we do draw something to indicate we are waking up
    SetSmallestFont(1);
    tft.println(" Waking up...");
  }
  else
  {
    // Draw a splash screen
    DrawSplash();
    LOC_LOGD(module, "Displaying version details");
    SetSmallestFont(1);
    tft.setTextSize(1);
    tft.setCursor(1, tft.fontHeight()+1);
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.printf("Loading version %s\n", versionnumber);
    LOC_LOGI(module, "Loading version %s", versionnumber);
  }

  HandleAudio(Sounds::STARTUP);
// Calibrate the touch screen and retrieve the scaling factors
#ifndef USECAPTOUCH
  touch_calibrate();
#endif

  //CacheBitmaps();
  LoadAllMenus();


  LOC_LOGI(module, "All config files loaded");

  //------------------BLE Initialization ------------------------------------------------------------------------
  LOC_LOGI(module, "Starting BLE Keyboard");
  bleKeyboard.deviceName = generalconfig.deviceName;
  bleKeyboard.deviceManufacturer = generalconfig.manufacturer;
  bleKeyboard.begin();

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
  PrintMemInfo();
  HandleSleepConfig();
  HandleBeepConfig();
#ifdef ACTIONS_IN_TASKS
  xTaskCreate(ScreenHandleTask, "Screen", 1024 * 3, NULL, tskIDLE_PRIORITY + 8, &xScreenTask);
  PrintMemInfo();
  LOC_LOGD(module, "Screen task created");
  xTaskCreate(ActionTask, "Action", 1024 * 4, NULL, tskIDLE_PRIORITY + 5, &xActionTask);
  PrintMemInfo();
  LOC_LOGD(module, "Action task created");
#endif
}
String readLine()
{
    String ret;
    int c = Serial.read();
    while(c >= 0 && c != '\r' && c != '\n') {
        Serial.print((char)c);
        ret += (char) c;
        c = Serial.read();
    }
    while (c >= 0 && c == '\r' && c == '\n') {
      Serial.print("Skipping char\n");
      Serial.read();
    }
    return ret;
}
void processSerial()
{
  // Check if there is data available on the serial input that needs to be handled.

  if (Serial.available())
  {

    String command = readLine();

    if (command == "cal")
    {
      FILESYSTEM.remove(CALIBRATION_FILE);
      ESP.restart();
    }
    else if (command.startsWith("loglevel"))
    {
      String level = command.substring(command.lastIndexOf(" "));
      LOC_LOGI(module, "Attempting to set log level to %s",level.c_str());
      LogLevels lev = static_cast<LogLevels>(level.toInt());
      if (lev >= LogLevels::NONE && lev <= LogLevels::VERBOSE)
      {
        generalconfig.LogLevel = lev;
        LOC_LOGI(module, "Log level changed to %d", generalconfig.LogLevel);
        saveConfig(false);
      }
      else
      {
        LOC_LOGE(module, "Invalid log level %s", level.c_str());
      }
    }
    else if (command == "console")
    {
      LOC_LOGW(module, "Changing mode to console");
      ChangeMode(SystemMode::CONSOLE);
    }
    else if (command == "configmode")
    {
      LOC_LOGW(module, "Changing mode to configuration");
      ChangeMode(SystemMode::CONFIG);
    }
    else if (command == "dir")
    {
      ShowDir();
    }

    else if (command == "menus")
    {
      LOC_LOGI(module, "Generating menus structure");
      char *json = FreeTouchDeck::MenusToJson(true);
      if (json)
      {
        LOC_LOGI(module, "Menu structure: \n%s", json);
        FREE_AND_NULL(json);
      }
      else
      {
        LOC_LOGE(module, "Unable to print menu structure");
      }
    }

    else if (command.startsWith("activate"))
{
      String value = command.substring(command.lastIndexOf(" "));
      value.trim();
      LOC_LOGD(module,"Activating screen %s",value.c_str());
      if(!SetActiveScreen(value.c_str()))
      {
        LOC_LOGE(module,"unable to activate screen %s",value.c_str());
      }
}
    else if (command.startsWith("rot"))
    {
      String value = command.substring(command.lastIndexOf(" "));
      value.trim();
      uint8_t rot = value.toInt();
      if (rot <= 3 && rot >= 0)
      {
        generalconfig.screenrotation = rot;
        saveConfig(false);
        LOC_LOGI(module, "Screen rotation was updated to %d", generalconfig.screenrotation);
      }
    }
    else if (command == "revx")
    {
      generalconfig.reverse_x_touch = !generalconfig.reverse_x_touch;
      LOC_LOGI(module, "X axis touch reverse set to %s", generalconfig.reverse_x_touch ? "YES" : "NO");
      saveConfig(false);
    }
    else if (command.startsWith("rows"))
    {
      String value = command.substring(command.lastIndexOf(" "));
      value.trim();
      generalconfig.rowscount = value.toInt();
      LOC_LOGI(module, "Rows count set to %d", generalconfig.rowscount);
      saveConfig(false);
    }
    else if (command.startsWith("cols"))
    {
      String value = command.substring(command.lastIndexOf(" "));
      value.trim();
      generalconfig.colscount = value.toInt();
      LOC_LOGI(module, "Rows count set to %d", generalconfig.colscount);
      saveConfig(false);
    }
    else if (command == "revy")
    {
      generalconfig.reverse_y_touch = !generalconfig.reverse_y_touch;
      LOC_LOGI(module, "Y axis touch reverse set to %s", generalconfig.reverse_y_touch ? "YES" : "NO");
      saveConfig(false);
    }
    else if (command == "invaxis")
    {
      generalconfig.flip_touch_axis = !generalconfig.flip_touch_axis;
      LOC_LOGI(module, "Touch axis flip set to %s", generalconfig.flip_touch_axis ? "YES" : "NO");
      saveConfig(false);
    }
    else if (command.startsWith( "setssid"))
    {
      String value = command.substring(command.indexOf(" "));
      value.trim();
      if (saveWifiSSID(value))
      {
        LOC_LOGI(module, "Saved new SSID: %s\n", value.c_str());
        loadWifiConfig();
        LOC_LOGI(module, "New configuration loaded");
      }
    }
    else if (command.startsWith("setpassword"))
    {
      String value = command.substring(command.indexOf(" "));
      value.trim();
      if (saveWifiPW(value))
      {
        LOC_LOGI(module, "Saved new Password: %s\n", value.c_str());
        loadWifiConfig();
        LOC_LOGI(module, "New configuration loaded");
      }
    }
    else if (command.startsWith("setwifimode"))
    {
      String value = command.substring(command.indexOf(" "));
      value.trim();
      if (saveWifiMode(value))
      {
        LOC_LOGI(module, "Saved new WiFi Mode: %s\n", value.c_str());
        loadWifiConfig();
        LOC_LOGI(module, "New configuration loaded");
      }
    }
    else if (command == "restart")
    {
      LOC_LOGD(module, "Restarting");
      ESP.restart();
    }
    else if (command == "convertmenus")
    {
      LOC_LOGI(module, "Converting menu structure from old to new format");
      SaveFullFormat();
    }
    else if (command == "reset")
    {
      String file = Serial.readString();
      file.trim();
      ESP_LOGI(module, "Resetting %s.json now\n", file.c_str());
      resetconfig(file);
    }
    else if (command == "setmenus")
    {
      LOC_LOGI(module,"Paste menu(s) here. ~~~ to terminate");
      const char *tempName = "/config/~tempmenu.";
      fs::File tempfile = SPIFFS.open(tempName, FILE_WRITE);
      if (tempfile)
      {
        int v;
        bool end = false;
        bool cancel = false;
        uint8_t endCount = 0;
        while (!end && !cancel)
        {
          if (Serial.available() > 0)
          {
            v = Serial.read();
            if ((char)v == '~')
            {
              endCount++;
            }
            else if ((char)v == '\032')
            {
              LOC_LOGW(module, "Cancelling");
              cancel = true;
            }
            else
            {
              while (endCount > 0)
              {
                endCount--;
                tempfile.write((uint8_t)'~');
              }
              tempfile.write((uint8_t)v);
            }
            if (endCount >= 3)
            {
              end = true;
            }
          }
        }
        tempfile.close();
        if (!cancel)
        {
          if(!LoadFullFormat(tempName))
          {
            LOC_LOGE(module,"Error loading file. ");
          }
          else 
          {
            SaveFullFormat();
          }
        }
      }
    }
    else if (command == "setconfig")
    {
      LOC_LOGI(module,"Paste configuration here. ~~~ to terminate");
      const char *tempName = "/config/~temp.";
      fs::File tempfile = SPIFFS.open(tempName, FILE_WRITE);
      if (tempfile)
      {
        int v;
        bool end = false;
        bool cancel = false;
        uint8_t endCount = 0;
        while (!end && !cancel)
        {
          if (Serial.available() > 0)
          {
            v = Serial.read();
            if ((char)v == '~')
            {
              endCount++;
            }
            else if ((char)v == '\032')
            {
              LOC_LOGW(module, "Cancelling");
              cancel = true;
            }
            else
            {
              while (endCount > 0)
              {
                endCount--;
                tempfile.write((uint8_t)'~');
              }
              tempfile.write((uint8_t)v);
            }
            if (endCount >= 3)
            {
              end = true;
            }
          }
        }
        tempfile.close();
        if (!cancel)
        {
          if(loadConfig(tempName))
          {
            saveConfig(false);
          }
        }
      }
    }
    else if (command == "showconfig")
    {
      saveConfig(true);
    }
    else if (command == "help")
    {
      LOC_LOGI(module,
               R"(
====================
Help 
====================
cal : Restarts in screen calibration mode (resistive screens only)
revx : reverse X touch axis
revy : reverse Y touch axis
rot (0-3) : sets the rotation of the screen
invaxis: Flip the X and Y axis
setssid YOURSSID: Sets the WiFi SSID access point to connect to
setpassword YOURPASWORD: Sets the WiFi password 
setwifimode (WIFI_STA|WIFI_AP)
convertmenus : converts from older menu formats to new format
restart : Restarts the system
reset : reset configuration and reboot 
menus : dump the menu structure
setmenus : load menu structure from console.  End with ~~~
showconfig  : dump current configuration
setconfig (config json text) : Upload the configuration file - terminate with ~~~
console : change the system mode to console
configmode : change the system mode to configuration
loglevel (0-5) : increase log details for some activities - warning: more logs will slow down the system
dir : show the content of the file system
)");
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
    static bool isInit = false;

    if (!isInit)
    {
      // Setup PWM channel for Piezo speaker
      ledcSetup(2, 500, 8);
      isInit = true;
    }
    AudioChipTune(sound);
  }
  // todo:  add support for i2s audio
}

void ResetSleep()
{
  previousMillis = millis();
}
//--------------------- LOOP ---------------------------------------------------------------------
#ifdef USECAPTOUCH
bool getTouch(uint16_t *t_x, uint16_t *t_y)
{
  static bool prev = false;
  if (ts.touched())
  {
    // Retrieve a point
    TS_Point p = ts.getPoint();
    if (generalconfig.flip_touch_axis)
    {
      //Flip things around so it matches our screen rotation
      *t_y = p.x;
      *t_x = p.y;
    }
    else
    {
      *t_y = p.y;
      *t_x = p.x;
    }

    if (generalconfig.reverse_x_touch)
    {
      *t_x = (uint16_t)map((long)*t_x, (long)0, (long)tft.width(), (long)tft.width(), (long)0);
    }
    if (generalconfig.reverse_y_touch)
    {
      *t_y = (uint16_t)map((long)*t_y, (long)0, (long)tft.height(), (long)tft.height(), (long)0);
    }

    if (!prev)
    {
      LOC_LOGD(module, "Input touch (%dx%d)=>(%dx%d) - Screen is %dx%d", p.x, p.y, *t_x, *t_y, tft.width(), tft.height());
      prev = true;
    }

    ResetSleep();
    return true;
  }
  prev = false;
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
void processSleep()
{
  if (generalconfig.sleepenable && touchInterruptPin >= 0)
  {
    if (millis() > previousMillis + Interval)
    {
      // The timer has ended and we are going to sleep  .
      EnterSleep();
    }
  }
}
void handleDisplay(bool pressed, uint16_t t_x, uint16_t t_y)
{
  static unsigned nextlog = 0;
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
    if (nextlog <= millis())
    {
      LOC_LOGD(module, "No active display");
      // to prevent flooding of the serial log, reduce the rate of this message
      nextlog = millis() + 1000;
    }
  }
}
void HandleActions()
{
  LOC_LOGV(module, "Checking for regular actions");
  FTAction *Action = NULL;
  do
  {
    Action = PopQueue();
    if (Action)
    {
      ResetSleep();
      Action->Execute();
    }
  } while (Action);
}
void HandleScreen()
{
  uint16_t t_x = 0;
  uint16_t t_y = 0;
  bool pressed = getTouch(&t_x, &t_y);
  if (RunMode == SystemMode::CONFIG || RunMode == SystemMode::CONSOLE)
  {
    delay(100);
    if (pressed)
    {
      ESP.restart();
    }
    return;
  }

  handleDisplay(pressed, t_x, t_y);
  LOC_LOGV(module, "Checking for screen actions");
  FTAction *Action = PopScreenQueue();
  if (Action)
  {
    ResetSleep();
    Action->Execute();
  }
}
void loop(void)
{

  processSerial();
  if (RunMode != SystemMode::CONSOLE && RunMode != SystemMode::CONFIG)
  {
    processSleep();
  }

#ifndef ACTIONS_IN_TASKS
  HandleActions();
  HandleScreen();
#endif
  delay(10);
}
#ifdef ACTIONS_IN_TASKS
void ScreenHandleTask(void *pvParameters)
{

  // Load menu definitions

  for (;;)
  {
    HandleScreen();
    delay(50);
  }
}

void ActionTask(void *pvParameters)
{
  for (;;)
  {
    HandleActions();
    delay(50);
  }
}
#endif