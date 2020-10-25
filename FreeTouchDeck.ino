/*
  FreeTouchDeck (secret codename) based on the FreeDeck idea by Koriwi.
  It uses the TFT_eSPI library by Bodmer for the display and touch functionality and
  the ESP32-BLE-Keyboard library by T-vK. For loading configuration it uses 
  ArduinoJson V6.

  FreeTouchDeck uses a few (4 or 5 depending wether you use resistive or capacitive touch) 
  libraries from other sources. These must be installed for FreeTouchDeck to compile and run. 
  
  These are those libraries:

      !----------------------------- Library Dependencies --------------------------- !
      - Adafruit-GFX-Library (version 1.10.0), available through Library Manager
      - TFT_eSPI (version 2.2.14), available through Library Manager
      - ESP32-BLE-Keyboard (latest version) download from: https://github.com/T-vK/ESP32-BLE-Keyboard
      - ArduinoJson (version 6.16.1), available through Library Manager

      --- If you use Capacitive touch ---
      - Dustin Watts FT6236 Library (version 1.0.1), https://github.com/DustinWatts/FT6236
      

  As this is an early Pre-alpha version, the code is ugly and sometimes way more complicated
  then necessary. It also lacks good documentation and comments in the code.

  The FILESYSTEM (SPI FLASH filing system) is used to hold touch screen calibration data.
  It has to be runs at least once. After that you can set REPEAT_CAL to false (default).

  !-- Make sure you have setup your TFT display and ESP setup correctly in TFT_eSPI/user_setup.h --!
        
        Select the right screen driver and the board (ESP32 is the only one tested) you are
        using. Also make sure TOUCH_CS is defined correctly. TFT_BL can also be needed!
  
*/

// ------- Uncomment the next line if you use capacitive touch -------
//#define USECAPTOUCH

// PAY ATTENTION! Even if resistive touch is not used, the TOUCH pin has to be defined!
// It can be a random unused pin.
// TODO: Find a way around this! 

// ------- Uncomment the define below if you want to use SLEEP and wake up on touch -------
// The pin where the IRQ from the touch screen is connected uses ESP-style GPIO_NUM_* instead of just pinnumber
//#define touchInterruptPin GPIO_NUM_27

// ------- Uncomment the define below if you want to use a piezo buzzer and specify the pin where the speaker is connected -------
//#define speakerPin 26

#include <pgmspace.h> // PROGMEM support header
#include "FS.h"       // File System header

#include <TFT_eSPI.h> // The TFT_eSPI library

#include <BleKeyboard.h> // BleKeyboard is used to communicate over BLE
#include "BLEDevice.h"   // Additional BLE functionaity
#include "BLEUtils.h"    // Additional BLE functionaity
#include "BLEBeacon.h"   // Additional BLE functionaity
#include "esp_sleep.h"   // Additional BLE functionaity

#include <ArduinoJson.h> // Using ArduinoJson to read and write config files

#include <WiFi.h>       // Wifi support
#include <WiFiClient.h> // Wifi support
#include <WebServer.h>  // Webserver functionality
#include <ESPmDNS.h>    // DNS functionality

#ifdef USECAPTOUCH
#include <FT6236.h>
FT6236 ts = FT6236();
#endif

BleKeyboard bleKeyboard("NewFreeTouchDeck", "Made by me");

WebServer server(80);

File fsUploadFile;

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
};

struct Wificonfig
{
  char ssid[64];
  char password[64];
  char hostname[64];
  bool sleepenable;
  uint16_t sleeptimer;
};

// Array to hold all the latching statuses
bool islatched[30] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

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

// Invoke the TFT_eSPI button class and create all the button objects
TFT_eSPI_Button key[6];

//-------------------------------- SETUP --------------------------------------------------------------

