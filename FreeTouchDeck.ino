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

// ------- Uncomment the define below if you want to use SLEEP and wake up on touch -------
// The pin where the IRQ from the touch screen is connected uses ESP-style GPIO_NUM_* instead of just pinnumber
#define touchInterruptPin GPIO_NUM_27

// ------- Uncomment the define below if you want to use a piezo buzzer and specify the pin where the speaker is connected -------
//#define speakerPin 26

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

#include <vector>

#ifdef USECAPTOUCH
#include <Wire.h>
#include <FT6236.h>
FT6236 ts = FT6236();
#endif

BleKeyboard bleKeyboard("FreeTouchDeck", "Made by me");

AsyncWebServer webserver(80);

TFT_eSPI tft = TFT_eSPI();

// Define the storage to be used. For now just SPIFFS.
#define FILESYSTEM SPIFFS

// This is the file name used to store the calibration data
// You can change this to create new calibration files.
// The FILESYSTEM file name must start with "/".
#define CALIBRATION_FILE "/TouchCalData"

// Set REPEAT_CAL to true instead of false to run calibration
// again, otherwise it will only be done once.
// Repeat calibration if you change the screen rotation.
#define REPEAT_CAL false

// Set the width and height of your screen here:
#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 320

// Arbitrary upper bound on buttons per pages
#define MAX_BUTTONS 256

// Upper limit of size for config files we will read.
#define MAX_CONFIG_SIZE 32768

// Font size multiplier
#define KEY_TEXTSIZE 1

// Text Button Label Font
#define LABEL_FONT &FreeSansBold12pt7b

enum bleAction {
  NoAction = 0,
  DelayAction = 1,
  TabArrowAction = 2,
  MediaAction = 3,
  CharAction = 4,
  OptionAction = 5,
  FnAction = 6,
  NumberAction = 7,
  SpecialCharAction = 8,
  ComboAction = 9,
  HelperAction = 10,
  SpecialFnAction = 11,
};

enum specialPages {
  HomePage = 0,
  SettingsPage = MAX_BUTTONS, 
  EmptyPage = -2,
  InfoPage = -3,
  WifiErrorPage = -4,
  ConfigErrorPage = -5
};

// placeholder for the pagenumber we are on (0 indicates home)
int pageNum = 0;

// Initial LED brightness
int ledBrightness = 255;

//path to the directory the logo are in ! including leading AND trailing / !
const String logopath = "/logos/";
const String configpath = "/config/";

// Struct Action: 3 actions and 3 values per button
struct Actions
{
  // TODO use vector to reduce memory usage allow customizable list of actions?
  uint8_t action0;
  uint8_t value0;
  char symbol0[64];
  uint8_t action1;
  uint8_t value1;
  char symbol1[64];
  uint8_t action2;
  uint8_t value2;
  char symbol2[64];
};

// Each button has an action struct in it
struct Button
{
  struct Actions actions;
  bool latch;
  bool isLatched;
  String latchlogo;
};

// Each screen has logos and maybe buttons
struct Screen
{
  std::vector<String> logo;
  // Note button will be empty for homescreen
  std::vector<Button*> button;
  int cols;
  int rows;
  Screen() : cols(3), rows(2) {}
};

// Struct to hold the general logos.
struct Generallogos
{
  String homebutton;
  String configurator;
};

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

struct Wificonfig
{
  char ssid[64];
  char password[64];
  char wifimode[9];
  char hostname[64];
  uint8_t attempts;
  uint16_t attemptdelay;
};

// Create instances of the structs
Wificonfig wificonfig;

Config generalconfig;

Generallogos generallogo;

std::vector<Screen*> screen;

/**
* @brief get the Screen for the current pageNum
* 
* @return Screen* screen for current page
*/
Screen* currentScreen() {
  if (pageNum < HomePage) {
    return NULL;
  }
  if (pageNum == SettingsPage) {
    return screen.back();
  }
  return screen[pageNum];
}

/**
* @brief center of first button for current screen
*
* @return int X coordinate
*/
int keyX() {
  Screen* s = currentScreen();
  return SCREEN_WIDTH / (s->cols * 2);
}

/**
* @brief center of first button for current screen
*
* @return int Y coordinate
*/
int keyY() {
  Screen* s = currentScreen();
  return SCREEN_HEIGHT / (s->rows * 2);
}

/**
* @brief horizontal gap between buttons for current screen
*
* @return int gap in pixels
*/
int keySpacingX() {
  Screen* s = currentScreen();
  return SCREEN_WIDTH / (s->cols * 8);
}

