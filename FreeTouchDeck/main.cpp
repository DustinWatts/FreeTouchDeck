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

const char *versionnumber = "0.9.13";

    /* Version 0.9.13.
     *  
     * Fix: Issue #54 Not all filenames are allowed for uploading.
     * Fix: Increase the size of DynamicJsonDocument to 1500 to support larger configs
     * Fix in ESP32-BLE-Keyboard: Changed to 115 keys (for F13-F24), added NUMPAD keys
     * 
     * Added: NUMPAD keys to configurator and Action.h (action 12)
     * Added: PrintScreen, HOME, END, and ESCAPE to Action.h and configurator
     *
     * 
     * 
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

// Keypad start position, centre of the first button
#define KEY_X SCREEN_WIDTH / 6
#define KEY_Y SCREEN_HEIGHT / 4

// Gaps between buttons
#define KEY_SPACING_X SCREEN_WIDTH / 24
#define KEY_SPACING_Y SCREEN_HEIGHT / 16

// Width and height of a button
#define KEY_W (SCREEN_WIDTH / 3) - KEY_SPACING_X
#define KEY_H (SCREEN_WIDTH / 3) - KEY_SPACING_Y

// Font size multiplier
#define KEY_TEXTSIZE 1

// Text Button Label Font
#define LABEL_FONT &FreeSansBold12pt7b

// placeholder for the pagenumber we are on (0 indicates home)
int pageNum = 0;

// Initial LED brightness
int ledBrightness = 255;

// Every button has a row associated with it
uint8_t rowArray[6] = {0, 0, 0, 1, 1, 1};
// Every button has a column associated with it
uint8_t colArray[6] = {0, 1, 2, 0, 1, 2};

//path to the directory the logo are in ! including leading AND trailing / !
char logopath[64] = "/logos/";

// templogopath is used to hold the complete path of an image. It is empty for now.
char templogopath[64] = "";

// Struct to hold the logos per screen
struct Logos
{
  char logo0[32];
  char logo1[32];
  char logo2[32];
  char logo3[32];
  char logo4[32];
  char logo5[32];
};

// Struct Action: 3 actions and 3 values per button
struct Actions
{
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
  char latchlogo[32];
};

// Each menu has 6 buttons
struct Menu
{
  struct Button button0;
  struct Button button1;
  struct Button button2;
  struct Button button3;
  struct Button button4;
  struct Button button5;
};

// Struct to hold the general logos.
struct Generallogos
{
  char homebutton[64];
  char configurator[64];
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

// Array to hold all the latching statuses
bool islatched[30] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

// Create instances of the structs
Wificonfig wificonfig;

Config generalconfig;

Generallogos generallogo;

Logos screen0;
Logos screen1;
Logos screen2;
Logos screen3;
Logos screen4;
Logos screen5;
Logos screen6;

Menu menu1;
Menu menu2;
Menu menu3;
Menu menu4;
Menu menu5;
Menu menu6;

unsigned long previousMillis = 0;
unsigned long Interval = 0;
bool displayinginfo;
char* jsonfilefail = "";

// Invoke the TFT_eSPI button class and create all the button objects
TFT_eSPI_Button key[6];

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
  Serial.begin(115200);
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
    drawBmp("/logos/freetouchdeck_logo.bmp", 0, 0);
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

  if (!checkfile("/config/menu1.json"))
  {
    Serial.println("[ERROR]: /config/menu1.json not found!");
    while (1)
      yield(); // Stop!
  }

  if (!checkfile("/config/menu2.json"))
  {
    Serial.println("[ERROR]: /config/menu2.json not found!");
    while (1)
      yield(); // Stop!
  }

  if (!checkfile("/config/menu3.json"))
  {
    Serial.println("[ERROR]: /config/menu3.json not found!");
    while (1)
      yield(); // Stop!
  }

  if (!checkfile("/config/menu4.json"))
  {
    Serial.println("[ERROR]: /config/menu4.json not found!");
    while (1)
      yield(); // Stop!
  }

  if (!checkfile("/config/menu5.json"))
  {
    Serial.println("[ERROR]: /config/menu5.json not found!");
    while (1)
      yield(); // Stop!
  }

  // After checking the config files exist, actually load them
  if(!loadConfig("general")){
    Serial.println("[WARNING]: general.json seems to be corrupted!");
    Serial.println("[WARNING]: To reset to default type 'reset general'.");
    jsonfilefail = "general";
    pageNum = 10;
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
    pageNum = 10;
  }
  if(!loadConfig("menu1")){
    Serial.println("[WARNING]: menu1.json seems to be corrupted!");
    Serial.println("[WARNING]: To reset to default type 'reset menu1'.");
    jsonfilefail = "menu1";
    pageNum = 10;
  }
  if(!loadConfig("menu2")){
    Serial.println("[WARNING]: menu2.json seems to be corrupted!");
    Serial.println("[WARNING]: To reset to default type 'reset menu2'.");
    jsonfilefail = "menu2";
    pageNum = 10;
  }
  if(!loadConfig("menu3")){
    Serial.println("[WARNING]: menu3.json seems to be corrupted!");
    Serial.println("[WARNING]: To reset to default type 'reset menu3'.");
    jsonfilefail = "menu3";
    pageNum = 10;
  }
  if(!loadConfig("menu4")){
    Serial.println("[WARNING]: menu4.json seems to be corrupted!");
    Serial.println("[WARNING]: To reset to default type 'reset menu4'.");
    jsonfilefail = "menu4";
    pageNum = 10;
  }
  if(!loadConfig("menu5")){
    Serial.println("[WARNING]: menu5.json seems to be corrupted!");
    Serial.println("[WARNING]: To reset to default type 'reset menu5'.");
    jsonfilefail = "menu5";
    pageNum = 10;
  }
  Serial.println("[INFO]: All configs loaded");

  

  strcpy(generallogo.homebutton, "/logos/home.bmp");
  strcpy(generallogo.configurator, "/logos/wifi.bmp");
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
    islatched[28] = 1;
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
  
  if (pageNum == 7)
  {

    // If the pageNum is set to 7, do not draw anything on screen or check for touch
    // and start handeling incomming web requests.
  }
  else if (pageNum == 8)
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
      pageNum = 6;
      tft.fillScreen(generalconfig.backgroundColour);
      drawKeypad();
    }
  }
  else if (pageNum == 9)
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
      pageNum = 6;
      tft.fillScreen(generalconfig.backgroundColour);
      drawKeypad();
    }
  }
  else if (pageNum == 10)
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
      pageNum = 0;
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

    // Check if the X and Y coordinates of the touch are within one of our buttons
    for (uint8_t b = 0; b < 6; b++)
    {
      if (pressed && key[b].contains(t_x, t_y))
      {
        key[b].press(true); // tell the button it is pressed

        // After receiving a valid touch reset the sleep timer
        previousMillis = millis();
      }
      else
      {
        key[b].press(false); // tell the button it is NOT pressed
      }
    }

    // Check if any key has changed state
    for (uint8_t b = 0; b < 6; b++)
    {
      if (key[b].justReleased())
      {

        // Draw normal button space (non inverted)

        int col, row;

        if (b == 0)
        {
          col = 0;
          row = 0;
        }
        else if (b == 1)
        {
          col = 1;
          row = 0;
        }
        else if (b == 2)
        {
          col = 2;
          row = 0;
        }
        else if (b == 3)
        {
          col = 0;
          row = 1;
        }
        else if (b == 4)
        {
          col = 1;
          row = 1;
        }
        else if (b == 5)
        {
          col = 2;
          row = 1;
        }

        int index;

        if (pageNum == 2)
        {
          index = b + 5;
        }
        else if (pageNum == 3)
        {
          index = b + 10;
        }
        else if (pageNum == 4)
        {
          index = b + 15;
        }
        else if (pageNum == 5)
        {
          index = b + 20;
        }
        else if (pageNum == 6)
        {
          index = b + 25;
        }
        else
        {
          index = b;
        }

        uint16_t buttonBG;
        bool drawTransparent;

        uint16_t imageBGColor;
        if (islatched[index] && b < 5)
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
          if (pageNum == 0)
          {
            buttonBG = generalconfig.menuButtonColour;
            drawTransparent = true;
          }
          else
          {
            if (pageNum == 6 && b == 5)
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
        key[b].initButton(&tft, KEY_X + col * (KEY_W + KEY_SPACING_X),
                          KEY_Y + row * (KEY_H + KEY_SPACING_Y), // x, y, w, h, outline, fill, text
                          KEY_W, KEY_H, TFT_WHITE, buttonBG, TFT_WHITE,
                          "", KEY_TEXTSIZE);
        key[b].drawButton();

        // After drawing the button outline we call this to draw a logo.
        if (islatched[index] && b < 5)
        {
          drawlogo(b, col, row, drawTransparent, true);
        }
        else
        {
          drawlogo(b, col, row, drawTransparent, false);
        }
      }

      if (key[b].justPressed())
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
        
        int col, row;

        if (b == 0)
        {
          col = 0;
          row = 0;
        }
        else if (b == 1)
        {
          col = 1;
          row = 0;
        }
        else if (b == 2)
        {
          col = 2;
          row = 0;
        }
        else if (b == 3)
        {
          col = 0;
          row = 1;
        }
        else if (b == 4)
        {
          col = 1;
          row = 1;
        }
        else if (b == 5)
        {
          col = 2;
          row = 1;
        }

        tft.setFreeFont(LABEL_FONT);
        key[b].initButton(&tft, KEY_X + col * (KEY_W + KEY_SPACING_X),
                          KEY_Y + row * (KEY_H + KEY_SPACING_Y), // x, y, w, h, outline, fill, text
                          KEY_W, KEY_H, TFT_WHITE, TFT_WHITE, TFT_WHITE,
                          "", KEY_TEXTSIZE);
        key[b].drawButton();

        //---------------------------------------- Button press handeling --------------------------------------------------

        if (pageNum == 0) //Home menu
        {
          if (b == 0) // Button 0
          {
            pageNum = 1;
            drawKeypad();
          }
          else if (b == 1) // Button 1
          {
            pageNum = 2;
            drawKeypad();
          }
          else if (b == 2) // Button 2
          {
            pageNum = 3;
            drawKeypad();
          }
          else if (b == 3) // Button 3
          {
            pageNum = 4;
            drawKeypad();
          }
          else if (b == 4) // Button 4
          {
            pageNum = 5;
            drawKeypad();
          }
          else if (b == 5) // Button 5
          {
            pageNum = 6;
            drawKeypad();
          }
        }

        else if (pageNum == 1) // Menu 1
        {
          if (b == 0) // Button 0
          {
            bleKeyboardAction(menu1.button0.actions.action0, menu1.button0.actions.value0, menu1.button0.actions.symbol0);
            bleKeyboardAction(menu1.button0.actions.action1, menu1.button0.actions.value1, menu1.button0.actions.symbol1);
            bleKeyboardAction(menu1.button0.actions.action2, menu1.button0.actions.value2, menu1.button0.actions.symbol2);
            bleKeyboard.releaseAll();
            if (menu1.button0.latch)
            {
              if (islatched[0])
              {
                islatched[0] = 0;
              }
              else
              {
                islatched[0] = 1;
              }
            }
          }
          else if (b == 1) // Button 1
          {
            bleKeyboardAction(menu1.button1.actions.action0, menu1.button1.actions.value0, menu1.button1.actions.symbol0);
            bleKeyboardAction(menu1.button1.actions.action1, menu1.button1.actions.value1, menu1.button1.actions.symbol1);
            bleKeyboardAction(menu1.button1.actions.action2, menu1.button1.actions.value2, menu1.button1.actions.symbol2);
            bleKeyboard.releaseAll();
            if (menu1.button1.latch)
            {
              if (islatched[1])
              {
                islatched[1] = 0;
              }
              else
              {
                islatched[1] = 1;
              }
            }
          }
          else if (b == 2) // Button 2
          {
            bleKeyboardAction(menu1.button2.actions.action0, menu1.button2.actions.value0, menu1.button2.actions.symbol0);
            bleKeyboardAction(menu1.button2.actions.action1, menu1.button2.actions.value1, menu1.button2.actions.symbol1);
            bleKeyboardAction(menu1.button2.actions.action2, menu1.button2.actions.value2, menu1.button2.actions.symbol2);
            bleKeyboard.releaseAll();
            if (menu1.button2.latch)
            {
              if (islatched[2])
              {
                islatched[2] = 0;
              }
              else
              {
                islatched[2] = 1;
              }
            }
          }
          else if (b == 3) // Button 3
          {
            bleKeyboardAction(menu1.button3.actions.action0, menu1.button3.actions.value0, menu1.button3.actions.symbol0);
            bleKeyboardAction(menu1.button3.actions.action1, menu1.button3.actions.value1, menu1.button3.actions.symbol1);
            bleKeyboardAction(menu1.button3.actions.action2, menu1.button3.actions.value2, menu1.button3.actions.symbol2);
            bleKeyboard.releaseAll();
            if (menu1.button3.latch)
            {
              if (islatched[3])
              {
                islatched[3] = 0;
              }
              else
              {
                islatched[3] = 1;
              }
            }
          }
          else if (b == 4) // Button 4
          {
            bleKeyboardAction(menu1.button4.actions.action0, menu1.button4.actions.value0, menu1.button4.actions.symbol0);
            bleKeyboardAction(menu1.button4.actions.action1, menu1.button4.actions.value1, menu1.button4.actions.symbol1);
            bleKeyboardAction(menu1.button4.actions.action2, menu1.button4.actions.value2, menu1.button4.actions.symbol2);
            bleKeyboard.releaseAll();
            if (menu1.button4.latch)
            {
              if (islatched[4])
              {
                islatched[4] = 0;
              }
              else
              {
                islatched[4] = 1;
              }
            }
          }
          else if (b == 5) // Button 5 / Back home
          {
            pageNum = 0;
            drawKeypad();
          }
        }

        else if (pageNum == 2) // Menu 2
        {
          if (b == 0) // Button 0
          {
            bleKeyboardAction(menu2.button0.actions.action0, menu2.button0.actions.value0, menu2.button0.actions.symbol0);
            bleKeyboardAction(menu2.button0.actions.action1, menu2.button0.actions.value1, menu2.button0.actions.symbol1);
            bleKeyboardAction(menu2.button0.actions.action2, menu2.button0.actions.value2, menu2.button0.actions.symbol2);
            bleKeyboard.releaseAll();
            if (menu2.button0.latch)
            {
              if (islatched[5])
              {
                islatched[5] = 0;
              }
              else
              {
                islatched[5] = 1;
              }
            }
          }
          else if (b == 1) // Button 1
          {
            bleKeyboardAction(menu2.button1.actions.action0, menu2.button1.actions.value0, menu2.button1.actions.symbol0);
            bleKeyboardAction(menu2.button1.actions.action1, menu2.button1.actions.value1, menu2.button1.actions.symbol1);
            bleKeyboardAction(menu2.button1.actions.action2, menu2.button1.actions.value2, menu2.button1.actions.symbol2);
            bleKeyboard.releaseAll();
            if (menu2.button1.latch)
            {
              if (islatched[6])
              {
                islatched[6] = 0;
              }
              else
              {
                islatched[6] = 1;
              }
            }
          }
          else if (b == 2) // Button 2
          {
            bleKeyboardAction(menu2.button2.actions.action0, menu2.button2.actions.value0, menu2.button2.actions.symbol0);
            bleKeyboardAction(menu2.button2.actions.action1, menu2.button2.actions.value1, menu2.button2.actions.symbol1);
            bleKeyboardAction(menu2.button2.actions.action2, menu2.button2.actions.value2, menu2.button2.actions.symbol2);
            bleKeyboard.releaseAll();
            if (menu2.button2.latch)
            {
              if (islatched[7])
              {
                islatched[7] = 0;
              }
              else
              {
                islatched[7] = 1;
              }
            }
          }
          else if (b == 3) // Button 3
          {
            bleKeyboardAction(menu2.button3.actions.action0, menu2.button3.actions.value0, menu2.button3.actions.symbol0);
            bleKeyboardAction(menu2.button3.actions.action1, menu2.button3.actions.value1, menu2.button3.actions.symbol1);
            bleKeyboardAction(menu2.button3.actions.action2, menu2.button3.actions.value2, menu2.button3.actions.symbol2);
            bleKeyboard.releaseAll();
            if (menu2.button3.latch)
            {
              if (islatched[8])
              {
                islatched[8] = 0;
              }
              else
              {
                islatched[8] = 1;
              }
            }
          }
          else if (b == 4) // Button 4
          {
            bleKeyboardAction(menu2.button4.actions.action0, menu2.button4.actions.value0, menu2.button4.actions.symbol0);
            bleKeyboardAction(menu2.button4.actions.action1, menu2.button4.actions.value1, menu2.button4.actions.symbol1);
            bleKeyboardAction(menu2.button4.actions.action2, menu2.button4.actions.value2, menu2.button4.actions.symbol2);
            bleKeyboard.releaseAll();
            if (menu2.button4.latch)
            {
              if (islatched[9])
              {
                islatched[9] = 0;
              }
              else
              {
                islatched[9] = 1;
              }
            }
          }
          else if (b == 5) // Button 5 / Back home
          {
            pageNum = 0;
            drawKeypad();
          }
        }

        else if (pageNum == 3) // Menu 3
        {
          if (b == 0) // Button 0
          {
            bleKeyboardAction(menu3.button0.actions.action0, menu3.button0.actions.value0, menu3.button0.actions.symbol0);
            bleKeyboardAction(menu3.button0.actions.action1, menu3.button0.actions.value1, menu3.button0.actions.symbol1);
            bleKeyboardAction(menu3.button0.actions.action2, menu3.button0.actions.value2, menu3.button0.actions.symbol2);
            bleKeyboard.releaseAll();
            if (menu3.button0.latch)
            {
              if (islatched[10])
              {
                islatched[10] = 0;
              }
              else
              {
                islatched[10] = 1;
              }
            }
          }
          else if (b == 1) // Button 1
          {
            bleKeyboardAction(menu3.button1.actions.action0, menu3.button1.actions.value0, menu3.button1.actions.symbol0);
            bleKeyboardAction(menu3.button1.actions.action1, menu3.button1.actions.value1, menu3.button1.actions.symbol1);
            bleKeyboardAction(menu3.button1.actions.action2, menu3.button1.actions.value2, menu3.button1.actions.symbol2);
            bleKeyboard.releaseAll();
            if (menu3.button1.latch)
            {
              if (islatched[11])
              {
                islatched[11] = 0;
              }
              else
              {
                islatched[11] = 1;
              }
            }
          }
          else if (b == 2) // Button 2
          {
            bleKeyboardAction(menu3.button2.actions.action0, menu3.button2.actions.value0, menu3.button2.actions.symbol0);
            bleKeyboardAction(menu3.button2.actions.action1, menu3.button2.actions.value1, menu3.button2.actions.symbol1);
            bleKeyboardAction(menu3.button2.actions.action2, menu3.button2.actions.value2, menu3.button2.actions.symbol2);
            bleKeyboard.releaseAll();
            if (menu3.button2.latch)
            {
              if (islatched[12])
              {
                islatched[12] = 0;
              }
              else
              {
                islatched[12] = 1;
              }
            }
          }
          else if (b == 3) // Button 3
          {
            bleKeyboardAction(menu3.button3.actions.action0, menu3.button3.actions.value0, menu3.button3.actions.symbol0);
            bleKeyboardAction(menu3.button3.actions.action1, menu3.button3.actions.value1, menu3.button3.actions.symbol1);
            bleKeyboardAction(menu3.button3.actions.action2, menu3.button3.actions.value2, menu3.button3.actions.symbol2);
            bleKeyboard.releaseAll();
            if (menu3.button3.latch)
            {
              if (islatched[13])
              {
                islatched[13] = 0;
              }
              else
              {
                islatched[13] = 1;
              }
            }
          }
          else if (b == 4) // Button 4
          {
            bleKeyboardAction(menu3.button4.actions.action0, menu3.button4.actions.value0, menu3.button4.actions.symbol0);
            bleKeyboardAction(menu3.button4.actions.action1, menu3.button4.actions.value1, menu3.button4.actions.symbol1);
            bleKeyboardAction(menu3.button4.actions.action2, menu3.button4.actions.value2, menu3.button4.actions.symbol2);
            bleKeyboard.releaseAll();
            if (menu3.button4.latch)
            {
              if (islatched[14])
              {
                islatched[14] = 0;
              }
              else
              {
                islatched[14] = 1;
              }
            }
          }
          else if (b == 5) // Button 5 / Back home
          {
            pageNum = 0;
            drawKeypad();
          }
        }

        else if (pageNum == 4) // Menu 4
        {
          if (b == 0) // Button 0
          {
            bleKeyboardAction(menu4.button0.actions.action0, menu4.button0.actions.value0, menu4.button0.actions.symbol0);
            bleKeyboardAction(menu4.button0.actions.action1, menu4.button0.actions.value1, menu4.button0.actions.symbol1);
            bleKeyboardAction(menu4.button0.actions.action2, menu4.button0.actions.value2, menu4.button0.actions.symbol2);
            bleKeyboard.releaseAll();
            if (menu4.button0.latch)
            {
              if (islatched[15])
              {
                islatched[15] = 0;
              }
              else
              {
                islatched[15] = 1;
              }
            }
          }
          else if (b == 1) // Button 1
          {
            bleKeyboardAction(menu4.button1.actions.action0, menu4.button1.actions.value0, menu4.button1.actions.symbol0);
            bleKeyboardAction(menu4.button1.actions.action1, menu4.button1.actions.value1, menu4.button1.actions.symbol1);
            bleKeyboardAction(menu4.button1.actions.action2, menu4.button1.actions.value2, menu4.button1.actions.symbol2);
            bleKeyboard.releaseAll();
            if (menu4.button1.latch)
            {
              if (islatched[16])
              {
                islatched[16] = 0;
              }
              else
              {
                islatched[16] = 1;
              }
            }
          }
          else if (b == 2) // Button 2
          {
            bleKeyboardAction(menu4.button2.actions.action0, menu4.button2.actions.value0, menu4.button2.actions.symbol0);
            bleKeyboardAction(menu4.button2.actions.action1, menu4.button2.actions.value1, menu4.button2.actions.symbol1);
            bleKeyboardAction(menu4.button2.actions.action2, menu4.button2.actions.value2, menu4.button2.actions.symbol2);
            bleKeyboard.releaseAll();
            if (menu4.button2.latch)
            {
              if (islatched[17])
              {
                islatched[17] = 0;
              }
              else
              {
                islatched[17] = 1;
              }
            }
          }
          else if (b == 3) // Button 3
          {
            bleKeyboardAction(menu4.button3.actions.action0, menu4.button3.actions.value0, menu4.button3.actions.symbol0);
            bleKeyboardAction(menu4.button3.actions.action1, menu4.button3.actions.value1, menu4.button3.actions.symbol1);
            bleKeyboardAction(menu4.button3.actions.action2, menu4.button3.actions.value2, menu4.button3.actions.symbol2);
            bleKeyboard.releaseAll();
            if (menu4.button3.latch)
            {
              if (islatched[18])
              {
                islatched[18] = 0;
              }
              else
              {
                islatched[18] = 1;
              }
            }
          }
          else if (b == 4) // Button 4
          {
            bleKeyboardAction(menu4.button4.actions.action0, menu4.button4.actions.value0, menu4.button4.actions.symbol0);
            bleKeyboardAction(menu4.button4.actions.action1, menu4.button4.actions.value1, menu4.button4.actions.symbol1);
            bleKeyboardAction(menu4.button4.actions.action2, menu4.button4.actions.value2, menu4.button4.actions.symbol2);
            bleKeyboard.releaseAll();
            if (menu4.button4.latch)
            {
              if (islatched[19])
              {
                islatched[19] = 0;
              }
              else
              {
                islatched[19] = 1;
              }
            }
          }
          else if (b == 5) // Button 5 / Back home
          {
            pageNum = 0;
            drawKeypad();
          }
        }

        else if (pageNum == 5) // Menu 5
        {
          if (b == 0) // Button 0
          {
            bleKeyboardAction(menu5.button0.actions.action0, menu5.button0.actions.value0, menu5.button0.actions.symbol0);
            bleKeyboardAction(menu5.button0.actions.action1, menu5.button0.actions.value1, menu5.button0.actions.symbol1);
            bleKeyboardAction(menu5.button0.actions.action2, menu5.button0.actions.value2, menu5.button0.actions.symbol2);
            bleKeyboard.releaseAll();
            if (menu5.button0.latch)
            {
              if (islatched[20])
              {
                islatched[20] = 0;
              }
              else
              {
                islatched[20] = 1;
              }
            }
          }
          else if (b == 1) // Button 1
          {
            bleKeyboardAction(menu5.button1.actions.action0, menu5.button1.actions.value0, menu5.button1.actions.symbol0);
            bleKeyboardAction(menu5.button1.actions.action1, menu5.button1.actions.value1, menu5.button1.actions.symbol1);
            bleKeyboardAction(menu5.button1.actions.action2, menu5.button1.actions.value2, menu5.button1.actions.symbol2);
            bleKeyboard.releaseAll();
            if (menu5.button1.latch)
            {
              if (islatched[21])
              {
                islatched[21] = 0;
              }
              else
              {
                islatched[21] = 1;
              }
            }
          }
          else if (b == 2) // Button 2
          {
            bleKeyboardAction(menu5.button2.actions.action0, menu5.button2.actions.value0, menu5.button2.actions.symbol0);
            bleKeyboardAction(menu5.button2.actions.action1, menu5.button2.actions.value1, menu5.button2.actions.symbol1);
            bleKeyboardAction(menu5.button2.actions.action2, menu5.button2.actions.value2, menu5.button2.actions.symbol2);
            bleKeyboard.releaseAll();
            if (menu5.button2.latch)
            {
              if (islatched[22])
              {
                islatched[22] = 0;
              }
              else
              {
                islatched[22] = 1;
              }
            }
          }
          else if (b == 3) // Button 3
          {
            bleKeyboardAction(menu5.button3.actions.action0, menu5.button3.actions.value0, menu5.button3.actions.symbol0);
            bleKeyboardAction(menu5.button3.actions.action1, menu5.button3.actions.value1, menu5.button3.actions.symbol1);
            bleKeyboardAction(menu5.button3.actions.action2, menu5.button3.actions.value2, menu5.button3.actions.symbol2);
            bleKeyboard.releaseAll();
            if (menu5.button3.latch)
            {
              if (islatched[23])
              {
                islatched[23] = 0;
              }
              else
              {
                islatched[23] = 1;
              }
            }
          }
          else if (b == 4) // Button 4
          {
            bleKeyboardAction(menu5.button4.actions.action0, menu5.button4.actions.value0, menu5.button4.actions.symbol0);
            bleKeyboardAction(menu5.button4.actions.action1, menu5.button4.actions.value1, menu5.button4.actions.symbol1);
            bleKeyboardAction(menu5.button4.actions.action2, menu5.button4.actions.value2, menu5.button4.actions.symbol2);
            bleKeyboard.releaseAll();
            if (menu5.button4.latch)
            {
              if (islatched[24])
              {
                islatched[24] = 0;
              }
              else
              {
                islatched[24] = 1;
              }
            }
          }
          else if (b == 5) // Button 5 / Back home
          {
            pageNum = 0;
            drawKeypad();
          }
        }

        else if (pageNum == 6) // Settings page
        {
          if (b == 0) // Button 0
          {
            bleKeyboardAction(11, 1, 0);
          }
          else if (b == 1) // Button 1
          {
            bleKeyboardAction(11, 2, 0);
          }
          else if (b == 2) // Button 2
          {
            bleKeyboardAction(11, 3, 0);
          }
          else if (b == 3) // Button 3
          {
            bleKeyboardAction(11, 4, 0);
            if (islatched[28])
            {
              islatched[28] = 0;
            }
            else
            {
              islatched[28] = 1;
            }
          }
          else if (b == 4) // Button 4
          {
            pageNum = 8;
            drawKeypad();
          }
          else if (b == 5)
          {
            pageNum = 0;
            drawKeypad();
          }
        }

        delay(10); // UI debouncing
      }
    }
  }
}