void setup()
{

  // Use serial port
  Serial.begin(9600);
  Serial.println("");

  #ifdef USECAPTOUCH
    if (!ts.begin(40)) { 
      Serial.println("[WARNING]: Unable to start the capacitive touchscreen.");
    } 
    else { 
      Serial.println("[INFO]: Capacitive touch started!");
    }
  #endif

  // Setup PWM channel and attach pin 32
  ledcSetup(0, 5000, 8);
  ledcAttachPin(32, 0);
  ledcWrite(0, ledBrightness); // Start @ initial Brightness

  // Setup PWM channel for Piezo speaker

  #ifdef speakerPin
  ledcSetup(1, 500, 8);
  
  ledcAttachPin(speakerPin, 0);
  ledcWriteTone(1, 600);
  delay(150);
  ledcDetachPin(speakerPin);
  ledcWrite(1, 0);

  ledcAttachPin(speakerPin, 0);
  ledcWriteTone(1, 800);
  delay(150);
  ledcDetachPin(speakerPin);
  ledcWrite(1, 0);

  ledcAttachPin(speakerPin, 0);
  ledcWriteTone(1, 1200);
  delay(150);
  ledcDetachPin(speakerPin);
  ledcWrite(1, 0);

  #endif

    if (!FILESYSTEM.begin())
    {
      Serial.println("[WARNING]: SPIFFS initialisation failed!");
      while (1)
        yield(); // We stop here
    }
    Serial.println("[INFO]: SPIFFS initialised.");

  //------------------ Load Wifi Config ----------------------------------------------

  Serial.println("[INFO]: Loading Wifi Config");
  if(!loadMainConfig())
  {
    Serial.println("[WARNING]: Failed to load WiFi Credentials!");
  }else{
    Serial.println("[INFO]: WiFi Credentials Loaded");
  }
  

  //------------------TFT/Touch Initialization ------------------------------------------------------------------------

  // Initialise the TFT screen
  tft.init();

  // Set the rotation before we calibrate
  tft.setRotation(1);

  // Clear the screen
  tft.fillScreen(TFT_BLACK);

  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();

  // If we are woken up we do not need the splash screen
  if(wakeup_reason > 0){

      // Don't draw splash screen

  } else {

  // Draw a splash screen
  drawBmp("/freetouchdeck_logo.bmp", 0, 0);
  tft.setCursor(1, 3);
  tft.setTextFont(2);
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  /* Version 0.8.18 Added support for Deep Sleep. Use #define touchInterruptPin and "sleepenable": true + "sleeptimer": 10 in wificonfig.json
   * to enable Deep Sleep and set the sleeptimer in minutes. 
   * Also added support for a buzzer (piezo speaker) use #define speakerPin to select the pin where the speaker is connected.
   * For both: when the #defines are commented out, they are not used.
   * Some other changes: debug.h is removed. And wificonfig.json is modified to add the option to change wether sleep is
   * enabled and to set the sleep timer.
  */

  tft.print("Loading version 0.8.18");
  Serial.println("[INFO]: Loading version 0.8.18");
    
  }

  // Calibrate the touch screen and retrieve the scaling factors

  #ifndef USECAPTOUCH
  touch_calibrate();
  #endif

  // Let's first check if all the files we need exist

  if (!checkfile("/config/colors.json"))
  {
    Serial.println("[ERROR]: /config/colors.json not found!");
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

  // Load the all the configuration
  loadConfig("colors");
  loadConfig("homescreen");
  loadConfig("menu1");
  loadConfig("menu2");
  loadConfig("menu3");
  loadConfig("menu4");
  loadConfig("menu5");
  Serial.println("[INFO]: All configs loaded");

  strcpy(generallogo.homebutton, "/logos/home.bmp");
  strcpy(generallogo.configurator, "/logos/wifi.bmp");
  Serial.println("[INFO]: General logo's loaded.");

  // Setup the Font used for plain text
  tft.setFreeFont(LABEL_FONT);  

  //------------------Webserver Initialization ------------------------------------------------------------------------

  // Restart handle
  server.on("/restart", HTTP_POST, handleRestart);

  server.onNotFound([]() {
    if (!handleFileRead(server.uri()))
    {
      server.send(404, "text/plain", "FileNotFound");
    }
  });

  server.on("/upload", HTTP_GET, []() {                 // if the client requests the upload page
    if (!handleFileRead("/upload.htm"))                 // send it if it exists
      server.send(404, "text/plain", "404: Not Found"); // otherwise, respond with a 404 (Not Found) error
  });

  server.on(
      "/upload", HTTP_POST,       // if the client posts to the upload page
      []() { server.send(200); }, // Send status 200 (OK) to tell the client we are ready to receive
      handleFileUpload            // Receive and save the file
  );

  // Save config handle
  server.on("/saveconfig", HTTP_POST, saveconfig);

  // Favicon Handle
  server.on("/favicon.ico", HTTP_GET, faviconhandle);

  server.on("/list", HTTP_GET, handleFileList);

  Serial.println("[INFO]: Server handles loaded.");

  // Draw background
  tft.fillScreen(generalconfig.backgroundColour);

  // Draw keypad
  Serial.println("[INFO]: Drawing keypad");
  drawKeypad();

  //------------------BLE Initialization ------------------------------------------------------------------------

  Serial.println("[INFO]: Starting BLE");
  bleKeyboard.begin();

#ifdef touchInterruptPin 
  if(wificonfig.sleepenable){
    pinMode(touchInterruptPin,INPUT_PULLUP);
    Interval = wificonfig.sleeptimer * 60000;
    Serial.println("[INFO]: Sleep enabled.");
    Serial.print("[INFO]: Sleep timer = ");
    Serial.print(wificonfig.sleeptimer);
    Serial.println(" minutes");
    islatched[28] = 1;
  }
#endif

}

//--------------------- LOOP ---------------------------------------------------------------------

void loop(void)
{

  if (pageNum == 7){

    // If the pageNum is set to 7, do no draw anything on screen or check for touch
    // and start handeling incomming web requests.
    server.handleClient();
    
  }else{

  // Check if sleep is enabled and if our timer has ended.

  #ifdef touchInterruptPin 
  if(wificonfig.sleepenable){
    if(millis() > previousMillis + Interval){
      
      // The timer has ended and we are going to sleep  .
      tft.fillScreen(TFT_BLACK);
      Serial.println("[INFO]: Going to sleep.");
      #ifdef speakerPin
      ledcAttachPin(speakerPin, 0);
      ledcWriteTone(1, 1200);
      delay(150);
      ledcDetachPin(speakerPin);
      ledcWrite(1, 0);
    
      ledcAttachPin(speakerPin, 0);
      ledcWriteTone(1, 800);
      delay(150);
      ledcDetachPin(speakerPin);
      ledcWrite(1, 0);
    
      ledcAttachPin(speakerPin, 0);
      ledcWriteTone(1, 600);
      delay(150);
      ledcDetachPin(speakerPin);
      ledcWrite(1, 0);
      #endif
  
      esp_sleep_enable_ext0_wakeup(touchInterruptPin,0);
      esp_deep_sleep_start();     
    }
  }
  #endif

     
  // Touch coordinates are stored here
  uint16_t t_x = 0, t_y = 0;

  //At the beginning of a new loop, make sure we do not use last loop's touch.
  boolean pressed = false;

    #ifdef USECAPTOUCH
    if (ts.touched()){   
      
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
        key[b].drawButton();
        // Call the drawLogo function
        drawlogo(b, colArray[b], rowArray[b]);
      }

      if (key[b].justPressed())
      {
        key[b].drawButton(true); // Draw inverted button space (white)
        
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
      
      else if(pageNum == 1) // Menu 1
      {
          if (b == 0)  // Button 0
          {
              bleKeyboardAction(menu1.button0.actions.action0, menu1.button0.actions.value0, menu1.button0.actions.symbol0);
              bleKeyboardAction(menu1.button0.actions.action1, menu1.button0.actions.value1, menu1.button0.actions.symbol1);
              bleKeyboardAction(menu1.button0.actions.action2, menu1.button0.actions.value2, menu1.button0.actions.symbol2);
              bleKeyboard.releaseAll();
              if(menu1.button0.latch){
                if(islatched[0]){
                  islatched[0] = 0;
                  drawlatched(b, 0, 0, false);
                }else{
                  islatched[0] = 1;
                  drawlatched(b, 0, 0, true);
                }
              }
          }
          else if (b == 1) // Button 1
          {
              bleKeyboardAction(menu1.button1.actions.action0, menu1.button1.actions.value0, menu1.button1.actions.symbol0);
              bleKeyboardAction(menu1.button1.actions.action1, menu1.button1.actions.value1, menu1.button1.actions.symbol1);
              bleKeyboardAction(menu1.button1.actions.action2, menu1.button1.actions.value2, menu1.button1.actions.symbol2);
              bleKeyboard.releaseAll();
              if(menu1.button1.latch){
                if(islatched[1]){
                  islatched[1] = 0;
                  drawlatched(b, 1, 0, false);
                }else{
                  islatched[1] = 1;
                  drawlatched(b, 1, 0, true);
                }
              }
          }
          else if (b == 2) // Button 2
          {
              bleKeyboardAction(menu1.button2.actions.action0, menu1.button2.actions.value0, menu1.button2.actions.symbol0);
              bleKeyboardAction(menu1.button2.actions.action1, menu1.button2.actions.value1, menu1.button2.actions.symbol1);
              bleKeyboardAction(menu1.button2.actions.action2, menu1.button2.actions.value2, menu1.button2.actions.symbol2);
              bleKeyboard.releaseAll();
              if(menu1.button2.latch){
                if(islatched[2]){
                  islatched[2] = 0;
                  drawlatched(b, 2, 0, false);
                }else{
                  islatched[2] = 1;
                  drawlatched(b, 2, 0, true);
                }
              }
          }
          else if (b == 3) // Button 3
          {
              bleKeyboardAction(menu1.button3.actions.action0, menu1.button3.actions.value0, menu1.button3.actions.symbol0);
              bleKeyboardAction(menu1.button3.actions.action1, menu1.button3.actions.value1, menu1.button3.actions.symbol1);
              bleKeyboardAction(menu1.button3.actions.action2, menu1.button3.actions.value2, menu1.button3.actions.symbol2);
              bleKeyboard.releaseAll();
              if(menu1.button3.latch){
                if(islatched[3]){
                  islatched[3] = 0;
                  drawlatched(b, 0, 1, false);
                }else{
                  islatched[3] = 1;
                  drawlatched(b, 0, 1, true);
                }
              }
          }
          else if (b == 4) // Button 4
          {
              bleKeyboardAction(menu1.button4.actions.action0, menu1.button4.actions.value0, menu1.button4.actions.symbol0);
              bleKeyboardAction(menu1.button4.actions.action1, menu1.button4.actions.value1, menu1.button4.actions.symbol1);
              bleKeyboardAction(menu1.button4.actions.action2, menu1.button4.actions.value2, menu1.button4.actions.symbol2);
              bleKeyboard.releaseAll();
              if(menu1.button4.latch){
                if(islatched[4]){
                  islatched[4] = 0;
                  drawlatched(b, 1, 1, false);
                }else{
                  islatched[4] = 1;
                  drawlatched(b, 1, 1, true);
                }
              }
          }
          else if (b == 5) // Button 5 / Back home
          {
              pageNum = 0;
              drawKeypad();
          }
      }
      
      else if(pageNum == 2) // Menu 2
      {
          if (b == 0)  // Button 0
          {
              bleKeyboardAction(menu2.button0.actions.action0, menu2.button0.actions.value0, menu2.button0.actions.symbol0);
              bleKeyboardAction(menu2.button0.actions.action1, menu2.button0.actions.value1, menu2.button0.actions.symbol1);
              bleKeyboardAction(menu2.button0.actions.action2, menu2.button0.actions.value2, menu2.button0.actions.symbol2);
              bleKeyboard.releaseAll();
              if(menu2.button0.latch){
                if(islatched[5]){
                  islatched[5] = 0;
                  drawlatched(b, 0, 0, false);
                }else{
                  islatched[5] = 1;
                  drawlatched(b, 0, 0, true);
                }
              }
          }
          else if (b == 1) // Button 1
          {
              bleKeyboardAction(menu2.button1.actions.action0, menu2.button1.actions.value0, menu2.button1.actions.symbol0);
              bleKeyboardAction(menu2.button1.actions.action1, menu2.button1.actions.value1, menu2.button1.actions.symbol1);
              bleKeyboardAction(menu2.button1.actions.action2, menu2.button1.actions.value2, menu2.button1.actions.symbol2);
              bleKeyboard.releaseAll();
              if(menu2.button1.latch){
                if(islatched[6]){
                  islatched[6] = 0;
                  drawlatched(b, 1, 0, false);
                }else{
                  islatched[6] = 1;
                  drawlatched(b, 1, 0, true);
                }
              }
          }
          else if (b == 2) // Button 2
          {
              bleKeyboardAction(menu2.button2.actions.action0, menu2.button2.actions.value0, menu2.button2.actions.symbol0);
              bleKeyboardAction(menu2.button2.actions.action1, menu2.button2.actions.value1, menu2.button2.actions.symbol1);
              bleKeyboardAction(menu2.button2.actions.action2, menu2.button2.actions.value2, menu2.button2.actions.symbol2);
              bleKeyboard.releaseAll();
              if(menu2.button2.latch){
                if(islatched[7]){
                  islatched[7] = 0;
                  drawlatched(b, 2, 0, false);
                }else{
                  islatched[7] = 1;
                  drawlatched(b, 2, 0, true);
                }
              }
          }
          else if (b == 3) // Button 3
          {   
              bleKeyboardAction(menu2.button3.actions.action0, menu2.button3.actions.value0, menu2.button3.actions.symbol0);
              bleKeyboardAction(menu2.button3.actions.action1, menu2.button3.actions.value1, menu2.button3.actions.symbol1);
              bleKeyboardAction(menu2.button3.actions.action2, menu2.button3.actions.value2, menu2.button3.actions.symbol2);
              bleKeyboard.releaseAll();
              if(menu2.button3.latch){
                if(islatched[8]){
                  islatched[8] = 0;
                  drawlatched(b, 0, 1, false);
                }else{
                  islatched[8] = 1;
                  drawlatched(b, 0, 1, true);
                }
              }
          }
          else if (b == 4) // Button 4
          {
              bleKeyboardAction(menu2.button4.actions.action0, menu2.button4.actions.value0, menu2.button4.actions.symbol0);
              bleKeyboardAction(menu2.button4.actions.action1, menu2.button4.actions.value1, menu2.button4.actions.symbol1);
              bleKeyboardAction(menu2.button4.actions.action2, menu2.button4.actions.value2, menu2.button4.actions.symbol2);
              bleKeyboard.releaseAll();
              if(menu2.button4.latch){
                if(islatched[9]){
                  islatched[9] = 0;
                  drawlatched(b, 1, 1, false);
                }else{
                  islatched[9] = 1;
                  drawlatched(b, 1, 1, true);
                }
              }
          }
          else if (b == 5) // Button 5 / Back home
          {
              pageNum = 0;
              drawKeypad();
          }
      }
      
      else if(pageNum == 3) // Menu 3
      {
          if (b == 0)  // Button 0
          {
              bleKeyboardAction(menu3.button0.actions.action0, menu3.button0.actions.value0, menu3.button0.actions.symbol0);
              bleKeyboardAction(menu3.button0.actions.action1, menu3.button0.actions.value1, menu3.button0.actions.symbol1);
              bleKeyboardAction(menu3.button0.actions.action2, menu3.button0.actions.value2, menu3.button0.actions.symbol2);
              bleKeyboard.releaseAll();
              if(menu3.button0.latch){
                if(islatched[10]){
                  islatched[10] = 0;
                  drawlatched(b, 0, 0, false);
                }else{
                  islatched[10] = 1;
                  drawlatched(b, 0, 0, true);
                }
              }
          }
          else if (b == 1) // Button 1
          {
              bleKeyboardAction(menu3.button1.actions.action0, menu3.button1.actions.value0, menu3.button1.actions.symbol0);
              bleKeyboardAction(menu3.button1.actions.action1, menu3.button1.actions.value1, menu3.button1.actions.symbol1);
              bleKeyboardAction(menu3.button1.actions.action2, menu3.button1.actions.value2, menu3.button1.actions.symbol2);
              bleKeyboard.releaseAll();
              if(menu3.button1.latch){
                if(islatched[11]){
                  islatched[11] = 0;
                  drawlatched(b, 1, 0, false);
                }else{
                  islatched[11] = 1;
                  drawlatched(b, 1, 0, true);
                }
              }
          }
          else if (b == 2) // Button 2
          {
              bleKeyboardAction(menu3.button2.actions.action0, menu3.button2.actions.value0, menu3.button2.actions.symbol0);
              bleKeyboardAction(menu3.button2.actions.action1, menu3.button2.actions.value1, menu3.button2.actions.symbol1);
              bleKeyboardAction(menu3.button2.actions.action2, menu3.button2.actions.value2, menu3.button2.actions.symbol2);
              bleKeyboard.releaseAll();
              if(menu3.button2.latch){
                if(islatched[12]){
                  islatched[12] = 0;
                  drawlatched(b, 2, 0, false);
                }else{
                  islatched[12] = 1;
                  drawlatched(b, 2, 0, true);
                }
              }
          }
          else if (b == 3) // Button 3
          {
              bleKeyboardAction(menu3.button3.actions.action0, menu3.button3.actions.value0, menu3.button3.actions.symbol0);
              bleKeyboardAction(menu3.button3.actions.action1, menu3.button3.actions.value1, menu3.button3.actions.symbol1);
              bleKeyboardAction(menu3.button3.actions.action2, menu3.button3.actions.value2, menu3.button3.actions.symbol2);
              bleKeyboard.releaseAll();
              if(menu3.button3.latch){
                if(islatched[13]){
                  islatched[13] = 0;
                  drawlatched(b, 0, 1, false);
                }else{
                  islatched[13] = 1;
                  drawlatched(b, 0, 1, true);
                }
              }
          }
          else if (b == 4) // Button 4
          {
              bleKeyboardAction(menu3.button4.actions.action0, menu3.button4.actions.value0, menu3.button4.actions.symbol0);
              bleKeyboardAction(menu3.button4.actions.action1, menu3.button4.actions.value1, menu3.button4.actions.symbol1);
              bleKeyboardAction(menu3.button4.actions.action2, menu3.button4.actions.value2, menu3.button4.actions.symbol2);
              bleKeyboard.releaseAll();
              if(menu3.button4.latch){
                if(islatched[14]){
                  islatched[14] = 0;
                  drawlatched(b, 1, 1, false);
                }else{
                  islatched[14] = 1;
                  drawlatched(b, 1, 1, true);
                }
              }
          }
          else if (b == 5) // Button 5 / Back home
          {
              pageNum = 0;
              drawKeypad();
          }
      }
      
      else if(pageNum == 4) // Menu 4
      {
          if (b == 0)  // Button 0
          {
              bleKeyboardAction(menu4.button0.actions.action0, menu4.button0.actions.value0, menu4.button0.actions.symbol0);
              bleKeyboardAction(menu4.button0.actions.action1, menu4.button0.actions.value1, menu4.button0.actions.symbol1);
              bleKeyboardAction(menu4.button0.actions.action2, menu4.button0.actions.value2, menu4.button0.actions.symbol2);
              bleKeyboard.releaseAll();
              if(menu4.button0.latch){
                if(islatched[15]){
                  islatched[15] = 0;
                  drawlatched(b, 0, 0, false);
                }else{
                  islatched[15] = 1;
                  drawlatched(b, 0, 0, true);
                }
              }
          }
          else if (b == 1) // Button 1
          {
              bleKeyboardAction(menu4.button1.actions.action0, menu4.button1.actions.value0, menu4.button1.actions.symbol0);
              bleKeyboardAction(menu4.button1.actions.action1, menu4.button1.actions.value1, menu4.button1.actions.symbol1);
              bleKeyboardAction(menu4.button1.actions.action2, menu4.button1.actions.value2, menu4.button1.actions.symbol2);
              bleKeyboard.releaseAll();
              if(menu4.button1.latch){
                if(islatched[16]){
                  islatched[16] = 0;
                  drawlatched(b, 1, 0, false);
                }else{
                  islatched[16] = 1;
                  drawlatched(b, 1, 0, true);
                }
              }
          }
          else if (b == 2) // Button 2
          {
              bleKeyboardAction(menu4.button2.actions.action0, menu4.button2.actions.value0, menu4.button2.actions.symbol0);
              bleKeyboardAction(menu4.button2.actions.action1, menu4.button2.actions.value1, menu4.button2.actions.symbol1);
              bleKeyboardAction(menu4.button2.actions.action2, menu4.button2.actions.value2, menu4.button2.actions.symbol2);
              bleKeyboard.releaseAll();
              if(menu4.button2.latch){
                if(islatched[17]){
                  islatched[17] = 0;
                  drawlatched(b, 2, 0, false);
                }else{
                  islatched[17] = 1;
                  drawlatched(b, 2, 0, true);
                }
              }
          }
          else if (b == 3) // Button 3
          {
              bleKeyboardAction(menu4.button3.actions.action0, menu4.button3.actions.value0, menu4.button3.actions.symbol0);
              bleKeyboardAction(menu4.button3.actions.action1, menu4.button3.actions.value1, menu4.button3.actions.symbol1);
              bleKeyboardAction(menu4.button3.actions.action2, menu4.button3.actions.value2, menu4.button3.actions.symbol2);
              bleKeyboard.releaseAll();
              if(menu4.button3.latch){
                if(islatched[18]){
                  islatched[18] = 0;
                  drawlatched(b, 0, 1, false);
                }else{
                  islatched[18] = 1;
                  drawlatched(b, 0, 1, true);
                }
              }
          }
          else if (b == 4) // Button 4
          {
              bleKeyboardAction(menu4.button4.actions.action0, menu4.button4.actions.value0, menu4.button4.actions.symbol0);
              bleKeyboardAction(menu4.button4.actions.action1, menu4.button4.actions.value1, menu4.button4.actions.symbol1);
              bleKeyboardAction(menu4.button4.actions.action2, menu4.button4.actions.value2, menu4.button4.actions.symbol2);
              bleKeyboard.releaseAll();
              if(menu4.button4.latch){
                if(islatched[19]){
                  islatched[19] = 0;
                  drawlatched(b, 1, 1, false);
                }else{
                  islatched[19] = 1;
                  drawlatched(b, 1, 1, true);
                }
              }
          }
          else if (b == 5) // Button 5 / Back home
          {
              pageNum = 0;
              drawKeypad();
          }
      }
      
      else if(pageNum == 5) // Menu 5
      {
          if (b == 0)  // Button 0
          {
              bleKeyboardAction(menu5.button0.actions.action0, menu5.button0.actions.value0, menu5.button0.actions.symbol0);
              bleKeyboardAction(menu5.button0.actions.action1, menu5.button0.actions.value1, menu5.button0.actions.symbol1);
              bleKeyboardAction(menu5.button0.actions.action2, menu5.button0.actions.value2, menu5.button0.actions.symbol2);
              bleKeyboard.releaseAll();
              if(menu5.button0.latch){
                if(islatched[20]){
                  islatched[20] = 0;
                  drawlatched(b, 0, 0, false);
                }else{
                  islatched[20] = 1;
                  drawlatched(b, 0, 0, true);
                }
              }
          }
          else if (b == 1) // Button 1
          {
              bleKeyboardAction(menu5.button1.actions.action0, menu5.button1.actions.value0, menu5.button1.actions.symbol0);
              bleKeyboardAction(menu5.button1.actions.action1, menu5.button1.actions.value1, menu5.button1.actions.symbol1);
              bleKeyboardAction(menu5.button1.actions.action2, menu5.button1.actions.value2, menu5.button1.actions.symbol2);
              bleKeyboard.releaseAll();
              if(menu5.button1.latch){
                if(islatched[21]){
                  islatched[21] = 0;
                  drawlatched(b, 1, 0, false);
                }else{
                  islatched[21] = 1;
                  drawlatched(b, 1, 0, true);
                }
              }
          }
          else if (b == 2) // Button 2
          {
              bleKeyboardAction(menu5.button2.actions.action0, menu5.button2.actions.value0, menu5.button2.actions.symbol0);
              bleKeyboardAction(menu5.button2.actions.action1, menu5.button2.actions.value1, menu5.button2.actions.symbol1);
              bleKeyboardAction(menu5.button2.actions.action2, menu5.button2.actions.value2, menu5.button2.actions.symbol2);
              bleKeyboard.releaseAll();
              if(menu5.button2.latch){
                if(islatched[22]){
                  islatched[22] = 0;
                  drawlatched(b, 2, 0, false);
                }else{
                  islatched[22] = 1;
                  drawlatched(b, 2, 0, true);
                }
              }
          }
          else if (b == 3) // Button 3
          {
              bleKeyboardAction(menu5.button3.actions.action0, menu5.button3.actions.value0, menu5.button3.actions.symbol0);
              bleKeyboardAction(menu5.button3.actions.action1, menu5.button3.actions.value1, menu5.button3.actions.symbol1);
              bleKeyboardAction(menu5.button3.actions.action2, menu5.button3.actions.value2, menu5.button3.actions.symbol2);
              bleKeyboard.releaseAll();
              if(menu5.button3.latch){
                if(islatched[23]){
                  islatched[23] = 0;
                  drawlatched(b, 0, 1, false);
                }else{
                  islatched[23] = 1;
                  drawlatched(b, 0, 1, true);
                }
              }
          }
          else if (b == 4) // Button 4
          {
              bleKeyboardAction(menu5.button4.actions.action0, menu5.button4.actions.value0, menu5.button4.actions.symbol0);
              bleKeyboardAction(menu5.button4.actions.action1, menu5.button4.actions.value1, menu5.button4.actions.symbol1);
              bleKeyboardAction(menu5.button4.actions.action2, menu5.button4.actions.value2, menu5.button4.actions.symbol2);
              bleKeyboard.releaseAll();
              if(menu5.button4.latch){
                if(islatched[24]){
                  islatched[24] = 0;
                  drawlatched(b, 1, 1, false);
                }else{
                  islatched[24] = 1;
                  drawlatched(b, 1, 1, true);
                }
              }
          }
          else if (b == 5) // Button 5 / Back home
          {
              pageNum = 0;
              drawKeypad();
          }
      }
      
      else if(pageNum == 6) // Settings page
      {
          if (b == 0)  // Button 0
          {
              bleKeyboardAction(9, 1, "0");
          }
          else if (b == 1) // Button 1
          {
              bleKeyboardAction(9, 2, "0");
          }
          else if (b == 2) // Button 2
          {
              bleKeyboardAction(9, 3, "0");
          }
          else if (b == 3) // Button 3
          {
              bleKeyboardAction(9, 4, "0");
              if(islatched[28]){
                  islatched[28] = 0;
                  drawlatched(b, 0, 1, false);
                }else{
                  islatched[28] = 1;
                  drawlatched(b, 0, 1, true);
                }
          }
          else if (b == 4) // Button 4
          {
              // No function yet
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

//------------------------------------ Drawing Buttons --------------------------------------------------------

void drawKeypad()
{
  // Draw the home screen button outlines and fill them with colours
  if (pageNum == 0)
  {
    for (uint8_t row = 0; row < 2; row++)
    {
      for (uint8_t col = 0; col < 3; col++)
      {

        uint8_t b = col + row * 3;
        tft.setFreeFont(LABEL_FONT);
        key[b].initButton(&tft, KEY_X + col * (KEY_W + KEY_SPACING_X),
                          KEY_Y + row * (KEY_H + KEY_SPACING_Y), // x, y, w, h, outline, fill, text
                          KEY_W, KEY_H, TFT_WHITE, generalconfig.menuButtonColour, 0xFFFF,
                          "", KEY_TEXTSIZE);
        key[b].drawButton();
        drawlogo(b, col, row); // After drawing the button outline we call this to draw a logo.
      }
    }
  }
  else if (pageNum == 7)
  {
    // Config mode...
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0, 0);
    tft.setTextFont(2);
    if(SCREEN_WIDTH == 480){
      tft.setTextSize(2);
    }else{
      tft.setTextSize(1);
    }
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.println("Now in config mode. To configure:");
    tft.println("http://freetouchdeck.local");
    tft.print("Your IP is: ");
    tft.println(WiFi.localIP());
  }
  else
  {
    // Draw the button outlines and fill them with colours
    for (uint8_t row = 0; row < 2; row++)
    {
      for (uint8_t col = 0; col < 3; col++)
      {

        uint8_t b = col + row * 3;

        if (row == 1 && col == 2)
        {
          // If it is the last button ("back home button") create it with the menuButtonColour
          tft.setFreeFont(LABEL_FONT);
          key[b].initButton(&tft, KEY_X + col * (KEY_W + KEY_SPACING_X),
                            KEY_Y + row * (KEY_H + KEY_SPACING_Y), // x, y, w, h, outline, fill, text
                            KEY_W, KEY_H, TFT_WHITE, generalconfig.menuButtonColour, TFT_WHITE,
                            "", KEY_TEXTSIZE);
          key[b].drawButton();
          drawlogo(b, col, row);
        }
        else
        {
          // Otherwise use functionButtonColour

          int index;
          
          if(pageNum == 2){
            index = b + 5;
          }else if (pageNum == 3){
            index = b + 10;
          }else if (pageNum == 4){
            index = b + 15; 
          }else if (pageNum == 5){
            index = b + 20;
          }else if (pageNum == 6){
            index = b + 25;        
          }else{
            index = b;
          }
          
          if(islatched[index]){
            drawlatched(b, col, row, true);
          }else{
          tft.setFreeFont(LABEL_FONT);
          key[b].initButton(&tft, KEY_X + col * (KEY_W + KEY_SPACING_X),
                            KEY_Y + row * (KEY_H + KEY_SPACING_Y), // x, y, w, h, outline, fill, text
                            KEY_W, KEY_H, TFT_WHITE, generalconfig.functionButtonColour, TFT_WHITE,
                            "", KEY_TEXTSIZE);
          key[b].drawButton();
  
          }
          drawlogo(b, col, row);
        }
      }
    }
  }
}

// -------------------- Draw Latched Button ------------------------

void drawlatched(int b, int col, int row, bool latched)
{
  if(latched){
    tft.setFreeFont(LABEL_FONT);
    key[b].initButton(&tft, KEY_X + col * (KEY_W + KEY_SPACING_X),
                      KEY_Y + row * (KEY_H + KEY_SPACING_Y), // x, y, w, h, outline, fill, text
                      KEY_W, KEY_H, TFT_WHITE, generalconfig.latchedColour, TFT_WHITE,
                      "", KEY_TEXTSIZE);
    key[b].drawButton();
  }else{
    tft.setFreeFont(LABEL_FONT);
    key[b].initButton(&tft, KEY_X + col * (KEY_W + KEY_SPACING_X),
                      KEY_Y + row * (KEY_H + KEY_SPACING_Y), // x, y, w, h, outline, fill, text
                      KEY_W, KEY_H, TFT_WHITE, generalconfig.functionButtonColour, TFT_WHITE,
                      "", KEY_TEXTSIZE);
    key[b].drawButton();
  }
}

//----------------------------------- Drawing Logo's ----------------------------------------------------------------

void drawlogo(int logonumber, int col, int row)
{

  if (pageNum == 0)
  {
    //Draw Home screen logo's
    if (logonumber == 0)
    {
      drawBmpTransparent(screen0.logo0, KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y));
    }
    else if (logonumber == 1)
    {
      drawBmpTransparent(screen0.logo1, KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y));
    }
    else if (logonumber == 2)
    {
      drawBmpTransparent(screen0.logo2, KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y));
    }
    else if (logonumber == 3)
    {
      drawBmpTransparent(screen0.logo3, KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y));
    }
    else if (logonumber == 4)
    {
      drawBmpTransparent(screen0.logo4, KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y));
    }
    else if (logonumber == 5)
    {
      drawBmpTransparent(screen0.logo5, KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y));
    }
  }
  else if (pageNum == 1)
  {
    // Menu 1
    if (logonumber == 0)
    {
      drawBmpTransparent(screen1.logo0, KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y));
    }
    else if (logonumber == 1)
    {
      drawBmpTransparent(screen1.logo1, KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y));
    }
    else if (logonumber == 2)
    {
      drawBmpTransparent(screen1.logo2, KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y));
    }
    else if (logonumber == 3)
    {
      drawBmpTransparent(screen1.logo3, KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y));
    }
    else if (logonumber == 4)
    {
      drawBmpTransparent(screen1.logo4, KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y));
    }
    else if (logonumber == 5)
    {
      drawBmpTransparent(generallogo.homebutton, KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y));
    }
  }
  else if (pageNum == 2)
  {
    // Menu 2
    if (logonumber == 0)
    {
      drawBmpTransparent(screen2.logo0, KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y));
    }
    else if (logonumber == 1)
    {
      drawBmpTransparent(screen2.logo1, KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y));
    }
    else if (logonumber == 2)
    {
      drawBmpTransparent(screen2.logo2, KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y));
    }
    else if (logonumber == 3)
    {
      drawBmpTransparent(screen2.logo3, KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y));
    }
    else if (logonumber == 4)
    {
      drawBmpTransparent(screen2.logo4, KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y));
    }
    else if (logonumber == 5)
    {
      drawBmpTransparent(generallogo.homebutton, KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y));
    }
  }
  else if (pageNum == 3)
  {
    // Menu 3
    if (logonumber == 0)
    {
      drawBmpTransparent(screen3.logo0, KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y));
    }
    else if (logonumber == 1)
    {
      drawBmpTransparent(screen3.logo1, KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y));
    }
    else if (logonumber == 2)
    {
      drawBmpTransparent(screen3.logo2, KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y));
    }
    else if (logonumber == 3)
    {
      drawBmpTransparent(screen3.logo3, KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y));
    }
    else if (logonumber == 4)
    {
      drawBmpTransparent(screen3.logo4, KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y));
    }
    else if (logonumber == 5)
    {
      drawBmpTransparent(generallogo.homebutton, KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y));
    }
  }
  else if (pageNum == 4)
  {
    // Menu 4
    if (logonumber == 0)
    {
      drawBmpTransparent(screen4.logo0, KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y));
    }
    else if (logonumber == 1)
    {
      drawBmpTransparent(screen4.logo1, KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y));
    }
    else if (logonumber == 2)
    {
      drawBmpTransparent(screen4.logo2, KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y));
    }
    else if (logonumber == 3)
    {
      drawBmpTransparent(screen4.logo3, KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y));
    }
    else if (logonumber == 4)
    {
      drawBmpTransparent(screen4.logo4, KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y));
    }
    else if (logonumber == 5)
    {
      drawBmpTransparent(generallogo.homebutton, KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y));
    }
  }
  else if (pageNum == 5)
  {
    // Menu 5
    if (logonumber == 0)
    {
      drawBmpTransparent(screen5.logo0, KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y));
    }
    else if (logonumber == 1)
    {
      drawBmpTransparent(screen5.logo1, KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y));
    }
    else if (logonumber == 2)
    {
      drawBmpTransparent(screen5.logo2, KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y));
    }
    else if (logonumber == 3)
    {
      drawBmpTransparent(screen5.logo3, KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y));
    }
    else if (logonumber == 4)
    {
      drawBmpTransparent(screen5.logo4, KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y));
    }
    else if (logonumber == 5)
    {
      drawBmpTransparent(generallogo.homebutton, KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y));
    }
  }
  else if (pageNum == 6)
  {
    // pageNum6 contains settings logos
    if (logonumber == 0)
    {
      drawBmpTransparent(generallogo.configurator, KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y));
    }
    else if (logonumber == 1)
    {
      drawBmpTransparent("/logos/brightnessdown.bmp", KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y));
    }
    else if (logonumber == 2)
    {
      drawBmpTransparent("/logos/brightnessup.bmp", KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y));
    }
    else if (logonumber == 3)
    {
      drawBmpTransparent("/logos/sleep.bmp", KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y));
    }
    else if (logonumber == 4)
    {
      drawBmpTransparent(screen6.logo4, KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y));
    }
    else if (logonumber == 5)
    {
      drawBmpTransparent(generallogo.homebutton, KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y));
    }
  }
}