/**
* @brief vertical gap between buttons for current screen
*
* @return int gap in pixels
*/
int keySpacingY() {
  Screen* s = currentScreen();
  return SCREEN_HEIGHT / (s->rows * 8);
}

/**
* @brief Width of a button for the current screen
*
* @return int width
*/
int keyW() {
  Screen* s = currentScreen();
  return (SCREEN_WIDTH / s->cols) - keySpacingX();
}

/**
* @brief Height of a button for current screen
*
* @return int height
*/
int keyH() {
  Screen* s = currentScreen();  
  return (SCREEN_HEIGHT / s->rows) - keySpacingY();
}

unsigned long previousMillis = 0;
unsigned long Interval = 0;
bool displayinginfo;
const char* jsonfilefail = "";

// vector to hold all the buttons we might use.
std::vector<TFT_eSPI_Button*> key;

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
#include "Action.h"
#include "Webserver.h"
#include "Touch.h"

//-------------------------------- SETUP --------------------------------------------------------------

void setup()
{

  // Use serial port
  Serial.begin(9600);
  Serial.setDebugOutput(true);
  Serial.println("");

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
  tft.setRotation(1);

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
    drawBmp("/logos/freetouchdeck_logo.bmp", SCREEN_WIDTH/2, SCREEN_HEIGHT/2);
    tft.setCursor(1, 3);
    tft.setTextFont(2);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.printf("Loading version %s\n", versionnumber);
    Serial.printf("[INFO]: Loading version %s\n", versionnumber);
  }

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

  if (!checkfile("/config/homescreen.json"))
  {
    Serial.println("[ERROR]: /config/homescreen.json not found!");
    while (1)
      yield(); // Stop!
  }

  // After checking the config files exist, actually load them
  if(!loadConfig("general")){
    Serial.println("[WARNING]: general.json seems to be corrupted!");
    Serial.println("[WARNING]: To reset to default type 'reset general'.");
    jsonfilefail = "general";
    setPage(ConfigErrorPage);
  }

    // Setup PWM channel for Piezo speaker

#ifdef speakerPin
  ledcSetup(2, 500, 8);

if(generalconfig.beep){
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

#endif

  if(!loadConfig("homescreen")){
    Serial.println("[WARNING]: homescreen.json seems to be corrupted!");
    Serial.println("[WARNING]: To reset to default type 'reset homescreen'.");
    jsonfilefail = "homescreen";
    setPage(ConfigErrorPage);
  }

  for( size_t i = 1; i < screen.size()-1; i++ ) {
    String menu = String("menu") + i;
    String config = configpath + menu + ".json";
    if (!checkfile(config.c_str())) {
      Serial.printf("[WARNING]: %s not found!\n", config.c_str());
      continue;
    } 
    if(!loadConfig(menu)){
      Serial.printf("[WARNING]: %s.json seems to be corrupted!\n", menu.c_str());
      Serial.printf("[WARNING]: To reset to default type 'reset %s'.\n", menu.c_str());
      jsonfilefail = menu.c_str();
      setPage(ConfigErrorPage);
    }
  }

  // Last page is Settings, add 5 buttons (6th is Home)
  for(int i=0; i<5; i++) {
      screen.back()->button.push_back(new Button());
      screen.back()->logo.push_back("");
  }

  Serial.println("[INFO]: All configs loaded");

  
  
  generallogo.homebutton = "/logos/home.bmp";
  generallogo.configurator =  "/logos/wifi.bmp";
  Serial.println("[INFO]: General logos loaded.");

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

  // Draw keypad
  Serial.println("[INFO]: Drawing keypad");
  setPage(HomePage);
  drawKeypad();

#ifdef touchInterruptPin
  if (generalconfig.sleepenable)
  {
    pinMode(touchInterruptPin, INPUT_PULLUP);
    Interval = generalconfig.sleeptimer * 60000;
    Serial.println("[INFO]: Sleep enabled.");
    Serial.print("[INFO]: Sleep timer = ");
    Serial.print(generalconfig.sleeptimer);
    Serial.println(" minutes");
    // Last page is Settings, 4th button is Timer
    screen.back()->button[3]->isLatched = 1;
  }
#endif
}

//--------------------- LOOP ---------------------------------------------------------------------