void bleKeyboardAction(int action, int value, char *symbol)
{

  Serial.println("[INFO]: BLE Keyboard action received");
  switch (action)
  {
  case 0:
    // No Action
    break;
  case 1: // Delay
    delay(value);
    break;
  case 2: // Send TAB ARROW etc
    switch (value)
    {
    case 1:
      bleKeyboard.write(KEY_UP_ARROW);
      break;
    case 2:
      bleKeyboard.write(KEY_DOWN_ARROW);
      break;
    case 3:
      bleKeyboard.write(KEY_LEFT_ARROW);
      break;
    case 4:
      bleKeyboard.write(KEY_RIGHT_ARROW);
      break;
    case 5:
      bleKeyboard.write(KEY_BACKSPACE);
      break;
    case 6:
      bleKeyboard.write(KEY_TAB);
      break;
    case 7:
      bleKeyboard.write(KEY_RETURN);
      break;
    case 8:
      bleKeyboard.write(KEY_PAGE_UP);
      break;
    case 9:
      bleKeyboard.write(KEY_PAGE_DOWN);
      break;
    default:
      //if nothing matches do nothing
      break;
    }
    break;
  case 3: // Send Media Key
    switch (value)
    {
    case 1:
      bleKeyboard.write(KEY_MEDIA_MUTE);
      break;
    case 2:
      bleKeyboard.write(KEY_MEDIA_VOLUME_DOWN);
      break;
    case 3:
      bleKeyboard.write(KEY_MEDIA_VOLUME_UP);
      break;
    case 4:
      bleKeyboard.write(KEY_MEDIA_PLAY_PAUSE);
      break;
    case 5:
      bleKeyboard.write(KEY_MEDIA_STOP);
      break;
    case 6:
      bleKeyboard.write(KEY_MEDIA_NEXT_TRACK);
      break;
    case 7:
      bleKeyboard.write(KEY_MEDIA_PREVIOUS_TRACK);
      break;
    default:
      //if nothing matches do nothing
      break;
    }
    break;
  case 4: // Send Character
    bleKeyboard.print(symbol);
    break;
  case 5: // Option Keys
    switch (value)
    {
    case 1:
      bleKeyboard.press(KEY_LEFT_CTRL);
      break;
    case 2:
      bleKeyboard.press(KEY_LEFT_SHIFT);
      break;
    case 3:
      bleKeyboard.press(KEY_LEFT_ALT);
      break;
    case 4:
      bleKeyboard.press(KEY_LEFT_GUI);
      break;
    case 5:
      bleKeyboard.press(KEY_RIGHT_CTRL);
      break;
    case 6:
      bleKeyboard.press(KEY_RIGHT_SHIFT);
      break;
    case 7:
      bleKeyboard.press(KEY_RIGHT_ALT);
      break;
    case 8:
      bleKeyboard.press(KEY_RIGHT_GUI);
      break;
    case 9:
      bleKeyboard.releaseAll();
      break;
    default:
      //if nothing matches do nothing
      break;
    }
    break;
  case 6: // Function Keys
    switch (value)
    {
    case 1:
      bleKeyboard.press(KEY_F1);
      break;
    case 2:
      bleKeyboard.press(KEY_F2);
      break;
    case 3:
      bleKeyboard.press(KEY_F3);
      break;
    case 4:
      bleKeyboard.press(KEY_F4);
      break;
    case 5:
      bleKeyboard.press(KEY_F5);
      break;
    case 6:
      bleKeyboard.press(KEY_F6);
      break;
    case 7:
      bleKeyboard.press(KEY_F7);
      break;
    case 8:
      bleKeyboard.press(KEY_F8);
      break;
    case 9:
      bleKeyboard.press(KEY_F9);
      break;
    case 10:
      bleKeyboard.press(KEY_F10);
      break;
    case 11:
      bleKeyboard.press(KEY_F11);
      break;
    case 12:
      bleKeyboard.press(KEY_F12);
      break;
    default:
      //if nothing matches do nothing
      break;
    }
    break;
  case 7: // Send Number
    bleKeyboard.print(value);
    break;
  case 8: // Send Special Character
    bleKeyboard.print(symbol);
    break;
  case 9: // Special functions
    switch (value)
    {
    case 1:        // Enter config mode
      pageNum = 7; // By setting pageNum to 7
      configmode();
      drawKeypad(); // and calling drawKeypad() a new keypad is drawn with pageNum 7
      break;
    case 2:        // Display Brightness Down
      if(ledBrightness > 25){
      ledBrightness = ledBrightness - 25;
      ledcWrite(0, ledBrightness);
      }
      break;
    case 3:        // Display Brightness Up
      if(ledBrightness < 230){
      ledBrightness = ledBrightness + 25;
      ledcWrite(0, ledBrightness);
      }
      break;
    case 4:        // Display Brightness Up
      if(wificonfig.sleepenable){
        wificonfig.sleepenable = false;
        Serial.println("[INFO]: Sleep disabled.");
      }else{
        wificonfig.sleepenable = true;
        Serial.println("[INFO]: Sleep enabled.");
      }
      break;
      
    }
    break;
  default:
    //If nothing matches do nothing
    break;
  }
}

// ------------------------------------ Load JSON File --------------------------------------------------

bool loadMainConfig()
{
  if (!FILESYSTEM.exists("/config/wificonfig.json"))
  {
    Serial.println("[WARNING]: Config file not found!");
    return false;
  }
  File configfile = FILESYSTEM.open("/config/wificonfig.json");

  DynamicJsonDocument doc(256);

  DeserializationError error = deserializeJson(doc, configfile);

  strlcpy(wificonfig.ssid, doc["ssid"] | "FAILED", sizeof(wificonfig.ssid));
  strlcpy(wificonfig.password, doc["password"] | "FAILED", sizeof(wificonfig.password));
  strlcpy(wificonfig.hostname, doc["wifihostname"] | "freetouchdeck", sizeof(wificonfig.hostname));  
  wificonfig.sleepenable = doc["sleepenable"] | false;
  wificonfig.sleeptimer = doc["sleeptimer"] | 15;

  configfile.close();

  return true;
}

void loadConfig(String value)
{

  if (value == "colors")
  {
    File configfile = FILESYSTEM.open("/config/colors.json", "r");

    DynamicJsonDocument doc(256);

    DeserializationError error = deserializeJson(doc, configfile);

    // Parsing colors
    const char *menubuttoncolor = doc["menubuttoncolor"];         // Get the colour for the menu and back home buttons.
    const char *functionbuttoncolor = doc["functionbuttoncolor"]; // Get the colour for the function buttons.
    const char *latchcolor = doc["latchcolor"];                     // Get the colour to use when latching.
    const char *bgcolor = doc["background"];                      // Get the colour for the background.

    char menubuttoncolorchar[64];
    strcpy(menubuttoncolorchar, menubuttoncolor);
    unsigned long rgb888menubuttoncolor = convertHTMLtoRGB888(menubuttoncolorchar);
    generalconfig.menuButtonColour = convertRGB888ToRGB565(rgb888menubuttoncolor);

    char functionbuttoncolorchar[64];
    strcpy(functionbuttoncolorchar, functionbuttoncolor);
    unsigned long rgb888functionbuttoncolor = convertHTMLtoRGB888(functionbuttoncolorchar);
    generalconfig.functionButtonColour = convertRGB888ToRGB565(rgb888functionbuttoncolor);

    char latchcolorchar[64];
    strcpy(latchcolorchar, latchcolor);
    unsigned long rgb888latchcolor = convertHTMLtoRGB888(latchcolorchar);
    generalconfig.latchedColour = convertRGB888ToRGB565(rgb888latchcolor);

    char backgroundcolorchar[64];
    strcpy(backgroundcolorchar, bgcolor);
    unsigned long rgb888backgroundcolor = convertHTMLtoRGB888(backgroundcolorchar);
    generalconfig.backgroundColour = convertRGB888ToRGB565(rgb888backgroundcolor);

    configfile.close();
  }
  else if (value == "homescreen")
  {
    File configfile = FILESYSTEM.open("/config/homescreen.json", "r");

    DynamicJsonDocument doc(256);

    DeserializationError error = deserializeJson(doc, configfile);

    const char *logo00 = doc["logo0"];
    const char *logo01 = doc["logo1"];
    const char *logo02 = doc["logo2"];
    const char *logo03 = doc["logo3"];
    const char *logo04 = doc["logo4"];
    const char *logo05 = doc["logo5"]; // Only screen 0 has 6 buttons

    strcpy(templogopath, logopath);
    strcat(templogopath, logo00);
    strcpy(screen0.logo0, templogopath);

    strcpy(templogopath, logopath);
    strcat(templogopath, logo01);
    strcpy(screen0.logo1, templogopath);

    strcpy(templogopath, logopath);
    strcat(templogopath, logo02);
    strcpy(screen0.logo2, templogopath);

    strcpy(templogopath, logopath);
    strcat(templogopath, logo03);
    strcpy(screen0.logo3, templogopath);

    strcpy(templogopath, logopath);
    strcat(templogopath, logo04);
    strcpy(screen0.logo4, templogopath);

    strcpy(templogopath, logopath);
    strcat(templogopath, logo05);
    strcpy(screen0.logo5, templogopath);

    configfile.close();

    // --------------------- Loading menu 1 ----------------------
  }
  else if (value == "menu1")
  {
    File configfile = FILESYSTEM.open("/config/menu1.json", "r");

    DynamicJsonDocument doc(1200);

    DeserializationError error = deserializeJson(doc, configfile);

    const char *logo10 = doc["logo0"];
    const char *logo11 = doc["logo1"];
    const char *logo12 = doc["logo2"];
    const char *logo13 = doc["logo3"];
    const char *logo14 = doc["logo4"];

    menu1.button0.latch = doc["button0"]["latch"];

    JsonArray button0_actionarray = doc["button0"]["actionarray"];

    int button0_actionarray_0 = button0_actionarray[0];
    int button0_actionarray_1 = button0_actionarray[1];
    int button0_actionarray_2 = button0_actionarray[2];

    JsonArray button0_valuearray = doc["button0"]["valuearray"];

    if (button0_actionarray_0 == 4 || button0_actionarray_0 == 8)
    {
      const char *button0_symbolarray_0 = button0_valuearray[0];
      strcpy(menu1.button0.actions.symbol0, button0_symbolarray_0);
    }
    else
    {
      int button0_valuearray_0 = button0_valuearray[0];
      menu1.button0.actions.value0 = button0_valuearray_0;
    }

    if (button0_actionarray_1 == 4 || button0_actionarray_1 == 8)
    {
      const char *button0_symbolarray_1 = button0_valuearray[1];
      strcpy(menu1.button0.actions.symbol1, button0_symbolarray_1);
    }
    else
    {
      int button0_valuearray_1 = button0_valuearray[1];
      menu1.button0.actions.value1 = button0_valuearray_1;
    }

    if (button0_actionarray_2 == 4 || button0_actionarray_2 == 8)
    {
      const char *button0_symbolarray_2 = button0_valuearray[2];
      strcpy(menu1.button0.actions.symbol2, button0_symbolarray_2);
    }
    else
    {
      int button0_valuearray_2 = button0_valuearray[2];
      menu1.button0.actions.value2 = button0_valuearray_2;
    }

    menu1.button0.actions.action0 = button0_actionarray_0;
    menu1.button0.actions.action1 = button0_actionarray_1;
    menu1.button0.actions.action2 = button0_actionarray_2;

    menu1.button1.latch = doc["button1"]["latch"];
    
    JsonArray button1_actionarray = doc["button1"]["actionarray"];
    int button1_actionarray_0 = button1_actionarray[0];
    int button1_actionarray_1 = button1_actionarray[1];
    int button1_actionarray_2 = button1_actionarray[2];

    JsonArray button1_valuearray = doc["button1"]["valuearray"];

    if (button1_actionarray_0 == 4 || button1_actionarray_0 == 8)
    {
      const char *button1_symbolarray_0 = button1_valuearray[0];
      strcpy(menu1.button1.actions.symbol0, button1_symbolarray_0);
    }
    else
    {
      int button1_valuearray_0 = button1_valuearray[0];
      menu1.button1.actions.value0 = button1_valuearray_0;
    }

    if (button1_actionarray_1 == 4 || button1_actionarray_1 == 8)
    {
      const char *button1_symbolarray_1 = button1_valuearray[1];
      strcpy(menu1.button1.actions.symbol1, button1_symbolarray_1);
    }
    else
    {
      int button1_valuearray_1 = button1_valuearray[1];
      menu1.button1.actions.value1 = button1_valuearray_1;
    }

    if (button1_actionarray_2 == 4 || button1_actionarray_2 == 8)
    {
      const char *button1_symbolarray_2 = button1_valuearray[2];
      strcpy(menu1.button1.actions.symbol2, button1_symbolarray_2);
    }
    else
    {
      int button1_valuearray_2 = button1_valuearray[2];
      menu1.button1.actions.value2 = button1_valuearray_2;
    }

    menu1.button1.actions.action0 = button1_actionarray_0;
    menu1.button1.actions.action1 = button1_actionarray_1;
    menu1.button1.actions.action2 = button1_actionarray_2;

    menu1.button2.latch = doc["button2"]["latch"];

    JsonArray button2_actionarray = doc["button2"]["actionarray"];
    int button2_actionarray_0 = button2_actionarray[0];
    int button2_actionarray_1 = button2_actionarray[1];
    int button2_actionarray_2 = button2_actionarray[2];

    JsonArray button2_valuearray = doc["button2"]["valuearray"];

    if (button2_actionarray_0 == 4 || button2_actionarray_0 == 8)
    {
      const char *button2_symbolarray_0 = button2_valuearray[0];
      strcpy(menu1.button2.actions.symbol0, button2_symbolarray_0);
    }
    else
    {
      int button2_valuearray_0 = button2_valuearray[0];
      menu1.button2.actions.value0 = button2_valuearray_0;
    }

    if (button2_actionarray_1 == 4 || button2_actionarray_1 == 8)
    {
      const char *button2_symbolarray_1 = button2_valuearray[1];
      strcpy(menu1.button2.actions.symbol1, button2_symbolarray_1);
    }
    else
    {
      int button2_valuearray_1 = button2_valuearray[1];
      menu1.button2.actions.value1 = button2_valuearray_1;
    }

    if (button2_actionarray_2 == 4 || button2_actionarray_2 == 8)
    {
      const char *button2_symbolarray_2 = button2_valuearray[2];
      strcpy(menu1.button2.actions.symbol2, button2_symbolarray_2);
    }
    else
    {
      int button2_valuearray_2 = button2_valuearray[2];
      menu1.button2.actions.value2 = button2_valuearray_2;
    }

    menu1.button2.actions.action0 = button2_actionarray_0;
    menu1.button2.actions.action1 = button2_actionarray_1;
    menu1.button2.actions.action2 = button2_actionarray_2;

    menu1.button3.latch = doc["button3"]["latch"];

    JsonArray button3_actionarray = doc["button3"]["actionarray"];
    int button3_actionarray_0 = button3_actionarray[0]; // 3
    int button3_actionarray_1 = button3_actionarray[1]; // 0
    int button3_actionarray_2 = button3_actionarray[2]; // 0

    JsonArray button3_valuearray = doc["button3"]["valuearray"];

    if (button3_actionarray_0 == 4 || button3_actionarray_0 == 8)
    {
      const char *button3_symbolarray_0 = button3_valuearray[0];
      strcpy(menu1.button3.actions.symbol0, button3_symbolarray_0);
    }
    else
    {
      int button3_valuearray_0 = button3_valuearray[0]; // 1
      menu1.button3.actions.value0 = button3_valuearray_0;
    }

    if (button3_actionarray_1 == 4 || button3_actionarray_1 == 8)
    {
      const char *button3_symbolarray_1 = button3_valuearray[1]; // 1
      strcpy(menu1.button3.actions.symbol1, button3_symbolarray_1);
    }
    else
    {
      int button3_valuearray_1 = button3_valuearray[1]; // 1
      menu1.button3.actions.value1 = button3_valuearray_1;
    }

    if (button3_actionarray_2 == 4 || button3_actionarray_2 == 8)
    {
      const char *button3_symbolarray_2 = button3_valuearray[2]; // 1
      strcpy(menu1.button3.actions.symbol2, button3_symbolarray_2);
    }
    else
    {
      int button3_valuearray_2 = button3_valuearray[2]; // 1
      menu1.button3.actions.value2 = button3_valuearray_2;
    }

    menu1.button3.actions.action0 = button3_actionarray_0;
    menu1.button3.actions.action1 = button3_actionarray_1;
    menu1.button3.actions.action2 = button3_actionarray_2;

    menu1.button4.latch = doc["button4"]["latch"];

    JsonArray button4_actionarray = doc["button4"]["actionarray"];
    int button4_actionarray_0 = button4_actionarray[0]; // 3
    int button4_actionarray_1 = button4_actionarray[1]; // 0
    int button4_actionarray_2 = button4_actionarray[2]; // 0

    JsonArray button4_valuearray = doc["button4"]["valuearray"];

    if (button4_actionarray_0 == 4 || button4_actionarray_0 == 8)
    {
      const char *button4_symbolarray_0 = button4_valuearray[0]; // 1
      strcpy(menu1.button4.actions.symbol0, button4_symbolarray_0);
    }
    else
    {
      int button4_valuearray_0 = button4_valuearray[0]; // 1
      menu1.button4.actions.value0 = button4_valuearray_0;
    }

    if (button4_actionarray_1 == 4 || button4_actionarray_1 == 8)
    {
      const char *button4_symbolarray_1 = button4_valuearray[1]; // 1
      strcpy(menu1.button4.actions.symbol1, button4_symbolarray_1);
    }
    else
    {
      int button4_valuearray_1 = button4_valuearray[1]; // 1
      menu1.button4.actions.value1 = button4_valuearray_1;
    }

    if (button4_actionarray_2 == 4 || button4_actionarray_2 == 8)
    {
      const char *button4_symbolarray_2 = button4_valuearray[2]; // 1
      strcpy(menu1.button4.actions.symbol2, button4_symbolarray_2);
    }
    else
    {
      int button4_valuearray_2 = button4_valuearray[2]; // 1
      menu1.button4.actions.value2 = button4_valuearray_2;
    }
    menu1.button4.actions.action0 = button4_actionarray_0;
    menu1.button4.actions.action1 = button4_actionarray_1;
    menu1.button4.actions.action2 = button4_actionarray_2;

    strcpy(templogopath, logopath);
    strcat(templogopath, logo10);
    strcpy(screen1.logo0, templogopath);
    
    strcpy(templogopath, logopath);
    strcat(templogopath, logo11);
    strcpy(screen1.logo1, templogopath);

    strcpy(templogopath, logopath);
    strcat(templogopath, logo12);
    strcpy(screen1.logo2, templogopath);

    strcpy(templogopath, logopath);
    strcat(templogopath, logo13);
    strcpy(screen1.logo3, templogopath);

    strcpy(templogopath, logopath);
    strcat(templogopath, logo14);
    strcpy(screen1.logo4, templogopath);

    configfile.close();

    // --------------------- Loading menu 2 ----------------------
  }
  else if (value == "menu2")
  {
    File configfile = FILESYSTEM.open("/config/menu2.json", "r");

    DynamicJsonDocument doc(1200);

    DeserializationError error = deserializeJson(doc, configfile);

    const char *logo20 = doc["logo0"];
    const char *logo21 = doc["logo1"];
    const char *logo22 = doc["logo2"];
    const char *logo23 = doc["logo3"];
    const char *logo24 = doc["logo4"];

    menu2.button0.latch = doc["button0"]["latch"];
    
    JsonArray button0_actionarray = doc["button0"]["actionarray"];

    int button0_actionarray_0 = button0_actionarray[0];
    int button0_actionarray_1 = button0_actionarray[1];
    int button0_actionarray_2 = button0_actionarray[2];

    JsonArray button0_valuearray = doc["button0"]["valuearray"];

    if (button0_actionarray_0 == 4 || button0_actionarray_0 == 8)
    {
      const char *button0_symbolarray_0 = button0_valuearray[0];
      strcpy(menu2.button0.actions.symbol0, button0_symbolarray_0);
    }
    else
    {
      int button0_valuearray_0 = button0_valuearray[0];
      menu2.button0.actions.value0 = button0_valuearray_0;
    }

    if (button0_actionarray_1 == 4 || button0_actionarray_1 == 8)
    {
      const char *button0_symbolarray_1 = button0_valuearray[1];
      strcpy(menu2.button0.actions.symbol1, button0_symbolarray_1);
    }
    else
    {
      int button0_valuearray_1 = button0_valuearray[1];
      menu2.button0.actions.value1 = button0_valuearray_1;
    }

    if (button0_actionarray_2 == 4 || button0_actionarray_2 == 8)
    {
      const char *button0_symbolarray_2 = button0_valuearray[2];
      strcpy(menu2.button0.actions.symbol2, button0_symbolarray_2);
    }
    else
    {
      int button0_valuearray_2 = button0_valuearray[2];
      menu2.button0.actions.value2 = button0_valuearray_2;
    }

    menu2.button0.actions.action0 = button0_actionarray_0;
    menu2.button0.actions.action1 = button0_actionarray_1;
    menu2.button0.actions.action2 = button0_actionarray_2;

    menu2.button1.latch = doc["button1"]["latch"];

    JsonArray button1_actionarray = doc["button1"]["actionarray"];
    int button1_actionarray_0 = button1_actionarray[0];
    int button1_actionarray_1 = button1_actionarray[1];
    int button1_actionarray_2 = button1_actionarray[2];

    JsonArray button1_valuearray = doc["button1"]["valuearray"];

    if (button1_actionarray_0 == 4 || button1_actionarray_0 == 8)
    {
      const char *button1_symbolarray_0 = button1_valuearray[0];
      strcpy(menu2.button1.actions.symbol0, button1_symbolarray_0);
    }
    else
    {
      int button1_valuearray_0 = button1_valuearray[0];
      menu2.button1.actions.value0 = button1_valuearray_0;
    }

    if (button1_actionarray_1 == 4 || button1_actionarray_1 == 8)
    {
      const char *button1_symbolarray_1 = button1_valuearray[1];
      strcpy(menu2.button1.actions.symbol1, button1_symbolarray_1);
    }
    else
    {
      int button1_valuearray_1 = button1_valuearray[1];
      menu2.button1.actions.value1 = button1_valuearray_1;
    }

    if (button1_actionarray_2 == 4 || button1_actionarray_2 == 8)
    {
      const char *button1_symbolarray_2 = button1_valuearray[2];
      strcpy(menu2.button1.actions.symbol2, button1_symbolarray_2);
    }
    else
    {
      int button1_valuearray_2 = button1_valuearray[2];
      menu2.button1.actions.value2 = button1_valuearray_2;
    }

    menu2.button1.actions.action0 = button1_actionarray_0;
    menu2.button1.actions.action1 = button1_actionarray_1;
    menu2.button1.actions.action2 = button1_actionarray_2;

    menu2.button2.latch = doc["button2"]["latch"];

    JsonArray button2_actionarray = doc["button2"]["actionarray"];
    int button2_actionarray_0 = button2_actionarray[0];
    int button2_actionarray_1 = button2_actionarray[1];
    int button2_actionarray_2 = button2_actionarray[2];

    JsonArray button2_valuearray = doc["button2"]["valuearray"];

    if (button2_actionarray_0 == 4 || button2_actionarray_0 == 8)
    {
      const char *button2_symbolarray_0 = button2_valuearray[0];
      strcpy(menu2.button2.actions.symbol0, button2_symbolarray_0);
    }
    else
    {
      int button2_valuearray_0 = button2_valuearray[0];
      menu2.button2.actions.value0 = button2_valuearray_0;
    }

    if (button2_actionarray_1 == 4 || button2_actionarray_1 == 8)
    {
      const char *button2_symbolarray_1 = button2_valuearray[1];
      strcpy(menu2.button2.actions.symbol1, button2_symbolarray_1);
    }
    else
    {
      int button2_valuearray_1 = button2_valuearray[1];
      menu2.button2.actions.value1 = button2_valuearray_1;
    }

    if (button2_actionarray_2 == 4 || button2_actionarray_2 == 8)
    {
      const char *button2_symbolarray_2 = button2_valuearray[2];
      strcpy(menu2.button2.actions.symbol2, button2_symbolarray_2);
    }
    else
    {
      int button2_valuearray_2 = button2_valuearray[2];
      menu2.button2.actions.value2 = button2_valuearray_2;
    }

    menu2.button2.actions.action0 = button2_actionarray_0;
    menu2.button2.actions.action1 = button2_actionarray_1;
    menu2.button2.actions.action2 = button2_actionarray_2;

    menu2.button3.latch = doc["button3"]["latch"];

    JsonArray button3_actionarray = doc["button3"]["actionarray"];
    int button3_actionarray_0 = button3_actionarray[0]; // 3
    int button3_actionarray_1 = button3_actionarray[1]; // 0
    int button3_actionarray_2 = button3_actionarray[2]; // 0

    JsonArray button3_valuearray = doc["button3"]["valuearray"];

    if (button3_actionarray_0 == 4 || button3_actionarray_0 == 8)
    {
      const char *button3_symbolarray_0 = button3_valuearray[0];
      strcpy(menu2.button3.actions.symbol0, button3_symbolarray_0);
    }
    else
    {
      int button3_valuearray_0 = button3_valuearray[0]; // 1
      menu2.button3.actions.value0 = button3_valuearray_0;
    }

    if (button3_actionarray_1 == 4 || button3_actionarray_1 == 8)
    {
      const char *button3_symbolarray_1 = button3_valuearray[1]; // 1
      strcpy(menu2.button3.actions.symbol1, button3_symbolarray_1);
    }
    else
    {
      int button3_valuearray_1 = button3_valuearray[1]; // 1
      menu2.button3.actions.value1 = button3_valuearray_1;
    }

    if (button3_actionarray_2 == 4 || button3_actionarray_2 == 8)
    {
      const char *button3_symbolarray_2 = button3_valuearray[2]; // 1
      strcpy(menu2.button3.actions.symbol2, button3_symbolarray_2);
    }
    else
    {
      int button3_valuearray_2 = button3_valuearray[2]; // 1
      menu2.button3.actions.value2 = button3_valuearray_2;
    }

    menu2.button3.actions.action0 = button3_actionarray_0;
    menu2.button3.actions.action1 = button3_actionarray_1;
    menu2.button3.actions.action2 = button3_actionarray_2;

    menu2.button4.latch = doc["button4"]["latch"];

    JsonArray button4_actionarray = doc["button4"]["actionarray"];
    int button4_actionarray_0 = button4_actionarray[0]; // 3
    int button4_actionarray_1 = button4_actionarray[1]; // 0
    int button4_actionarray_2 = button4_actionarray[2]; // 0

    JsonArray button4_valuearray = doc["button4"]["valuearray"];

    if (button4_actionarray_0 == 4 || button4_actionarray_0 == 8)
    {
      const char *button4_symbolarray_0 = button4_valuearray[0]; // 1
      strcpy(menu2.button4.actions.symbol0, button4_symbolarray_0);
    }
    else
    {
      int button4_valuearray_0 = button4_valuearray[0]; // 1
      menu2.button4.actions.value0 = button4_valuearray_0;
    }

    if (button4_actionarray_1 == 4 || button4_actionarray_1 == 8)
    {
      const char *button4_symbolarray_1 = button4_valuearray[1]; // 1
      strcpy(menu2.button4.actions.symbol1, button4_symbolarray_1);
    }
    else
    {
      int button4_valuearray_1 = button4_valuearray[1]; // 1
      menu2.button4.actions.value1 = button4_valuearray_1;
    }

    if (button4_actionarray_2 == 4 || button4_actionarray_2 == 8)
    {
      const char *button4_symbolarray_2 = button4_valuearray[2]; // 1
      strcpy(menu2.button4.actions.symbol2, button4_symbolarray_2);
    }
    else
    {
      int button4_valuearray_2 = button4_valuearray[2]; // 1
      menu2.button4.actions.value2 = button4_valuearray_2;
    }
    menu2.button4.actions.action0 = button4_actionarray_0;
    menu2.button4.actions.action1 = button4_actionarray_1;
    menu2.button4.actions.action2 = button4_actionarray_2;

    strcpy(templogopath, logopath);
    strcat(templogopath, logo20);
    strcpy(screen2.logo0, templogopath);

    strcpy(templogopath, logopath);
    strcat(templogopath, logo21);
    strcpy(screen2.logo1, templogopath);

    strcpy(templogopath, logopath);
    strcat(templogopath, logo22);
    strcpy(screen2.logo2, templogopath);

    strcpy(templogopath, logopath);
    strcat(templogopath, logo23);
    strcpy(screen2.logo3, templogopath);

    strcpy(templogopath, logopath);
    strcat(templogopath, logo24);
    strcpy(screen2.logo4, templogopath);

    configfile.close();

    // --------------------- Loading menu 3 ----------------------
  }
  else if (value == "menu3")
  {
    File configfile = FILESYSTEM.open("/config/menu3.json", "r");

    DynamicJsonDocument doc(1200);

    DeserializationError error = deserializeJson(doc, configfile);

    const char *logo30 = doc["logo0"];
    const char *logo31 = doc["logo1"];
    const char *logo32 = doc["logo2"];
    const char *logo33 = doc["logo3"];
    const char *logo34 = doc["logo4"];

    menu3.button0.latch = doc["button0"]["latch"];

    JsonArray button0_actionarray = doc["button0"]["actionarray"];

    int button0_actionarray_0 = button0_actionarray[0];
    int button0_actionarray_1 = button0_actionarray[1];
    int button0_actionarray_2 = button0_actionarray[2];

    JsonArray button0_valuearray = doc["button0"]["valuearray"];

    if (button0_actionarray_0 == 4 || button0_actionarray_0 == 8)
    {
      const char *button0_symbolarray_0 = button0_valuearray[0];
      strcpy(menu3.button0.actions.symbol0, button0_symbolarray_0);
    }
    else
    {
      int button0_valuearray_0 = button0_valuearray[0];
      menu3.button0.actions.value0 = button0_valuearray_0;
    }

    if (button0_actionarray_1 == 4 || button0_actionarray_1 == 8)
    {
      const char *button0_symbolarray_1 = button0_valuearray[1];
      strcpy(menu3.button0.actions.symbol1, button0_symbolarray_1);
    }
    else
    {
      int button0_valuearray_1 = button0_valuearray[1];
      menu3.button0.actions.value1 = button0_valuearray_1;
    }

    if (button0_actionarray_2 == 4 || button0_actionarray_2 == 8)
    {
      const char *button0_symbolarray_2 = button0_valuearray[2];
      strcpy(menu3.button0.actions.symbol2, button0_symbolarray_2);
    }
    else
    {
      int button0_valuearray_2 = button0_valuearray[2];
      menu3.button0.actions.value2 = button0_valuearray_2;
    }

    menu3.button0.actions.action0 = button0_actionarray_0;
    menu3.button0.actions.action1 = button0_actionarray_1;
    menu3.button0.actions.action2 = button0_actionarray_2;
    
    menu3.button1.latch = doc["button1"]["latch"];

    JsonArray button1_actionarray = doc["button1"]["actionarray"];
    int button1_actionarray_0 = button1_actionarray[0];
    int button1_actionarray_1 = button1_actionarray[1];
    int button1_actionarray_2 = button1_actionarray[2];

    JsonArray button1_valuearray = doc["button1"]["valuearray"];

    if (button1_actionarray_0 == 4 || button1_actionarray_0 == 8)
    {
      const char *button1_symbolarray_0 = button1_valuearray[0];
      strcpy(menu3.button1.actions.symbol0, button1_symbolarray_0);
    }
    else
    {
      int button1_valuearray_0 = button1_valuearray[0];
      menu3.button1.actions.value0 = button1_valuearray_0;
    }

    if (button1_actionarray_1 == 4 || button1_actionarray_1 == 8)
    {
      const char *button1_symbolarray_1 = button1_valuearray[1];
      strcpy(menu3.button1.actions.symbol1, button1_symbolarray_1);
    }
    else
    {
      int button1_valuearray_1 = button1_valuearray[1];
      menu3.button1.actions.value1 = button1_valuearray_1;
    }

    if (button1_actionarray_2 == 4 || button1_actionarray_2 == 8)
    {
      const char *button1_symbolarray_2 = button1_valuearray[2];
      strcpy(menu3.button1.actions.symbol2, button1_symbolarray_2);
    }
    else
    {
      int button1_valuearray_2 = button1_valuearray[2];
      menu3.button1.actions.value2 = button1_valuearray_2;
    }

    menu3.button1.actions.action0 = button1_actionarray_0;
    menu3.button1.actions.action1 = button1_actionarray_1;
    menu3.button1.actions.action2 = button1_actionarray_2;

    menu3.button2.latch = doc["button2"]["latch"];

    JsonArray button2_actionarray = doc["button2"]["actionarray"];
    int button2_actionarray_0 = button2_actionarray[0];
    int button2_actionarray_1 = button2_actionarray[1];
    int button2_actionarray_2 = button2_actionarray[2];

    JsonArray button2_valuearray = doc["button2"]["valuearray"];

    if (button2_actionarray_0 == 4 || button2_actionarray_0 == 8)
    {
      const char *button2_symbolarray_0 = button2_valuearray[0];
      strcpy(menu3.button2.actions.symbol0, button2_symbolarray_0);
    }
    else
    {
      int button2_valuearray_0 = button2_valuearray[0];
      menu3.button2.actions.value0 = button2_valuearray_0;
    }

    if (button2_actionarray_1 == 4 || button2_actionarray_1 == 8)
    {
      const char *button2_symbolarray_1 = button2_valuearray[1];
      strcpy(menu3.button2.actions.symbol1, button2_symbolarray_1);
    }
    else
    {
      int button2_valuearray_1 = button2_valuearray[1];
      menu3.button2.actions.value1 = button2_valuearray_1;
    }

    if (button2_actionarray_2 == 4 || button2_actionarray_2 == 8)
    {
      const char *button2_symbolarray_2 = button2_valuearray[2];
      strcpy(menu3.button2.actions.symbol2, button2_symbolarray_2);
    }
    else
    {
      int button2_valuearray_2 = button2_valuearray[2];
      menu3.button2.actions.value2 = button2_valuearray_2;
    }

    menu3.button2.actions.action0 = button2_actionarray_0;
    menu3.button2.actions.action1 = button2_actionarray_1;
    menu3.button2.actions.action2 = button2_actionarray_2;

    JsonArray button3_actionarray = doc["button3"]["actionarray"];
    int button3_actionarray_0 = button3_actionarray[0]; // 3
    int button3_actionarray_1 = button3_actionarray[1]; // 0
    int button3_actionarray_2 = button3_actionarray[2]; // 0

    JsonArray button3_valuearray = doc["button3"]["valuearray"];

    if (button3_actionarray_0 == 4 || button3_actionarray_0 == 8)
    {
      const char *button3_symbolarray_0 = button3_valuearray[0];
      strcpy(menu3.button3.actions.symbol0, button3_symbolarray_0);
    }
    else
    {
      int button3_valuearray_0 = button3_valuearray[0]; // 1
      menu3.button3.actions.value0 = button3_valuearray_0;
    }

    if (button3_actionarray_1 == 4 || button3_actionarray_1 == 8)
    {
      const char *button3_symbolarray_1 = button3_valuearray[1]; // 1
      strcpy(menu3.button3.actions.symbol1, button3_symbolarray_1);
    }
    else
    {
      int button3_valuearray_1 = button3_valuearray[1]; // 1
      menu3.button3.actions.value1 = button3_valuearray_1;
    }

    if (button3_actionarray_2 == 4 || button3_actionarray_2 == 8)
    {
      const char *button3_symbolarray_2 = button3_valuearray[2]; // 1
      strcpy(menu3.button3.actions.symbol2, button3_symbolarray_2);
    }
    else
    {
      int button3_valuearray_2 = button3_valuearray[2]; // 1
      menu3.button3.actions.value2 = button3_valuearray_2;
    }

    menu3.button3.actions.action0 = button3_actionarray_0;
    menu3.button3.actions.action1 = button3_actionarray_1;
    menu3.button3.actions.action2 = button3_actionarray_2;

    menu3.button4.latch = doc["button4"]["latch"];

    JsonArray button4_actionarray = doc["button4"]["actionarray"];
    int button4_actionarray_0 = button4_actionarray[0]; // 3
    int button4_actionarray_1 = button4_actionarray[1]; // 0
    int button4_actionarray_2 = button4_actionarray[2]; // 0

    JsonArray button4_valuearray = doc["button4"]["valuearray"];

    if (button4_actionarray_0 == 4 || button4_actionarray_0 == 8)
    {
      const char *button4_symbolarray_0 = button4_valuearray[0]; // 1
      strcpy(menu3.button4.actions.symbol0, button4_symbolarray_0);
    }
    else
    {
      int button4_valuearray_0 = button4_valuearray[0]; // 1
      menu3.button4.actions.value0 = button4_valuearray_0;
    }

    if (button4_actionarray_1 == 4 || button4_actionarray_1 == 8)
    {
      const char *button4_symbolarray_1 = button4_valuearray[1]; // 1
      strcpy(menu3.button4.actions.symbol1, button4_symbolarray_1);
    }
    else
    {
      int button4_valuearray_1 = button4_valuearray[1]; // 1
      menu3.button4.actions.value1 = button4_valuearray_1;
    }

    if (button4_actionarray_2 == 4 || button4_actionarray_2 == 8)
    {
      const char *button4_symbolarray_2 = button4_valuearray[2]; // 1
      strcpy(menu3.button4.actions.symbol2, button4_symbolarray_2);
    }
    else
    {
      int button4_valuearray_2 = button4_valuearray[2]; // 1
      menu3.button4.actions.value2 = button4_valuearray_2;
    }
    menu3.button4.actions.action0 = button4_actionarray_0;
    menu3.button4.actions.action1 = button4_actionarray_1;
    menu3.button4.actions.action2 = button4_actionarray_2;

    strcpy(templogopath, logopath);
    strcat(templogopath, logo30);
    strcpy(screen3.logo0, templogopath);

    strcpy(templogopath, logopath);
    strcat(templogopath, logo31);
    strcpy(screen3.logo1, templogopath);

    strcpy(templogopath, logopath);
    strcat(templogopath, logo32);
    strcpy(screen3.logo2, templogopath);

    strcpy(templogopath, logopath);
    strcat(templogopath, logo33);
    strcpy(screen3.logo3, templogopath);

    strcpy(templogopath, logopath);
    strcat(templogopath, logo34);
    strcpy(screen3.logo4, templogopath);

    configfile.close();

    // --------------------- Loading menu 4 ----------------------
  }
  else if (value == "menu4")
  {
    File configfile = FILESYSTEM.open("/config/menu4.json", "r");

    DynamicJsonDocument doc(1200);

    DeserializationError error = deserializeJson(doc, configfile);

    const char *logo40 = doc["logo0"];
    const char *logo41 = doc["logo1"];
    const char *logo42 = doc["logo2"];
    const char *logo43 = doc["logo3"];
    const char *logo44 = doc["logo4"];

    menu4.button0.latch = doc["button0"]["latch"];

    JsonArray button0_actionarray = doc["button0"]["actionarray"];

    int button0_actionarray_0 = button0_actionarray[0];
    int button0_actionarray_1 = button0_actionarray[1];
    int button0_actionarray_2 = button0_actionarray[2];

    JsonArray button0_valuearray = doc["button0"]["valuearray"];

    if (button0_actionarray_0 == 4 || button0_actionarray_0 == 8)
    {
      const char *button0_symbolarray_0 = button0_valuearray[0];
      strcpy(menu4.button0.actions.symbol0, button0_symbolarray_0);
    }
    else
    {
      int button0_valuearray_0 = button0_valuearray[0];
      menu4.button0.actions.value0 = button0_valuearray_0;
    }

    if (button0_actionarray_1 == 4 || button0_actionarray_1 == 8)
    {
      const char *button0_symbolarray_1 = button0_valuearray[1];
      strcpy(menu4.button0.actions.symbol1, button0_symbolarray_1);
    }
    else
    {
      int button0_valuearray_1 = button0_valuearray[1];
      menu4.button0.actions.value1 = button0_valuearray_1;
    }

    if (button0_actionarray_2 == 4 || button0_actionarray_2 == 8)
    {
      const char *button0_symbolarray_2 = button0_valuearray[2];
      strcpy(menu4.button0.actions.symbol2, button0_symbolarray_2);
    }
    else
    {
      int button0_valuearray_2 = button0_valuearray[2];
      menu4.button0.actions.value2 = button0_valuearray_2;
    }

    menu4.button0.actions.action0 = button0_actionarray_0;
    menu4.button0.actions.action1 = button0_actionarray_1;
    menu4.button0.actions.action2 = button0_actionarray_2;

    menu4.button1.latch = doc["button1"]["latch"];

    JsonArray button1_actionarray = doc["button1"]["actionarray"];
    int button1_actionarray_0 = button1_actionarray[0];
    int button1_actionarray_1 = button1_actionarray[1];
    int button1_actionarray_2 = button1_actionarray[2];

    JsonArray button1_valuearray = doc["button1"]["valuearray"];

    if (button1_actionarray_0 == 4 || button1_actionarray_0 == 8)
    {
      const char *button1_symbolarray_0 = button1_valuearray[0];
      strcpy(menu4.button1.actions.symbol0, button1_symbolarray_0);
    }
    else
    {
      int button1_valuearray_0 = button1_valuearray[0];
      menu4.button1.actions.value0 = button1_valuearray_0;
    }

    if (button1_actionarray_1 == 4 || button1_actionarray_1 == 8)
    {
      const char *button1_symbolarray_1 = button1_valuearray[1];
      strcpy(menu4.button1.actions.symbol1, button1_symbolarray_1);
    }
    else
    {
      int button1_valuearray_1 = button1_valuearray[1];
      menu4.button1.actions.value1 = button1_valuearray_1;
    }

    if (button1_actionarray_2 == 4 || button1_actionarray_2 == 8)
    {
      const char *button1_symbolarray_2 = button1_valuearray[2];
      strcpy(menu4.button1.actions.symbol2, button1_symbolarray_2);
    }
    else
    {
      int button1_valuearray_2 = button1_valuearray[2];
      menu4.button1.actions.value2 = button1_valuearray_2;
    }

    menu4.button1.actions.action0 = button1_actionarray_0;
    menu4.button1.actions.action1 = button1_actionarray_1;
    menu4.button1.actions.action2 = button1_actionarray_2;

    menu4.button2.latch = doc["button2"]["latch"];

    JsonArray button2_actionarray = doc["button2"]["actionarray"];
    int button2_actionarray_0 = button2_actionarray[0];
    int button2_actionarray_1 = button2_actionarray[1];
    int button2_actionarray_2 = button2_actionarray[2];

    JsonArray button2_valuearray = doc["button2"]["valuearray"];

    if (button2_actionarray_0 == 4 || button2_actionarray_0 == 8)
    {
      const char *button2_symbolarray_0 = button2_valuearray[0];
      strcpy(menu4.button2.actions.symbol0, button2_symbolarray_0);
    }
    else
    {
      int button2_valuearray_0 = button2_valuearray[0];
      menu4.button2.actions.value0 = button2_valuearray_0;
    }

    if (button2_actionarray_1 == 4 || button2_actionarray_1 == 8)
    {
      const char *button2_symbolarray_1 = button2_valuearray[1];
      strcpy(menu4.button2.actions.symbol1, button2_symbolarray_1);
    }
    else
    {
      int button2_valuearray_1 = button2_valuearray[1];
      menu4.button2.actions.value1 = button2_valuearray_1;
    }

    if (button2_actionarray_2 == 4 || button2_actionarray_2 == 8)
    {
      const char *button2_symbolarray_2 = button2_valuearray[2];
      strcpy(menu4.button2.actions.symbol2, button2_symbolarray_2);
    }
    else
    {
      int button2_valuearray_2 = button2_valuearray[2];
      menu4.button2.actions.value2 = button2_valuearray_2;
    }

    menu4.button2.actions.action0 = button2_actionarray_0;
    menu4.button2.actions.action1 = button2_actionarray_1;
    menu4.button2.actions.action2 = button2_actionarray_2;

    menu4.button3.latch = doc["button3"]["latch"];

    JsonArray button3_actionarray = doc["button3"]["actionarray"];
    int button3_actionarray_0 = button3_actionarray[0]; // 3
    int button3_actionarray_1 = button3_actionarray[1]; // 0
    int button3_actionarray_2 = button3_actionarray[2]; // 0

    JsonArray button3_valuearray = doc["button3"]["valuearray"];

    if (button3_actionarray_0 == 4 || button3_actionarray_0 == 8)
    {
      const char *button3_symbolarray_0 = button3_valuearray[0];
      strcpy(menu4.button3.actions.symbol0, button3_symbolarray_0);
    }
    else
    {
      int button3_valuearray_0 = button3_valuearray[0]; // 1
      menu4.button3.actions.value0 = button3_valuearray_0;
    }

    if (button3_actionarray_1 == 4 || button3_actionarray_1 == 8)
    {
      const char *button3_symbolarray_1 = button3_valuearray[1]; // 1
      strcpy(menu4.button3.actions.symbol1, button3_symbolarray_1);
    }
    else
    {
      int button3_valuearray_1 = button3_valuearray[1]; // 1
      menu4.button3.actions.value1 = button3_valuearray_1;
    }

    if (button3_actionarray_2 == 4 || button3_actionarray_2 == 8)
    {
      const char *button3_symbolarray_2 = button3_valuearray[2]; // 1
      strcpy(menu4.button3.actions.symbol2, button3_symbolarray_2);
    }
    else
    {
      int button3_valuearray_2 = button3_valuearray[2]; // 1
      menu4.button3.actions.value2 = button3_valuearray_2;
    }

    menu4.button3.actions.action0 = button3_actionarray_0;
    menu4.button3.actions.action1 = button3_actionarray_1;
    menu4.button3.actions.action2 = button3_actionarray_2;

    menu4.button4.latch = doc["button4"]["latch"];

    JsonArray button4_actionarray = doc["button4"]["actionarray"];
    int button4_actionarray_0 = button4_actionarray[0]; // 3
    int button4_actionarray_1 = button4_actionarray[1]; // 0
    int button4_actionarray_2 = button4_actionarray[2]; // 0

    JsonArray button4_valuearray = doc["button4"]["valuearray"];

    if (button4_actionarray_0 == 4 || button4_actionarray_0 == 8)
    {
      const char *button4_symbolarray_0 = button4_valuearray[0]; // 1
      strcpy(menu4.button4.actions.symbol0, button4_symbolarray_0);
    }
    else
    {
      int button4_valuearray_0 = button4_valuearray[0]; // 1
      menu4.button4.actions.value0 = button4_valuearray_0;
    }

    if (button4_actionarray_1 == 4 || button4_actionarray_1 == 8)
    {
      const char *button4_symbolarray_1 = button4_valuearray[1]; // 1
      strcpy(menu4.button4.actions.symbol1, button4_symbolarray_1);
    }
    else
    {
      int button4_valuearray_1 = button4_valuearray[1]; // 1
      menu4.button4.actions.value1 = button4_valuearray_1;
    }

    if (button4_actionarray_2 == 4 || button4_actionarray_2 == 8)
    {
      const char *button4_symbolarray_2 = button4_valuearray[2]; // 1
      strcpy(menu4.button4.actions.symbol2, button4_symbolarray_2);
    }
    else
    {
      int button4_valuearray_2 = button4_valuearray[2]; // 1
      menu4.button4.actions.value2 = button4_valuearray_2;
    }
    menu4.button4.actions.action0 = button4_actionarray_0;
    menu4.button4.actions.action1 = button4_actionarray_1;
    menu4.button4.actions.action2 = button4_actionarray_2;

    strcpy(templogopath, logopath);
    strcat(templogopath, logo40);
    strcpy(screen4.logo0, templogopath);

    strcpy(templogopath, logopath);
    strcat(templogopath, logo41);
    strcpy(screen4.logo1, templogopath);

    strcpy(templogopath, logopath);
    strcat(templogopath, logo42);
    strcpy(screen4.logo2, templogopath);

    strcpy(templogopath, logopath);
    strcat(templogopath, logo43);
    strcpy(screen4.logo3, templogopath);

    strcpy(templogopath, logopath);
    strcat(templogopath, logo44);
    strcpy(screen4.logo4, templogopath);

    configfile.close();

    // --------------------- Loading menu 5 ----------------------
  }
  else if (value == "menu5")
  {
    File configfile = FILESYSTEM.open("/config/menu5.json", "r");

    DynamicJsonDocument doc(1200);

    DeserializationError error = deserializeJson(doc, configfile);

    const char *logo50 = doc["logo0"];
    const char *logo51 = doc["logo1"];
    const char *logo52 = doc["logo2"];
    const char *logo53 = doc["logo3"];
    const char *logo54 = doc["logo4"];

    menu5.button0.latch = doc["button0"]["latch"];

    JsonArray button0_actionarray = doc["button0"]["actionarray"];

    int button0_actionarray_0 = button0_actionarray[0];
    int button0_actionarray_1 = button0_actionarray[1];
    int button0_actionarray_2 = button0_actionarray[2];

    JsonArray button0_valuearray = doc["button0"]["valuearray"];

    if (button0_actionarray_0 == 4 || button0_actionarray_0 == 8)
    {
      const char *button0_symbolarray_0 = button0_valuearray[0];
      strcpy(menu5.button0.actions.symbol0, button0_symbolarray_0);
    }
    else
    {
      int button0_valuearray_0 = button0_valuearray[0];
      menu5.button0.actions.value0 = button0_valuearray_0;
    }

    if (button0_actionarray_1 == 4 || button0_actionarray_1 == 8)
    {
      const char *button0_symbolarray_1 = button0_valuearray[1];
      strcpy(menu5.button0.actions.symbol1, button0_symbolarray_1);
    }
    else
    {
      int button0_valuearray_1 = button0_valuearray[1];
      menu5.button0.actions.value1 = button0_valuearray_1;
    }

    if (button0_actionarray_2 == 4 || button0_actionarray_2 == 8)
    {
      const char *button0_symbolarray_2 = button0_valuearray[2];
      strcpy(menu5.button0.actions.symbol2, button0_symbolarray_2);
    }
    else
    {
      int button0_valuearray_2 = button0_valuearray[2];
      menu5.button0.actions.value2 = button0_valuearray_2;
    }

    menu5.button0.actions.action0 = button0_actionarray_0;
    menu5.button0.actions.action1 = button0_actionarray_1;
    menu5.button0.actions.action2 = button0_actionarray_2;

    menu5.button1.latch = doc["button1"]["latch"];

    JsonArray button1_actionarray = doc["button1"]["actionarray"];
    int button1_actionarray_0 = button1_actionarray[0];
    int button1_actionarray_1 = button1_actionarray[1];
    int button1_actionarray_2 = button1_actionarray[2];

    JsonArray button1_valuearray = doc["button1"]["valuearray"];

    if (button1_actionarray_0 == 4 || button1_actionarray_0 == 8)
    {
      const char *button1_symbolarray_0 = button1_valuearray[0];
      strcpy(menu5.button1.actions.symbol0, button1_symbolarray_0);
    }
    else
    {
      int button1_valuearray_0 = button1_valuearray[0];
      menu5.button1.actions.value0 = button1_valuearray_0;
    }

    if (button1_actionarray_1 == 4 || button1_actionarray_1 == 8)
    {
      const char *button1_symbolarray_1 = button1_valuearray[1];
      strcpy(menu5.button1.actions.symbol1, button1_symbolarray_1);
    }
    else
    {
      int button1_valuearray_1 = button1_valuearray[1];
      menu5.button1.actions.value1 = button1_valuearray_1;
    }

    if (button1_actionarray_2 == 4 || button1_actionarray_2 == 8)
    {
      const char *button1_symbolarray_2 = button1_valuearray[2];
      strcpy(menu5.button1.actions.symbol2, button1_symbolarray_2);
    }
    else
    {
      int button1_valuearray_2 = button1_valuearray[2];
      menu5.button1.actions.value2 = button1_valuearray_2;
    }

    menu5.button1.actions.action0 = button1_actionarray_0;
    menu5.button1.actions.action1 = button1_actionarray_1;
    menu5.button1.actions.action2 = button1_actionarray_2;

    menu5.button2.latch = doc["button2"]["latch"];

    JsonArray button2_actionarray = doc["button2"]["actionarray"];
    int button2_actionarray_0 = button2_actionarray[0];
    int button2_actionarray_1 = button2_actionarray[1];
    int button2_actionarray_2 = button2_actionarray[2];

    JsonArray button2_valuearray = doc["button2"]["valuearray"];

    if (button2_actionarray_0 == 4 || button2_actionarray_0 == 8)
    {
      const char *button2_symbolarray_0 = button2_valuearray[0];
      strcpy(menu5.button2.actions.symbol0, button2_symbolarray_0);
    }
    else
    {
      int button2_valuearray_0 = button2_valuearray[0];
      menu5.button2.actions.value0 = button2_valuearray_0;
    }

    if (button2_actionarray_1 == 4 || button2_actionarray_1 == 8)
    {
      const char *button2_symbolarray_1 = button2_valuearray[1];
      strcpy(menu5.button2.actions.symbol1, button2_symbolarray_1);
    }
    else
    {
      int button2_valuearray_1 = button2_valuearray[1];
      menu5.button2.actions.value1 = button2_valuearray_1;
    }

    if (button2_actionarray_2 == 4 || button2_actionarray_2 == 8)
    {
      const char *button2_symbolarray_2 = button2_valuearray[2];
      strcpy(menu5.button2.actions.symbol2, button2_symbolarray_2);
    }
    else
    {
      int button2_valuearray_2 = button2_valuearray[2];
      menu5.button2.actions.value2 = button2_valuearray_2;
    }

    menu5.button2.actions.action0 = button2_actionarray_0;
    menu5.button2.actions.action1 = button2_actionarray_1;
    menu5.button2.actions.action2 = button2_actionarray_2;

    menu5.button3.latch = doc["button3"]["latch"];

    JsonArray button3_actionarray = doc["button3"]["actionarray"];
    int button3_actionarray_0 = button3_actionarray[0]; // 3
    int button3_actionarray_1 = button3_actionarray[1]; // 0
    int button3_actionarray_2 = button3_actionarray[2]; // 0

    JsonArray button3_valuearray = doc["button3"]["valuearray"];

    if (button3_actionarray_0 == 4 || button3_actionarray_0 == 8)
    {
      const char *button3_symbolarray_0 = button3_valuearray[0];
      strcpy(menu5.button3.actions.symbol0, button3_symbolarray_0);
    }
    else
    {
      int button3_valuearray_0 = button3_valuearray[0]; // 1
      menu5.button3.actions.value0 = button3_valuearray_0;
    }

    if (button3_actionarray_1 == 4 || button3_actionarray_1 == 8)
    {
      const char *button3_symbolarray_1 = button3_valuearray[1]; // 1
      strcpy(menu5.button3.actions.symbol1, button3_symbolarray_1);
    }
    else
    {
      int button3_valuearray_1 = button3_valuearray[1]; // 1
      menu5.button3.actions.value1 = button3_valuearray_1;
    }

    if (button3_actionarray_2 == 4 || button3_actionarray_2 == 8)
    {
      const char *button3_symbolarray_2 = button3_valuearray[2]; // 1
      strcpy(menu5.button3.actions.symbol2, button3_symbolarray_2);
    }
    else
    {
      int button3_valuearray_2 = button3_valuearray[2]; // 1
      menu5.button3.actions.value2 = button3_valuearray_2;
    }

    menu5.button3.actions.action0 = button3_actionarray_0;
    menu5.button3.actions.action1 = button3_actionarray_1;
    menu5.button3.actions.action2 = button3_actionarray_2;

    menu5.button4.latch = doc["button4"]["latch"];

    JsonArray button4_actionarray = doc["button4"]["actionarray"];
    int button4_actionarray_0 = button4_actionarray[0]; // 3
    int button4_actionarray_1 = button4_actionarray[1]; // 0
    int button4_actionarray_2 = button4_actionarray[2]; // 0

    JsonArray button4_valuearray = doc["button4"]["valuearray"];

    if (button4_actionarray_0 == 4 || button4_actionarray_0 == 8)
    {
      const char *button4_symbolarray_0 = button4_valuearray[0]; // 1
      strcpy(menu5.button4.actions.symbol0, button4_symbolarray_0);
    }
    else
    {
      int button4_valuearray_0 = button4_valuearray[0]; // 1
      menu5.button4.actions.value0 = button4_valuearray_0;
    }

    if (button4_actionarray_1 == 4 || button4_actionarray_1 == 8)
    {
      const char *button4_symbolarray_1 = button4_valuearray[1]; // 1
      strcpy(menu5.button4.actions.symbol1, button4_symbolarray_1);
    }
    else
    {
      int button4_valuearray_1 = button4_valuearray[1]; // 1
      menu5.button4.actions.value1 = button4_valuearray_1;
    }

    if (button4_actionarray_2 == 4 || button4_actionarray_2 == 8)
    {
      const char *button4_symbolarray_2 = button4_valuearray[2]; // 1
      strcpy(menu5.button4.actions.symbol2, button4_symbolarray_2);
    }
    else
    {
      int button4_valuearray_2 = button4_valuearray[2]; // 1
      menu5.button4.actions.value2 = button4_valuearray_2;
    }
    menu5.button4.actions.action0 = button4_actionarray_0;
    menu5.button4.actions.action1 = button4_actionarray_1;
    menu5.button4.actions.action2 = button4_actionarray_2;

    strcpy(templogopath, logopath);
    strcat(templogopath, logo50);
    strcpy(screen5.logo0, templogopath);

    strcpy(templogopath, logopath);
    strcat(templogopath, logo51);
    strcpy(screen5.logo1, templogopath);

    strcpy(templogopath, logopath);
    strcat(templogopath, logo52);
    strcpy(screen5.logo2, templogopath);

    strcpy(templogopath, logopath);
    strcat(templogopath, logo53);
    strcpy(screen5.logo3, templogopath);

    strcpy(templogopath, logopath);
    strcat(templogopath, logo54);
    strcpy(screen5.logo4, templogopath);

    configfile.close();
  }
}