void loop(void)
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
  
  if (pageNum == EmptyPage)
  {

    // If the pageNum is set to 7, do not draw anything on screen or check for touch
    // and start handeling incoming web requests.
  }
  else if (pageNum == InfoPage)
  {

    if (!displayinginfo)
    {
      printinfo();
    }

    uint16_t t_x = 0, t_y = 0;

    //At the beginning of a new loop, make sure we do not use last loop's touch.
    boolean pressed = false;

#ifdef USECAPTOUCH
    if (ts.touched())
    {

      // Retrieve a point
      TS_Point p = ts.getPoint();

      //Flip things around so it matches our screen rotation
      p.x = map(p.x, 0, 320, 320, 0);
      t_y = p.x;
      t_x = p.y;

      pressed = true;
    }

#else

    pressed = tft.getTouch(&t_x, &t_y);

#endif

    if (pressed)
    {     
      displayinginfo = false;
      setPage(SettingsPage);
      tft.fillScreen(generalconfig.backgroundColour);
      drawKeypad();
    }
  }
  else if (pageNum == WifiErrorPage)
  {

    // We were unable to connect to WiFi. Waiting for touch to get back to the settings menu.
    uint16_t t_x = 0, t_y = 0;

    //At the beginning of a new loop, make sure we do not use last loop's touch.
    boolean pressed = false;

#ifdef USECAPTOUCH
    if (ts.touched())
    {

      // Retrieve a point
      TS_Point p = ts.getPoint();

      //Flip things around so it matches our screen rotation
      p.x = map(p.x, 0, 320, 320, 0);
      t_y = p.x;
      t_x = p.y;

      pressed = true;
    }

#else

    pressed = tft.getTouch(&t_x, &t_y);

#endif

    if (pressed)
    {     
      // Return to Settings page
      displayinginfo = false;
      setPage(SettingsPage);
      tft.fillScreen(generalconfig.backgroundColour);
      drawKeypad();
    }
  }
  else if (pageNum == ConfigErrorPage)
  {

    // A JSON file failed to load. We are drawing an error message. And waiting for a touch.
    uint16_t t_x = 0, t_y = 0;

    //At the beginning of a new loop, make sure we do not use last loop's touch.
    boolean pressed = false;

#ifdef USECAPTOUCH
    if (ts.touched())
    {

      // Retrieve a point
      TS_Point p = ts.getPoint();

      //Flip things around so it matches our screen rotation
      p.x = map(p.x, 0, 320, 320, 0);
      t_y = p.x;
      t_x = p.y;

      pressed = true;
    }

#else

    pressed = tft.getTouch(&t_x, &t_y);

#endif

    if (pressed)
    {     
      // Load home screen
      displayinginfo = false;
      setPage(HomePage);
      tft.fillScreen(generalconfig.backgroundColour);
      drawKeypad();
    }
  }
  else
  {

    // Check if sleep is enabled and if our timer has ended.

#ifdef touchInterruptPin
    if (generalconfig.sleepenable)
    {
      if (millis() > previousMillis + Interval)
      {

        // The timer has ended and we are going to sleep  .
        tft.fillScreen(TFT_BLACK);
        Serial.println("[INFO]: Going to sleep.");
#ifdef speakerPin
        if(generalconfig.beep){
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
#endif

        esp_sleep_enable_ext0_wakeup(touchInterruptPin, 0);
        esp_deep_sleep_start();
      }
    }
#endif

    // Touch coordinates are stored here
    uint16_t t_x = 0, t_y = 0;

    //At the beginning of a new loop, make sure we do not use last loop's touch.
    boolean pressed = false;

#ifdef USECAPTOUCH
    if (ts.touched())
    {

      // Retrieve a point
      TS_Point p = ts.getPoint();

      //Flip things around so it matches our screen rotation
      p.x = map(p.x, 0, 320, 320, 0);
      t_y = p.x;
      t_x = p.y;

      pressed = true;
    }

#else

    pressed = tft.getTouch(&t_x, &t_y);

#endif

    Screen* s = currentScreen();
    int pageKeyCount = s->cols * s->rows;

    // Check if the X and Y coordinates of the touch are within one of our buttons
    for (uint8_t b = 0; b < pageKeyCount; b++)
    {
      if (pressed && key[b]->contains(t_x, t_y))
      {
        key[b]->press(true); // tell the button it is pressed

        // After receiving a valid touch reset the sleep timer
        previousMillis = millis();
      }
      else
      {
        key[b]->press(false); // tell the button it is NOT pressed
      }
    }

    // Check if any key has changed state
    for (uint8_t b = 0; b < pageKeyCount; b++)
    {
      if (key[b]->justReleased())
      {

        // Draw normal button space (non inverted)

        int col = b % s->cols;
        int row = int(b / s->cols);

        uint16_t buttonBG;
        bool drawTransparent;

        uint16_t imageBGColor;

        if (b < s->button.size() && s->button[b]->isLatched)
        {
          imageBGColor = getLatchImageBG(b);
        }
        else
        {
          imageBGColor = getImageBG(b);
        }

        if (imageBGColor > 0)
        {
          buttonBG = imageBGColor;
          drawTransparent = false;
        }
        else
        {
          if (pageNum == HomePage)
          {
            buttonBG = generalconfig.menuButtonColour;
            drawTransparent = true;
          }
          else
          {
            if (b == s->cols * s->rows -1) // Home button
            {
              buttonBG = generalconfig.menuButtonColour;
              drawTransparent = true;
            }
            else
            {
              buttonBG = generalconfig.functionButtonColour;
              drawTransparent = true;
            }
          }
        }
        tft.setFreeFont(LABEL_FONT);
        key[b]->initButton(&tft, keyX() + col * (keyW() + keySpacingX()),
                          keyY() + row * (keyH() + keySpacingY()), // x, y, w, h, outline, fill, text
                          keyW(), keyH(), TFT_WHITE, buttonBG, TFT_WHITE,
                          "", KEY_TEXTSIZE);
        key[b]->drawButton();

        // After drawing the button outline we call this to draw a logo.
        if (b < s->button.size() && s->button[b]->isLatched)
        {
          drawlogo(b, col, row, drawTransparent, true);
        }
        else
        {
          drawlogo(b, col, row, drawTransparent, false);
        }
      }

      if (key[b]->justPressed())
      {

        // Beep
        #ifdef speakerPin
        if(generalconfig.beep){
          ledcAttachPin(speakerPin, 2);
          ledcWriteTone(2, 600);
          delay(50);
          ledcDetachPin(speakerPin);
          ledcWrite(2, 0);
        }
        #endif 
        
        int col = b % s->cols;
        int row = int(b / s->cols);

        tft.setFreeFont(LABEL_FONT);
        key[b]->initButton(&tft, keyX() + col * (keyW() + keySpacingX()),
                          keyY() + row * (keyH() + keySpacingY()), // x, y, w, h, outline, fill, text
                          keyW(), keyH(), TFT_WHITE, TFT_WHITE, TFT_WHITE,
                          "", KEY_TEXTSIZE);
        key[b]->drawButton();

        //---------------------------------------- Button press handeling --------------------------------------------------

        if (pageNum == HomePage) //Home menu
        {

          Screen * s = currentScreen();
          if (b == s->logo.size()-1) {
            // Last button on home screen pressed
            setPage(SettingsPage);
          }
          else {
            // otherwise set page corresponding to button pressed
            setPage(b+1);
          }
          drawKeypad();
        }
        else if (pageNum == SettingsPage)
        {
          if (b == 0) // Button 0
          {
            bleKeyboardAction(SpecialFnAction, 1, 0);
          }
          else if (b == 1) // Button 1
          {
            bleKeyboardAction(SpecialFnAction, 2, 0);
          }
          else if (b == 2) // Button 2
          {
            bleKeyboardAction(SpecialFnAction, 3, 0);
          }
          else if (b == 3) // Button 3
          {
            bleKeyboardAction(SpecialFnAction, 4, 0);
            s->button[b]->isLatched = !s->button[b]->isLatched;
          }
          else if (b == 4) // Button 4
          {
            setPage(InfoPage);
            drawKeypad();
          }
          else if (b == 5)
          {
            setPage(HomePage);
            drawKeypad();
          }
        }
        else
        {
          if (b < s->button.size()) {
            Button* but = s->button[b];

            bleKeyboardAction(but->actions.action0, but->actions.value0, but->actions.symbol0);
            bleKeyboardAction(but->actions.action1, but->actions.value1, but->actions.symbol1);
            bleKeyboardAction(but->actions.action2, but->actions.value2, but->actions.symbol2);
            bleKeyboard.releaseAll();
            if (but->latch)
            {
              but->isLatched = !but->isLatched;
            }
          }
          else if (b == s->rows * s->cols - 1) // Last Button / Back home
          {
            setPage(HomePage);
            drawKeypad();
          }
        }

        delay(10); // UI debouncing
      }
    }
  }
}