// -------------------------------- SAVE CONFIG -------------------------------------

void saveconfig()
{
  if (server.arg("save") == "homescreen")
  {

    Serial.println("[INFO]: Saving Homescreen");
    FILESYSTEM.remove("/config/homescreen.json");
    File file = FILESYSTEM.open("/config/homescreen.json", "w");
    if (!file)
    {
      Serial.println("[WARNING]: Failed to create homescreen.json");
      return;
    }

    DynamicJsonDocument doc(256);

    JsonObject homescreen = doc.to<JsonObject>();

    homescreen["logo0"] = server.arg("homescreenlogo0");
    homescreen["logo1"] = server.arg("homescreenlogo1");
    homescreen["logo2"] = server.arg("homescreenlogo2");
    homescreen["logo3"] = server.arg("homescreenlogo3");
    homescreen["logo4"] = server.arg("homescreenlogo4");
    homescreen["logo5"] = server.arg("homescreenlogo5");

    if (serializeJsonPretty(doc, file) == 0)
    {
      Serial.println("[WARNING]: Failed to write to file");
    }
    file.close();
  }
  else if (server.arg("save") == "savecolors")
  {
    Serial.println("[INFO]: Saving Colors");

    FILESYSTEM.remove("/config/colors.json");
    File file = FILESYSTEM.open("/config/colors.json", "w");
    if (!file)
    {
      Serial.println("[WARNING]: Failed to create file");
      return;
    }

    DynamicJsonDocument doc(256);

    JsonObject colors = doc.to<JsonObject>();

    colors["menubuttoncolor"] = server.arg("menubuttoncolor");
    colors["functionbuttoncolor"] = server.arg("functionbuttoncolor");
    colors["latchcolor"] = server.arg("latchcolor");
    colors["background"] = server.arg("background");

    if (serializeJsonPretty(doc, file) == 0)
    {
      Serial.println("[WARNING]: Failed to write to file");
    }
    file.close();

    // ----------------- Saving menu 1 -------------------
  }
  else if (server.arg("save") == "menu1")
  {
    
    Serial.println("[INFO]: Saving Menu 1");
    FILESYSTEM.remove("/config/menu1.json");
    File file = FILESYSTEM.open("/config/menu1.json", "w");
    if (!file)
    {
      Serial.println("[WARNING]: Failed to create menu1.json");
      return;
    }

    DynamicJsonDocument doc(1200);

    JsonObject menu = doc.to<JsonObject>();

    menu["logo0"] = server.arg("screen1logo0");
    menu["logo1"] = server.arg("screen1logo1");
    menu["logo2"] = server.arg("screen1logo2");
    menu["logo3"] = server.arg("screen1logo3");
    menu["logo4"] = server.arg("screen1logo4");

    JsonObject button0 = doc.createNestedObject("button0");
    
    if(server.arg("screen1button0latch") == "on"){
      button0["latch"] = true;
    }else{
      button0["latch"] = false;
    }

    JsonArray button0_actionarray = button0.createNestedArray("actionarray");
    button0_actionarray.add(server.arg("screen1button0action0"));
    button0_actionarray.add(server.arg("screen1button0action1"));
    button0_actionarray.add(server.arg("screen1button0action2"));

    JsonArray button0_valuearray = button0.createNestedArray("valuearray");
    button0_valuearray.add(server.arg("screen1button0value0"));
    button0_valuearray.add(server.arg("screen1button0value1"));
    button0_valuearray.add(server.arg("screen1button0value2"));

    JsonObject button1 = doc.createNestedObject("button1");

    if(server.arg("screen1button1latch") == "on"){
      button1["latch"] = true;
    }else{
      button1["latch"] = false;
    }

    JsonArray button1_actionarray = button1.createNestedArray("actionarray");
    button1_actionarray.add(server.arg("screen1button1action0"));
    button1_actionarray.add(server.arg("screen1button1action1"));
    button1_actionarray.add(server.arg("screen1button1action2"));

    JsonArray button1_valuearray = button1.createNestedArray("valuearray");
    button1_valuearray.add(server.arg("screen1button1value0"));
    button1_valuearray.add(server.arg("screen1button1value1"));
    button1_valuearray.add(server.arg("screen1button1value2"));

    JsonObject button2 = doc.createNestedObject("button2");

    if(server.arg("screen1button2latch") == "on"){
      button2["latch"] = true;
    }else{
      button2["latch"] = false;
    }

    JsonArray button2_actionarray = button2.createNestedArray("actionarray");
    button2_actionarray.add(server.arg("screen1button2action0"));
    button2_actionarray.add(server.arg("screen1button2action1"));
    button2_actionarray.add(server.arg("screen1button2action2"));

    JsonArray button2_valuearray = button2.createNestedArray("valuearray");
    button2_valuearray.add(server.arg("screen1button2value0"));
    button2_valuearray.add(server.arg("screen1button2value1"));
    button2_valuearray.add(server.arg("screen1button2value2"));

    JsonObject button3 = doc.createNestedObject("button3");

    if(server.arg("screen1button3latch") == "on"){
      button3["latch"] = true;
    }else{
      button3["latch"] = false;
    }

    JsonArray button3_actionarray = button3.createNestedArray("actionarray");
    button3_actionarray.add(server.arg("screen1button3action0"));
    button3_actionarray.add(server.arg("screen1button3action1"));
    button3_actionarray.add(server.arg("screen1button3action2"));

    JsonArray button3_valuearray = button3.createNestedArray("valuearray");
    button3_valuearray.add(server.arg("screen1button3value0"));
    button3_valuearray.add(server.arg("screen1button3value1"));
    button3_valuearray.add(server.arg("screen1button3value2"));

    JsonObject button4 = doc.createNestedObject("button4");

    if(server.arg("screen1button4latch") == "on"){
      button4["latch"] = true;
    }else{
      button4["latch"] = false;
    }

    JsonArray button4_actionarray = button4.createNestedArray("actionarray");
    button4_actionarray.add(server.arg("screen1button4action0"));
    button4_actionarray.add(server.arg("screen1button4action1"));
    button4_actionarray.add(server.arg("screen1button4action2"));

    JsonArray button4_valuearray = button4.createNestedArray("valuearray");
    button4_valuearray.add(server.arg("screen1button4value0"));
    button4_valuearray.add(server.arg("screen1button4value1"));
    button4_valuearray.add(server.arg("screen1button4value2"));

    if (serializeJsonPretty(doc, file) == 0)
    {
      Serial.println("[WARNING]: Failed to write to file");
    }
    file.close();

    // ----------------- Saving menu 2 -------------------
  }
  else if (server.arg("save") == "menu2")
  {

    Serial.println("[INFO]: Saving Menu 2");
    FILESYSTEM.remove("/config/menu2.json");
    File file = FILESYSTEM.open("/config/menu2.json", "w");
    if (!file)
    {
      Serial.println("[WARNING]: Failed to create menu2.json");
      return;
    }

    DynamicJsonDocument doc(1200);

    JsonObject menu = doc.to<JsonObject>();

    menu["logo0"] = server.arg("screen2logo0");
    menu["logo1"] = server.arg("screen2logo1");
    menu["logo2"] = server.arg("screen2logo2");
    menu["logo3"] = server.arg("screen2logo3");
    menu["logo4"] = server.arg("screen2logo4");

    JsonObject button0 = doc.createNestedObject("button0");

    if(server.arg("screen2button0latch") == "on"){
      button0["latch"] = true;
    }else{
      button0["latch"] = false;
    }

    JsonArray button0_actionarray = button0.createNestedArray("actionarray");
    button0_actionarray.add(server.arg("screen2button0action0"));
    button0_actionarray.add(server.arg("screen2button0action1"));
    button0_actionarray.add(server.arg("screen2button0action2"));

    JsonArray button0_valuearray = button0.createNestedArray("valuearray");
    button0_valuearray.add(server.arg("screen2button0value0"));
    button0_valuearray.add(server.arg("screen2button0value1"));
    button0_valuearray.add(server.arg("screen2button0value2"));

    JsonObject button1 = doc.createNestedObject("button1");

    if(server.arg("screen2button1latch") == "on"){
      button1["latch"] = true;
    }else{
      button1["latch"] = false;
    }

    JsonArray button1_actionarray = button1.createNestedArray("actionarray");
    button1_actionarray.add(server.arg("screen2button1action0"));
    button1_actionarray.add(server.arg("screen2button1action1"));
    button1_actionarray.add(server.arg("screen2button1action2"));

    JsonArray button1_valuearray = button1.createNestedArray("valuearray");
    button1_valuearray.add(server.arg("screen2button1value0"));
    button1_valuearray.add(server.arg("screen2button1value1"));
    button1_valuearray.add(server.arg("screen2button1value2"));

    JsonObject button2 = doc.createNestedObject("button2");

    if(server.arg("screen2button2latch") == "on"){
      button2["latch"] = true;
    }else{
      button2["latch"] = false;
    }

    JsonArray button2_actionarray = button2.createNestedArray("actionarray");
    button2_actionarray.add(server.arg("screen2button2action0"));
    button2_actionarray.add(server.arg("screen2button2action1"));
    button2_actionarray.add(server.arg("screen2button2action2"));

    JsonArray button2_valuearray = button2.createNestedArray("valuearray");
    button2_valuearray.add(server.arg("screen2button2value0"));
    button2_valuearray.add(server.arg("screen2button2value1"));
    button2_valuearray.add(server.arg("screen2button2value2"));

    JsonObject button3 = doc.createNestedObject("button3");

    if(server.arg("screen2button3latch") == "on"){
      button3["latch"] = true;
    }else{
      button3["latch"] = false;
    }

    JsonArray button3_actionarray = button3.createNestedArray("actionarray");
    button3_actionarray.add(server.arg("screen2button3action0"));
    button3_actionarray.add(server.arg("screen2button3action1"));
    button3_actionarray.add(server.arg("screen2button3action2"));

    JsonArray button3_valuearray = button3.createNestedArray("valuearray");
    button3_valuearray.add(server.arg("screen2button3value0"));
    button3_valuearray.add(server.arg("screen2button3value1"));
    button3_valuearray.add(server.arg("screen2button3value2"));

    JsonObject button4 = doc.createNestedObject("button4");

    if(server.arg("screen2button4latch") == "on"){
      button4["latch"] = true;
    }else{
      button4["latch"] = false;
    }

    JsonArray button4_actionarray = button4.createNestedArray("actionarray");
    button4_actionarray.add(server.arg("screen2button4action0"));
    button4_actionarray.add(server.arg("screen2button4action1"));
    button4_actionarray.add(server.arg("screen2button4action2"));

    JsonArray button4_valuearray = button4.createNestedArray("valuearray");
    button4_valuearray.add(server.arg("screen2button4value0"));
    button4_valuearray.add(server.arg("screen2button4value1"));
    button4_valuearray.add(server.arg("screen2button4value2"));

    if (serializeJsonPretty(doc, file) == 0)
    {
      Serial.println("[WARNING]: Failed to write to file");
    }
    file.close();

    // ----------------- Saving menu 3 -------------------
  }
  else if (server.arg("save") == "menu3")
  {

    Serial.println("[INFO]: Saving Menu 3");
    FILESYSTEM.remove("/config/menu3.json");
    File file = FILESYSTEM.open("/config/menu3.json", "w");
    if (!file)
    {
      Serial.println("[WARNING]: Failed to create menu3.json");
      return;
    }

    DynamicJsonDocument doc(1200);

    JsonObject menu = doc.to<JsonObject>();

    menu["logo0"] = server.arg("screen3logo0");
    menu["logo1"] = server.arg("screen3logo1");
    menu["logo2"] = server.arg("screen3logo2");
    menu["logo3"] = server.arg("screen3logo3");
    menu["logo4"] = server.arg("screen3logo4");

    JsonObject button0 = doc.createNestedObject("button0");

    if(server.arg("screen3button0latch") == "on"){
      button0["latch"] = true;
    }else{
      button0["latch"] = false;
    }

    JsonArray button0_actionarray = button0.createNestedArray("actionarray");
    button0_actionarray.add(server.arg("screen3button0action0"));
    button0_actionarray.add(server.arg("screen3button0action1"));
    button0_actionarray.add(server.arg("screen3button0action2"));

    JsonArray button0_valuearray = button0.createNestedArray("valuearray");
    button0_valuearray.add(server.arg("screen3button0value0"));
    button0_valuearray.add(server.arg("screen3button0value1"));
    button0_valuearray.add(server.arg("screen3button0value2"));

    JsonObject button1 = doc.createNestedObject("button1");

    if(server.arg("screen3button1latch") == "on"){
      button1["latch"] = true;
    }else{
      button1["latch"] = false;
    }

    JsonArray button1_actionarray = button1.createNestedArray("actionarray");
    button1_actionarray.add(server.arg("screen3button1action0"));
    button1_actionarray.add(server.arg("screen3button1action1"));
    button1_actionarray.add(server.arg("screen3button1action2"));

    JsonArray button1_valuearray = button1.createNestedArray("valuearray");
    button1_valuearray.add(server.arg("screen3button1value0"));
    button1_valuearray.add(server.arg("screen3button1value1"));
    button1_valuearray.add(server.arg("screen3button1value2"));

    JsonObject button2 = doc.createNestedObject("button2");
    
    if(server.arg("screen3button2latch") == "on"){
      button2["latch"] = true;
    }else{
      button2["latch"] = false;
    }

    JsonArray button2_actionarray = button2.createNestedArray("actionarray");
    button2_actionarray.add(server.arg("screen3button2action0"));
    button2_actionarray.add(server.arg("screen3button2action1"));
    button2_actionarray.add(server.arg("screen3button2action2"));

    JsonArray button2_valuearray = button2.createNestedArray("valuearray");
    button2_valuearray.add(server.arg("screen3button2value0"));
    button2_valuearray.add(server.arg("screen3button2value1"));
    button2_valuearray.add(server.arg("screen3button2value2"));

    JsonObject button3 = doc.createNestedObject("button3");

    if(server.arg("screen3button3latch") == "on"){
      button3["latch"] = true;
    }else{
      button0["latch"] = false;
    }

    JsonArray button3_actionarray = button3.createNestedArray("actionarray");
    button3_actionarray.add(server.arg("screen3button3action0"));
    button3_actionarray.add(server.arg("screen3button3action1"));
    button3_actionarray.add(server.arg("screen3button3action2"));

    JsonArray button3_valuearray = button3.createNestedArray("valuearray");
    button3_valuearray.add(server.arg("screen3button3value0"));
    button3_valuearray.add(server.arg("screen3button3value1"));
    button3_valuearray.add(server.arg("screen3button3value2"));

    JsonObject button4 = doc.createNestedObject("button4");

    if(server.arg("screen3button4latch") == "on"){
      button4["latch"] = true;
    }else{
      button4["latch"] = false;
    }

    JsonArray button4_actionarray = button4.createNestedArray("actionarray");
    button4_actionarray.add(server.arg("screen3button4action0"));
    button4_actionarray.add(server.arg("screen3button4action1"));
    button4_actionarray.add(server.arg("screen3button4action2"));

    JsonArray button4_valuearray = button4.createNestedArray("valuearray");
    button4_valuearray.add(server.arg("screen3button4value0"));
    button4_valuearray.add(server.arg("screen3button4value1"));
    button4_valuearray.add(server.arg("screen3button4value2"));

    if (serializeJsonPretty(doc, file) == 0)
    {
      Serial.println("[WARNING]: Failed to write to file");
    }
    file.close();

    // ----------------- Saving menu 4 -------------------
  }
  else if (server.arg("save") == "menu4")
  {

    Serial.println("[INFO]: Saving Menu 4");
    FILESYSTEM.remove("/config/menu4.json");
    File file = FILESYSTEM.open("/config/menu4.json", "w");
    if (!file)
    {
      Serial.println("[WARNING]: Failed to create menu4.json");
      return;
    }

    DynamicJsonDocument doc(1200);

    JsonObject menu = doc.to<JsonObject>();

    menu["logo0"] = server.arg("screen4logo0");
    menu["logo1"] = server.arg("screen4logo1");
    menu["logo2"] = server.arg("screen4logo2");
    menu["logo3"] = server.arg("screen4logo3");
    menu["logo4"] = server.arg("screen4logo4");

    JsonObject button0 = doc.createNestedObject("button0");

    if(server.arg("screen4button0latch") == "on"){
      button0["latch"] = true;
    }else{
      button0["latch"] = false;
    }

    JsonArray button0_actionarray = button0.createNestedArray("actionarray");
    button0_actionarray.add(server.arg("screen4button0action0"));
    button0_actionarray.add(server.arg("screen4button0action1"));
    button0_actionarray.add(server.arg("screen4button0action2"));

    JsonArray button0_valuearray = button0.createNestedArray("valuearray");
    button0_valuearray.add(server.arg("screen4button0value0"));
    button0_valuearray.add(server.arg("screen4button0value1"));
    button0_valuearray.add(server.arg("screen4button0value2"));

    JsonObject button1 = doc.createNestedObject("button1");

    if(server.arg("screen4button1latch") == "on"){
      button1["latch"] = true;
    }else{
      button1["latch"] = false;
    }

    JsonArray button1_actionarray = button1.createNestedArray("actionarray");
    button1_actionarray.add(server.arg("screen4button1action0"));
    button1_actionarray.add(server.arg("screen4button1action1"));
    button1_actionarray.add(server.arg("screen4button1action2"));

    JsonArray button1_valuearray = button1.createNestedArray("valuearray");
    button1_valuearray.add(server.arg("screen4button1value0"));
    button1_valuearray.add(server.arg("screen4button1value1"));
    button1_valuearray.add(server.arg("screen4button1value2"));

    JsonObject button2 = doc.createNestedObject("button2");

    if(server.arg("screen4button2latch") == "on"){
      button2["latch"] = true;
    }else{
      button2["latch"] = false;
    }

    JsonArray button2_actionarray = button2.createNestedArray("actionarray");
    button2_actionarray.add(server.arg("screen4button2action0"));
    button2_actionarray.add(server.arg("screen4button2action1"));
    button2_actionarray.add(server.arg("screen4button2action2"));

    JsonArray button2_valuearray = button2.createNestedArray("valuearray");
    button2_valuearray.add(server.arg("screen4button2value0"));
    button2_valuearray.add(server.arg("screen4button2value1"));
    button2_valuearray.add(server.arg("screen4button2value2"));

    JsonObject button3 = doc.createNestedObject("button3");

    if(server.arg("screen4button3latch") == "on"){
      button3["latch"] = true;
    }else{
      button3["latch"] = false;
    }

    JsonArray button3_actionarray = button3.createNestedArray("actionarray");
    button3_actionarray.add(server.arg("screen4button3action0"));
    button3_actionarray.add(server.arg("screen4button3action1"));
    button3_actionarray.add(server.arg("screen4button3action2"));

    JsonArray button3_valuearray = button3.createNestedArray("valuearray");
    button3_valuearray.add(server.arg("screen4button3value0"));
    button3_valuearray.add(server.arg("screen4button3value1"));
    button3_valuearray.add(server.arg("screen4button3value2"));

    JsonObject button4 = doc.createNestedObject("button4");

    if(server.arg("screen4button4latch") == "on"){
      button4["latch"] = true;
    }else{
      button4["latch"] = false;
    }

    JsonArray button4_actionarray = button4.createNestedArray("actionarray");
    button4_actionarray.add(server.arg("screen4button4action0"));
    button4_actionarray.add(server.arg("screen4button4action1"));
    button4_actionarray.add(server.arg("screen4button4action2"));

    JsonArray button4_valuearray = button4.createNestedArray("valuearray");
    button4_valuearray.add(server.arg("screen4button4value0"));
    button4_valuearray.add(server.arg("screen4button4value1"));
    button4_valuearray.add(server.arg("screen4button4value2"));

    if (serializeJsonPretty(doc, file) == 0)
    {
      Serial.println("[WARNING]: Failed to write to file");
    }
    file.close();

    // ----------------- Saving menu 5 -------------------
  }
  else if (server.arg("save") == "menu5")
  {

    Serial.println("[INFO]: Saving Menu 5");
    FILESYSTEM.remove("/config/menu5.json");
    File file = FILESYSTEM.open("/config/menu5.json", "w");
    if (!file)
    {
      Serial.println("[WARNING]: Failed to create menu5.json");
      return;
    }

    DynamicJsonDocument doc(1200);

    JsonObject menu = doc.to<JsonObject>();

    menu["logo0"] = server.arg("screen5logo0");
    menu["logo1"] = server.arg("screen5logo1");
    menu["logo2"] = server.arg("screen5logo2");
    menu["logo3"] = server.arg("screen5logo3");
    menu["logo4"] = server.arg("screen5logo4");

    JsonObject button0 = doc.createNestedObject("button0");

    if(server.arg("screen5button0latch") == "on"){
      button0["latch"] = true;
    }else{
      button0["latch"] = false;
    }

    JsonArray button0_actionarray = button0.createNestedArray("actionarray");
    button0_actionarray.add(server.arg("screen5button0action0"));
    button0_actionarray.add(server.arg("screen5button0action1"));
    button0_actionarray.add(server.arg("screen5button0action2"));

    JsonArray button0_valuearray = button0.createNestedArray("valuearray");
    button0_valuearray.add(server.arg("screen5button0value0"));
    button0_valuearray.add(server.arg("screen5button0value1"));
    button0_valuearray.add(server.arg("screen5button0value2"));

    JsonObject button1 = doc.createNestedObject("button1");

    if(server.arg("screen5button1latch") == "on"){
      button1["latch"] = true;
    }else{
      button1["latch"] = false;
    }

    JsonArray button1_actionarray = button1.createNestedArray("actionarray");
    button1_actionarray.add(server.arg("screen5button1action0"));
    button1_actionarray.add(server.arg("screen5button1action1"));
    button1_actionarray.add(server.arg("screen5button1action2"));

    JsonArray button1_valuearray = button1.createNestedArray("valuearray");
    button1_valuearray.add(server.arg("screen5button1value0"));
    button1_valuearray.add(server.arg("screen5button1value1"));
    button1_valuearray.add(server.arg("screen5button1value2"));

    JsonObject button2 = doc.createNestedObject("button2");

    if(server.arg("screen5button2latch") == "on"){
      button2["latch"] = true;
    }else{
      button2["latch"] = false;
    }

    JsonArray button2_actionarray = button2.createNestedArray("actionarray");
    button2_actionarray.add(server.arg("screen5button2action0"));
    button2_actionarray.add(server.arg("screen5button2action1"));
    button2_actionarray.add(server.arg("screen5button2action2"));

    JsonArray button2_valuearray = button2.createNestedArray("valuearray");
    button2_valuearray.add(server.arg("screen5button2value0"));
    button2_valuearray.add(server.arg("screen5button2value1"));
    button2_valuearray.add(server.arg("screen5button2value2"));

    JsonObject button3 = doc.createNestedObject("button3");

    if(server.arg("screen5button3latch") == "on"){
      button3["latch"] = true;
    }else{
      button3["latch"] = false;
    }

    JsonArray button3_actionarray = button3.createNestedArray("actionarray");
    button3_actionarray.add(server.arg("screen5button3action0"));
    button3_actionarray.add(server.arg("screen5button3action1"));
    button3_actionarray.add(server.arg("screen5button3action2"));

    JsonArray button3_valuearray = button3.createNestedArray("valuearray");
    button3_valuearray.add(server.arg("screen5button3value0"));
    button3_valuearray.add(server.arg("screen5button3value1"));
    button3_valuearray.add(server.arg("screen5button3value2"));

    JsonObject button4 = doc.createNestedObject("button4");
    
    if(server.arg("screen5button4latch") == "on"){
      button4["latch"] = true;
    }else{
      button4["latch"] = false;
    }

    JsonArray button4_actionarray = button4.createNestedArray("actionarray");
    button4_actionarray.add(server.arg("screen5button4action0"));
    button4_actionarray.add(server.arg("screen5button4action1"));
    button4_actionarray.add(server.arg("screen5button4action2"));

    JsonArray button4_valuearray = button4.createNestedArray("valuearray");
    button4_valuearray.add(server.arg("screen5button4value0"));
    button4_valuearray.add(server.arg("screen5button4value1"));
    button4_valuearray.add(server.arg("screen5button4value2"));

    if (serializeJsonPretty(doc, file) == 0)
    {
      Serial.println("[WARNING]: Failed to write to file");
    }
    file.close();
  }

  // Redirect page
  handleFileRead("/saveconfig.htm");
}

//----------------------------- Draw Error Message ------------------------------------------

void drawErrorMessage(String message)
{

  tft.fillScreen(TFT_BLACK);
  tft.setCursor(20, 20);
  tft.setTextFont(2);
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.println(message);
}

// --------------------------------- WIFI Stuff ----------------------------------------

void configmode()
{

  Serial.println("[INFO]: Entering Config Mode");
  
  // Stop BLE from interfering with our WIFI signal
  btStop();
  esp_bt_controller_disable();
  esp_bt_controller_deinit();
  esp_bt_controller_mem_release(ESP_BT_MODE_IDLE);
  Serial.println("[INFO]: BLE Stopped");

  if(String(wificonfig.ssid) == "YOUR_WIFI_SSID" || String(wificonfig.password) == "YOUR_WIFI_PASSWORD") // Still default
  { 
    drawErrorMessage("WiFi Config still set to default!");
    Serial.println("[ERROR]: WiFi Config still set to default!");
    while (1)
      yield(); // Stop!
  }

  if(String(wificonfig.ssid) == "FAILED" || String(wificonfig.password) == "FAILED") // The wificonfig.json failed to load
  { 
    drawErrorMessage("WiFi Config Failed to load!");
    Serial.println("[ERROR]: WiFi Config Failed to load!");
    while (1)
      yield(); // Stop!
  }

  drawErrorMessage("Connecting to Wifi...");
  
  Serial.printf("[INFO]: Connecting to %s", wificonfig.ssid);
  if (String(WiFi.SSID()) != String(wificonfig.ssid))
  {
    WiFi.mode(WIFI_STA);
    WiFi.begin(wificonfig.ssid, wificonfig.password);
  }

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("[INFO]: Connected! IP address: ");
  Serial.println(WiFi.localIP());

  MDNS.begin(wificonfig.hostname);

  // Set pageNum to 7 so no buttons are displayed and touches are ignored
  pageNum = 7;

  // Start the webserver
  server.begin();
  Serial.println("[INFO]: Webserver started");
}

void handleRestart()
{

  // First send some text to the browser otherwise an ugly browser error shows up
  server.send(200, "text/plain", "FreeTouchDeck is restarting...");
  // Then restart the ESP
  Serial.println("[WARNING]: Restarting");
  ESP.restart();
}

//----------- TOUCH Calibration -------------------------------------------------------------------------------


// If USECAPTOUCH is defined we do not need to calibrate touch

void touch_calibrate()
{
  uint16_t calData[5];
  uint8_t calDataOK = 0;

//  // check file system exists
//  if (!FILESYSTEM.begin())
//  {
//    Serial.println("[WARNING]: Formatting file system");
//    FILESYSTEM.format();
//    FILESYSTEM.begin();
//  }

  // check if calibration file exists and size is correct
  if (FILESYSTEM.exists(CALIBRATION_FILE))
  {
    if (REPEAT_CAL)
    {
      // Delete if we want to re-calibrate
      FILESYSTEM.remove(CALIBRATION_FILE);
    }
    else
    {
      File f = FILESYSTEM.open(CALIBRATION_FILE, "r");
      if (f)
      {
        if (f.readBytes((char *)calData, 14) == 14)
          calDataOK = 1;
        f.close();
      }
    }
  }

  if (calDataOK && !REPEAT_CAL)
  {
    // calibration data valid
    tft.setTouch(calData);
  }
  else
  {
    // data not valid so recalibrate
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(20, 0);
    tft.setTextFont(2);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    tft.println("Touch corners as indicated");

    tft.setTextFont(1);
    tft.println();

    if (REPEAT_CAL)
    {
      tft.setTextColor(TFT_RED, TFT_BLACK);
      tft.println("Set REPEAT_CAL to false to stop this running again!");
    }

    tft.calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);

    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.println("Calibration complete!");

    // store data
    File f = FILESYSTEM.open(CALIBRATION_FILE, "w");
    if (f)
    {
      f.write((const unsigned char *)calData, 14);
      f.close();
    }
  }
}


//----------- Functions used by Webserver -------------------------------------------------------------------------------

String getContentType(String filename)
{
  if (server.hasArg("download"))
  {
    return "application/octet-stream";
  }
  else if (filename.endsWith(".htm"))
  {
    return "text/html";
  }
  else if (filename.endsWith(".html"))
  {
    return "text/html";
  }
  else if (filename.endsWith(".css"))
  {
    return "text/css";
  }
  else if (filename.endsWith(".js"))
  {
    return "application/javascript";
  }
  else if (filename.endsWith(".png"))
  {
    return "image/png";
  }
  else if (filename.endsWith(".gif"))
  {
    return "image/gif";
  }
  else if (filename.endsWith(".jpg"))
  {
    return "image/jpeg";
  }
  else if (filename.endsWith(".ico"))
  {
    return "image/x-icon";
  }
  else if (filename.endsWith(".xml"))
  {
    return "text/xml";
  }
  else if (filename.endsWith(".pdf"))
  {
    return "application/x-pdf";
  }
  else if (filename.endsWith(".zip"))
  {
    return "application/x-zip";
  }
  else if (filename.endsWith(".gz"))
  {
    return "application/x-gzip";
  }
  else if (filename.endsWith(".json"))
  {
    return "application/json";
  }
  else if (filename.endsWith(".bmp"))
  {
    return "image/bmp";
  }
  return "text/plain";
}

bool exists(String path)
{
  bool yes = false;
  File file = FILESYSTEM.open(path, "r");
  if (!file.isDirectory())
  {
    yes = true;
  }
  file.close();
  return yes;
}

bool handleFileRead(String path)
{
  Serial.println("[INFO]: Handeling file read");
  Serial.println("handleFileRead: " + path);
  if (path.endsWith("/"))
  {
    path += "index.htm";
  }
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  if (exists(pathWithGz) || exists(path))
  {
    if (exists(pathWithGz))
    {
      path += ".gz";
    }
    File file = FILESYSTEM.open(path, "r");
    server.streamFile(file, contentType);
    file.close();
    return true;
  }
  return false;
}

void faviconhandle()
{

  File file = FILESYSTEM.open("/favicon.ico", "r");
  server.streamFile(file, "image/x-icon");
  file.close();
}

void handleFileList()
{
  Serial.println("[INFO]: Handeling file list");
  if (!server.hasArg("dir"))
  {
    server.send(500, "text/plain", "BAD ARGS");
    return;
  }

  String path = server.arg("dir");
  Serial.println("handleFileList: " + path);

  File root = FILESYSTEM.open(path);
  path = String();
  int filecount = 0;

  String output = "[";
  if (root.isDirectory())
  {
    File file = root.openNextFile();
    while (file)
    {
      if (output != "[")
      {
        output += ',';
      }

      output += "{\"";
      output += filecount;
      output += "\":\"";
      output += String(file.name()).substring(7);
      output += "\"}";
      file = root.openNextFile();
      filecount++;
    }

    file.close();
  }
  output += "]";
  server.send(200, "application/json", output);

  root.close();
}

// ----- Handle File Upload -------

void handleFileUpload()
{
  Serial.println("[INFO]: Handeling file upload");
  if (server.uri() != "/upload")
  {
    return;
  }
  HTTPUpload &upload = server.upload();
  if (upload.status == UPLOAD_FILE_START)
  {
    String filename = upload.filename;
    if (!filename.startsWith("/"))
    {
      filename = "/logos/" + filename; // TODO: This should not be hard-coded!
    }
    Serial.println("[INFO]: File uploading");
    fsUploadFile = FILESYSTEM.open(filename, "w");
    filename = String();
  }
  else if (upload.status == UPLOAD_FILE_WRITE)
  {
    if (fsUploadFile)
    {
      fsUploadFile.write(upload.buf, upload.currentSize);
    }
  }
  else if (upload.status == UPLOAD_FILE_END)
  {
    if (fsUploadFile)
    {
      fsUploadFile.close();
      //handleFileRead("/upload.htm");
    }
    Serial.println("[INFO]: File Uploaded");
    handleFileRead("/upload.htm");
  }
}

//-------------------- HTML color to RGB888 conversion ------------------------

unsigned long convertHTMLtoRGB888(char *html)
{
  char *hex = html + 1; // remove the #
  unsigned long rgb = strtoul(hex, NULL, 16);
  return rgb;
}

//-------------------- RGB888 to RGB565 conversion ------------------------

unsigned int convertRGB888ToRGB565(unsigned long rgb)
{
  return (((rgb & 0xf80000) >> 8) | ((rgb & 0xfc00) >> 5) | ((rgb & 0xf8) >> 3));
}

// ----------------- BMP Drawing ----------------------------------------

void drawBmpTransparent(const char *filename, int16_t x, int16_t y)
{

  if ((x >= tft.width()) || (y >= tft.height()))
    return;

  fs::File bmpFS;

  bmpFS = FILESYSTEM.open(filename, "r");

  if (!bmpFS)
  {
    Serial.println("[WARNING]: Bitmap not found: ");
    Serial.println(filename);
    filename = "/logos/question.bmp";
    bmpFS = FILESYSTEM.open(filename, "r");
  }

  uint32_t seekOffset;
  uint16_t w, h, row, col;
  uint8_t r, g, b;

  uint32_t startTime = millis();

  if (read16(bmpFS) == 0x4D42)
  {
    read32(bmpFS);
    read32(bmpFS);
    seekOffset = read32(bmpFS);
    read32(bmpFS);
    w = read32(bmpFS);
    h = read32(bmpFS);

    if ((read16(bmpFS) == 1) && (read16(bmpFS) == 24) && (read32(bmpFS) == 0))
    {
      y += h - 1;

      bool oldSwapBytes = tft.getSwapBytes();
      tft.setSwapBytes(true);
      bmpFS.seek(seekOffset);

      uint16_t padding = (4 - ((w * 3) & 3)) & 3;
      uint8_t lineBuffer[w * 3 + padding];

      for (row = 0; row < h; row++)
      {

        bmpFS.read(lineBuffer, sizeof(lineBuffer));
        uint8_t *bptr = lineBuffer;
        uint16_t *tptr = (uint16_t *)lineBuffer;
        // Convert 24 to 16 bit colours
        for (uint16_t col = 0; col < w; col++)
        {
          b = *bptr++;
          g = *bptr++;
          r = *bptr++;
          *tptr++ = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
        }

        // Push the pixel row to screen, pushImage will crop the line if needed
        // y is decremented as the BMP image is drawn bottom up
        tft.pushImage(x, y--, w, 1, (uint16_t *)lineBuffer, TFT_BLACK);
      }
      tft.setSwapBytes(oldSwapBytes);
    }
    else
      Serial.println("BMP format not recognized.");
  }
  bmpFS.close();
}

void drawBmp(const char *filename, int16_t x, int16_t y)
{

  if ((x >= tft.width()) || (y >= tft.height()))
    return;

  fs::File bmpFS;

  bmpFS = FILESYSTEM.open(filename, "r");

  if (!bmpFS)
  {
    Serial.print("File not found");
    return;
  }

  uint32_t seekOffset;
  uint16_t w, h, row, col;
  uint8_t r, g, b;

  uint32_t startTime = millis();

  if (read16(bmpFS) == 0x4D42)
  {
    read32(bmpFS);
    read32(bmpFS);
    seekOffset = read32(bmpFS);
    read32(bmpFS);
    w = read32(bmpFS);
    h = read32(bmpFS);

    if ((read16(bmpFS) == 1) && (read16(bmpFS) == 24) && (read32(bmpFS) == 0))
    {
      y += h - 1;

      bool oldSwapBytes = tft.getSwapBytes();
      tft.setSwapBytes(true);
      bmpFS.seek(seekOffset);

      uint16_t padding = (4 - ((w * 3) & 3)) & 3;
      uint8_t lineBuffer[w * 3 + padding];

      for (row = 0; row < h; row++)
      {

        bmpFS.read(lineBuffer, sizeof(lineBuffer));
        uint8_t *bptr = lineBuffer;
        uint16_t *tptr = (uint16_t *)lineBuffer;
        // Convert 24 to 16 bit colours
        for (uint16_t col = 0; col < w; col++)
        {
          b = *bptr++;
          g = *bptr++;
          r = *bptr++;
          *tptr++ = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
        }

        // Push the pixel row to screen, pushImage will crop the line if needed
        // y is decremented as the BMP image is drawn bottom up
        tft.pushImage(x, y--, w, 1, (uint16_t *)lineBuffer);
      }
      tft.setSwapBytes(oldSwapBytes);
    }
    else
      Serial.println("[WARNING]: BMP format not recognized.");
  }
  bmpFS.close();
}

uint16_t read16(fs::File &f)
{
  uint16_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read(); // MSB
  return result;
}

uint32_t read32(fs::File &f)
{
  uint32_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read();
  ((uint8_t *)&result)[2] = f.read();
  ((uint8_t *)&result)[3] = f.read(); // MSB
  return result;
}

/* ------------------------ Check if config file exists function ---------------- 
Purpose: This function checks if a file exists and returns a boolean accordingly.
         It then prints a debug message to the serial as wel as the tft.
Input  : char *filename
Output : boolean
Note   : Pass the filename including a leading /
*/

bool checkfile(char *filename)
{

  if (!FILESYSTEM.exists(filename))
  {
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(1, 3);
    tft.setTextFont(2);
    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.printf("%s not found!\n\n", filename);
    tft.setTextSize(1);
    tft.printf("If this has happend after confguration, the data on the ESP may \nbe corrupted.");
  return false;
  }
}
