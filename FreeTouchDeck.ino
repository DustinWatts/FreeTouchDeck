/*
  FreeTouchDeck (secret codename) based on the FreeDeck idea by Koriwi.
  It uses the TFT_eSPI library by Bodmer for the display and touch functionality and
  the ESP32-BLE-Keyboard library by T-vK. For loading configuration it uses 
  ArduinoJson V6.

  The following libraries must be installed:
  
  - https://github.com/adafruit/Adafruit-GFX-Library (also available trough Library Manager)
  - https://github.com/Bodmer/TFT_eSPI
  - https://github.com/T-vK/ESP32-BLE-Keyboard
  - https://github.com/bblanchon/ArduinoJson (also available trough Library Manager)

  As this is an early Pre-alpha version, the code is ugly and sometimes way more complicated
  then necessary. It also lacks good documentation and comments in the code.

  tft_config.h holds the configuration for the TFT display and the touch controller. This is where
  SPI pins are defined. Uncomment the lines needed for your display.

  The SPIFFS (FLASH filing system) is used to hold touch screen calibration data.
  It has to be runs at least once. After that you can set REPEAT_CAL to false.
  If you change the screen rotation, you need to run calibration data again.
  
*/

#include <pgmspace.h>  // PROGMEM support header

#include "FS.h" // File System header

#include "tft_config.h" // Configuration data for TFT_eSPI

#define USER_SETUP_LOADED // Let TFT_eSPI know that we have our own config file

#include <SPI.h>
#include <TFT_eSPI.h>
#include <BleKeyboard.h>
#include <ArduinoJson.h>

#include "BLEDevice.h"
#include "BLEUtils.h"
#include "BLEBeacon.h"
#include "esp_sleep.h"

#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>

const char* ssid = " ";
const char* password = " ";
const char* host = "FreeTouchDeck";

WebServer server(80);

BleKeyboard bleKeyboard("FreeTouchDeck", "Made by me");

TFT_eSPI tft = TFT_eSPI();

// This is the file name used to store the calibration data
// You can change this to create new calibration files.
// The SPIFFS file name must start with "/".
#define CALIBRATION_FILE "/TouchCalData"

// Set REPEAT_CAL to true instead of false to run calibration
// again, otherwise it will only be done once.
// Repeat calibration if you change the screen rotation.
#define REPEAT_CAL false

// Keypad start position, key sizes and spacing
#define KEY_X 80 // Centre of button
#define KEY_Y 80
#define KEY_W 140 // Width and height of a button
#define KEY_H 140
#define KEY_SPACING_X 20 // Gaps between buttons
#define KEY_SPACING_Y 20
#define KEY_TEXTSIZE 1   // Font size multiplier

#define LABEL_FONT &FreeSansBold12pt7b    // Text Button Label Font

#define logoWidth  75  // logo width
#define logoHeight 75  // logo height
                         
int pageNum = 0; // placeholder for the pagenumber we are on (0 indicates home)

uint8_t rowArray[6] = {0,0,0,1,1,1}; // Every button has a row associated with it
uint8_t colArray[6] = {0,1,2,0,1,2}; // Every button has a column associated with it

char logopath[64] = "/logos/"; //path to the directory the logo are in ! including leading AND trailing / !
char templogopath[64] = "";

struct Logos {
  char logo0[64];
  char logo1[64];
  char logo2[64];
  char logo3[64];
  char logo4[64];
  char logo5[64];
};

struct Generallogos {
  char homebutton[64];
};

struct Config {
  uint16_t menuButtonColour;
  uint16_t functionButtonColour;
  uint16_t logoColour;
  uint16_t backgroundColour;

};

Config generalconfig;
Generallogos generallogo;
Logos screen0;
Logos screen1; 
Logos screen2; 
Logos screen3; 
Logos screen4; 
Logos screen5;
Logos screen6; 

// Invoke the TFT_eSPI button class and create all the button objects
TFT_eSPI_Button key[6];

//------------------------------------------------------------------------------------------

void setup() {
  
  // Use serial port
  Serial.begin(9600);

  //HTML to RGB test...

  char* html = "%2361dfff";
  Serial.println(html);
  unsigned long rgb888 = convertHTMLtoRGB888(html);
  Serial.println(rgb888);
  unsigned int rgb565 = convertRGB888ToRGB565(rgb888);
  Serial.println(rgb565);

  if (!SPIFFS.begin()) {
    Serial.println("SPIFFS initialisation failed!");
    while (1) yield(); // Stay here twiddling thumbs waiting
  }
  Serial.println("\r\nSPIFFS initialised.");

//------------------TFT/Touch Initialization ------------------------------------------------------------------------
    
  // Initialise the TFT screen
  tft.init();

  // Set the rotation before we calibrate
  tft.setRotation(1);

  // Clear the screen
  tft.fillScreen(TFT_BLACK);

  // Draw a spalsh screen
  drawBmp("/freetouchdeck_logo.bmp", 0, 0);
  tft.setCursor(1, 3);
  tft.setTextFont(2);
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.print("Loading version 0.8.2");
  delay(1000);
  
  
  // Calibrate the touch screen and retrieve the scaling factors
  touch_calibrate();

  // Load the General Config
  loadGeneralConfig();

  // Setup the Font used for plain text
  tft.setFreeFont(LABEL_FONT);

//------------------WIFI Initialization ------------------------------------------------------------------------

  Serial.printf("Connecting to %s\n", ssid);
  if (String(WiFi.SSID()) != String(ssid)) {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
  }

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());

  MDNS.begin(host);

//------------------Webserver Initialization ------------------------------------------------------------------------

  // Restart handle
  server.on("/restart", HTTP_POST, handleRestart);

  //called when the url is not defined here
  //use it to load content from SPIFFS
  server.onNotFound([]() {
    if (!handleFileRead(server.uri())) {
      server.send(404, "text/plain", "FileNotFound");
    }
  });

  // Save config handle
  server.on("/saveconfig.htm", HTTP_POST, saveconfig);

  // Favicon Handle
  server.on("/favicon.ico", HTTP_GET, faviconhandle);

  server.on("/list", HTTP_GET, handleFileList);

  // Draw background
  tft.fillScreen(generalconfig.backgroundColour);

  // Draw keypad
  drawKeypad();
  
//------------------BLE Initialization ------------------------------------------------------------------------

  bleKeyboard.begin();

}

//------------------------------------------------------------------------------------------

void loop(void) {
  
  if(pageNum == 7){

  // If the pageNum is set to 7, do no draw anything on screen or check for touch
  server.handleClient();

  }else{
  
  // Touch coordinates are stored here
  uint16_t t_x = 0, t_y = 0; 

  // Pressed will be set true is there is a valid touch on the screen
  boolean pressed = tft.getTouch(&t_x, &t_y);

  // Check if any key coordinate boxes contain the touch coordinates
  for (uint8_t b = 0; b < 6; b++) {
    if (pressed && key[b].contains(t_x, t_y)) {
      key[b].press(true);  // tell the button it is pressed
    } else {
      key[b].press(false);  // tell the button it is NOT pressed
    }
  }

  // Check if any key has changed state
  for (uint8_t b = 0; b < 6; b++) {

    if (key[b].justReleased()) {

        // Draw normal button space (non inverted)
        key[b].drawButton();     
        // Call the drawLogo function with generalconfig.logoColour foreground (normal state)
        drawlogo(b,colArray[b],rowArray[b],generalconfig.logoColour);
    }

    if (key[b].justPressed()) {
        key[b].drawButton(true);  // Draw inverted button space (white)
        if(b == 5 | pageNum == 0){
        // Call the drawLogo function with menuButtonColour foreground (inverse state)
        drawlogo(b,colArray[b],rowArray[b],generalconfig.menuButtonColour);
        }else{
        // Call the drawLogo function with functionButtonColour foreground (inverse state)
        drawlogo(b,colArray[b],rowArray[b],generalconfig.functionButtonColour);
        }

//----------- Button press handeling -------------------------------------------------------------------------------
      
      if (b == 0) {
          //Button 0 functions
        if(pageNum == 0){
          //Page 0 button 0 function
          pageNum = 1;    // By setting pageNum to 1
          drawKeypad();   // and calling drawKeypad() a new keypad is drawn with pageNum 1
        }else if(pageNum == 1){
          //Page 1 button 0 function
          bleKeyboard.write(KEY_MEDIA_MUTE);
        }else if(pageNum == 2){
          //Page 2 button 0 function
          bleKeyboard.write(KEY_MEDIA_MUTE); 
        }else if(pageNum == 3){
          //Page 3 button 0 function
          bleKeyboard.press(KEY_LEFT_CTRL);
          bleKeyboard.print("t");
          delay(50);
          bleKeyboard.releaseAll();
        }else if(pageNum == 4){
          //Page 4 button 0 function
        }else if(pageNum == 5){
          //Page 5 button 0 function
        }else if(pageNum == 6){
          // Enter config mode
          pageNum = 7;    // By setting pageNum to 7
          configmode();
          drawKeypad();   // and calling drawKeypad() a new keypad is drawn with pageNum 7
        }
      }
      
      if (b == 1) {
        //Button 1 functions
        if(pageNum == 0){
          //Page 0 button 1 function
          pageNum = 2;    // By setting pageNum to 2
          drawKeypad();   // and calling drawKeypad() a new keypad is drawn with pageNum 2
        }else if(pageNum == 1){
          //Page 1 button 1 function
          bleKeyboard.write(KEY_MEDIA_VOLUME_DOWN);
        }else if(pageNum == 2){
          //Page 2 button 1 function
        }else if(pageNum == 3){
          //Page 3 button 1 function
          bleKeyboard.press(KEY_LEFT_CTRL);
          bleKeyboard.press(KEY_LEFT_SHIFT);
          bleKeyboard.press(KEY_TAB);
          delay(50);
          bleKeyboard.releaseAll();   
        }else if(pageNum == 4){
          //Page 4 button 1 function
        }else if(pageNum == 5){
          //Page 5 button 1 function
        }else if(pageNum == 6){
          //Page 6 button 1 function
        }
      }

      if (b == 2) {
        //Button 2 functions
        if(pageNum == 0){
          //Page 0 button 2 function
          pageNum = 3;    // By setting pageNum to 3
          drawKeypad();   // and calling drawKeypad(), a new keypad is drawn with pageNum 3    
        }else if(pageNum == 1){
          //Page 1 button 2 function
          bleKeyboard.write(KEY_MEDIA_VOLUME_UP);
        }else if(pageNum == 2){
          //Page 2 button 2 function
        }else if(pageNum == 3){
          //Page 3 button 2 function
          bleKeyboard.press(KEY_LEFT_CTRL);
          bleKeyboard.press(KEY_TAB);
          delay(50);
          bleKeyboard.releaseAll();
        }else if(pageNum == 4){
          //Page 4 button 2 function
        }else if(pageNum == 5){
          //Page 5 button 2 function
        }
      }

      if (b == 3) {
        //Button 3 functions
        if(pageNum == 0){
          //Page 0 button function
          pageNum = 4;    // By setting pageNum to 4
          drawKeypad();   // and calling drawKeypad() a new keypad is drawn with pageNum 4
        }else if(pageNum == 1){
          //Page 1 button 3 function
          bleKeyboard.write(KEY_MEDIA_PLAY_PAUSE);
        }else if(pageNum == 2){
          //Page 2 button 3 function
        }else if(pageNum == 3){
          //Page 3 button 3 function
        }else if(pageNum == 4){
          //Page 4 button 3 function
        }else if(pageNum == 5){
          //Page 5 button 3 function
        }
      }

      if (b == 4) {
        //Button 4 functions
        if(pageNum == 0){
          //Page 0 button 4 function
          pageNum = 5;    // By setting pageNum to 5
          drawKeypad();   // and calling drawKeypad() a new keypad is drawn with pageNum 5
        }else if(pageNum == 1){
          //Page 1 button 4 function
        }else if(pageNum == 2){
          //Page 2 button 4 function
          bleKeyboard.write(KEY_MEDIA_STOP);
        }else if(pageNum == 3){
          //Page 3 button 4 function
        }else if(pageNum == 4){
          //Page 4 button 4 function
        }else if(pageNum == 5){
          //Page 5 button 4 function
        }
      }

      if (b == 5) {
        //Button 5 functions
        if(pageNum == 0){
          //Page 0 button 5 function
          pageNum = 6;    // By setting pageNum to 6
          drawKeypad();   // and calling drawKeypad() a new keypad is drawn with pageNum 6
        }else{
          pageNum = 0; // On any other page button 5 is always the "back home button"
          drawKeypad();
        }
      }

      delay(10); // UI debouncing
      
    }
  }
}
}

//----------- Drawing Buttons -------------------------------------------------------------------------------

void drawKeypad()
{
  // Draw the home screen button outlines and fill them with colours   
  if(pageNum == 0){
    for (uint8_t row = 0; row < 2; row++) {
      for (uint8_t col = 0; col < 3; col++) {
      
        uint8_t b = col + row * 3;
        tft.setFreeFont(LABEL_FONT);
        key[b].initButton(&tft, KEY_X + col * (KEY_W + KEY_SPACING_X),
                          KEY_Y + row * (KEY_H + KEY_SPACING_Y), // x, y, w, h, outline, fill, text
                          KEY_W, KEY_H, TFT_WHITE, generalconfig.menuButtonColour, 0xFFFF,
                          "", KEY_TEXTSIZE);
        key[b].drawButton();
        drawlogo(b,col,row,generalconfig.logoColour); // After drawing the button outline we call this to draw a logo.   
      }
    }

  }else if(pageNum == 7){
    // Config mode...
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(20, 20);
    tft.setTextFont(2);
    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.println("Now in config mode. Go to:");
    tft.println("  http://freetouchdeck.local");
    tft.println("  to configure.");
  }else{
  // Draw the function button outlines and fill them with colours   
    for (uint8_t row = 0; row < 2; row++) {
      for (uint8_t col = 0; col < 3; col++) {
        
        uint8_t b = col + row * 3;
        
        if (row == 1 && col == 2){
          // If it is the last button ("back home button") create it with the menuButtonColour
          tft.setFreeFont(LABEL_FONT);
          key[b].initButton(&tft, KEY_X + col * (KEY_W + KEY_SPACING_X),
                            KEY_Y + row * (KEY_H + KEY_SPACING_Y), // x, y, w, h, outline, fill, text
                            KEY_W, KEY_H, TFT_WHITE, generalconfig.menuButtonColour, TFT_WHITE,
                            "", KEY_TEXTSIZE);
          key[b].drawButton();
          drawlogo(b,col,row,generalconfig.logoColour);
        }else{
          // Otherwise use functionButtonColour
          tft.setFreeFont(LABEL_FONT);
          key[b].initButton(&tft, KEY_X + col * (KEY_W + KEY_SPACING_X),
                            KEY_Y + row * (KEY_H + KEY_SPACING_Y), // x, y, w, h, outline, fill, text
                            KEY_W, KEY_H, TFT_WHITE, generalconfig.functionButtonColour, TFT_WHITE,
                            "", KEY_TEXTSIZE);
          key[b].drawButton();
          drawlogo(b,col,row,generalconfig.logoColour);
        }
      }
    }
  }
}

//----------- Drawing Logo's -------------------------------------------------------------------------------

void drawlogo(int logonumber, int col, int row, uint16_t fgcolor){

  if(pageNum == 0){
    //Draw Home screen logo's
    if(logonumber == 0){
      drawBmpTransparent(screen0.logo0, KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y));
    }else if(logonumber == 1){
      drawBmpTransparent(screen0.logo1, KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y));
    }else if(logonumber == 2){
      drawBmpTransparent(screen0.logo2, KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y));
    }else if(logonumber == 3){
      drawBmpTransparent(screen0.logo3, KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y));
    }else if(logonumber == 4){
      drawBmpTransparent(screen0.logo4, KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y));
    }else if(logonumber == 5){
      drawBmpTransparent(screen0.logo5, KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y));
    }
  }else if (pageNum == 1){
    // pageNum 1 contains the Music logo's
    if(logonumber == 0){
      drawBmpTransparent(screen1.logo0, KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y));
    }else if(logonumber == 1){
      drawBmpTransparent(screen1.logo1, KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y));
    }else if(logonumber == 2){
      drawBmpTransparent(screen1.logo2, KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y));
    }else if(logonumber == 3){
      drawBmpTransparent(screen1.logo3, KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y));
    }else if(logonumber == 4){
      drawBmpTransparent(screen1.logo4, KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y));
    }else if(logonumber == 5){
      drawBmpTransparent(generallogo.homebutton, KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y));
    }
  }else if (pageNum == 2){
    // pageNum 2 contains the obs logo's
    if(logonumber == 0){
      drawBmpTransparent(screen2.logo0, KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y));
    }else if(logonumber == 1){
      drawBmpTransparent(screen2.logo1, KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y));
    }else if(logonumber == 2){
      drawBmpTransparent(screen2.logo2, KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y));
    }else if(logonumber == 3){
      drawBmpTransparent(screen2.logo3, KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y));
    }else if(logonumber == 4){
      drawBmpTransparent(screen2.logo4, KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y));
    }else if(logonumber == 5){
      drawBmpTransparent(generallogo.homebutton, KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y));
    }
  }else if (pageNum == 3){
    // pageNum 3 contains the Firefox logo's
    if(logonumber == 0){
      drawBmpTransparent(screen3.logo0, KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y));
    }else if(logonumber == 1){
      drawBmpTransparent(screen3.logo1, KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y));
    }else if(logonumber == 2){
      drawBmpTransparent(screen3.logo2, KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y));
    }else if(logonumber == 3){
      drawBmpTransparent(screen3.logo3, KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y));
    }else if(logonumber == 4){
      drawBmpTransparent(screen3.logo4, KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y));
    }else if(logonumber == 5){
      drawBmpTransparent(generallogo.homebutton, KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y));
    }
  }else if (pageNum == 4){
    // pageNum 4 contains the Mail logo's
    if(logonumber == 0){
      drawBmpTransparent(screen4.logo0, KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y));
    }else if(logonumber == 1){
      drawBmpTransparent(screen4.logo1, KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y));
    }else if(logonumber == 2){
      drawBmpTransparent(screen4.logo2, KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y));
    }else if(logonumber == 3){
      drawBmpTransparent(screen4.logo3, KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y));
    }else if(logonumber == 4){
      drawBmpTransparent(screen4.logo4, KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y));
    }else if(logonumber == 5){
      drawBmpTransparent(generallogo.homebutton, KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y));
    }
  }else if (pageNum == 5){
    // pageNum 5 contains the Discord logo's
    if(logonumber == 0){
      drawBmpTransparent(screen5.logo0, KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y));
    }else if(logonumber == 1){
      drawBmpTransparent(screen5.logo1, KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y));
    }else if(logonumber == 2){
      drawBmpTransparent(screen5.logo2, KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y));
    }else if(logonumber == 3){
      drawBmpTransparent(screen5.logo3, KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y));
    }else if(logonumber == 4){
      drawBmpTransparent(screen5.logo4, KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y));
    }else if(logonumber == 5){
      drawBmpTransparent(generallogo.homebutton, KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y));
    }
  }else if (pageNum == 6){
    //pageNum6 contains settings logos
    if(logonumber == 0){
      drawBmpTransparent(screen6.logo0, KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y));
    }else if(logonumber == 1){
      drawBmpTransparent(screen6.logo1, KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y));
    }else if(logonumber == 2){
      drawBmpTransparent(screen6.logo2, KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y));
    }else if(logonumber == 3){
      drawBmpTransparent(screen6.logo3, KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y));
    }else if(logonumber == 4){
      drawBmpTransparent(screen6.logo4, KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y));
    }else if(logonumber == 5){
      drawBmpTransparent(generallogo.homebutton, KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y));
    }
  }
}

//----------- Load General Config -------------------------------------------------------------------------------

void loadGeneralConfig(){

  if(SPIFFS.exists("/generalconfig.json")){

  File generalconfigfile = SPIFFS.open("/generalconfig.json", "r");

  const size_t capacity = 6*JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(6) + JSON_OBJECT_SIZE(12) + 659;
  DynamicJsonDocument doc(capacity);

  DeserializationError error = deserializeJson(doc, generalconfigfile);

  // If parsing failed
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    Serial.println("Using default colours");
    generalconfig.menuButtonColour = 0xF81F; // Use default colour if failed to parse json
    generalconfig.functionButtonColour = 0x0000; // Use default colour if failed to parse json
    generalconfig.logoColour = 0xFFFF; // Use default colour if failed to parse json
    generalconfig.backgroundColour = 0x7BEF; // Use default colour if failed to parse json
    return;
  }

  const char* menubuttoncolor = doc["menubuttoncolor"]; // Get the colour for the menu and back home buttons.
  const char* functionbuttoncolor = doc["functionbuttoncolor"]; //Get the colour for the function buttons.
  const char* logocolor = doc["logocolor"]; //Get the colour for the function buttons.
  const char* bgcolor = doc["background"]; //Get the colour for the function buttons.

  //generalconfig.menuButtonColour = menubuttoncolor; // Pass the colour to generalconfig

  char menubuttoncolorchar[64];
  strcpy (menubuttoncolorchar,menubuttoncolor);
  unsigned long rgb888menubuttoncolor = convertHTMLtoRGB888(menubuttoncolorchar);
  //Serial.println(rgb888);
  generalconfig.menuButtonColour = convertRGB888ToRGB565(rgb888menubuttoncolor);

  char functionbuttoncolorchar[64];
  strcpy (functionbuttoncolorchar,functionbuttoncolor);
  unsigned long rgb888functionbuttoncolor = convertHTMLtoRGB888(functionbuttoncolorchar);
  //Serial.println(rgb888);
  generalconfig.functionButtonColour = convertRGB888ToRGB565(rgb888functionbuttoncolor);

  char logocolorchar[64];
  strcpy (menubuttoncolorchar,logocolor);
  unsigned long rgb888logocolor = convertHTMLtoRGB888(logocolorchar);
  //Serial.println(rgb888);
  generalconfig.logoColour = convertRGB888ToRGB565(rgb888logocolor);

  char backgroundcolorchar[64];
  strcpy (backgroundcolorchar,bgcolor);
  unsigned long rgb888backgroundcolor = convertHTMLtoRGB888(backgroundcolorchar);
  //Serial.println(rgb888);
  generalconfig.backgroundColour = convertRGB888ToRGB565(rgb888backgroundcolor);

  //Get general logos
  const char* logohome = doc["homebuttonlogo"];
  strcpy (templogopath,logopath); // cpy the logo path (char logopath[64] = "/logos/") to templogopath
  strcat(templogopath,logohome); // append the logo filename (const char*) to templogopath
  strcpy (generallogo.homebutton,templogopath); // cpy the logo filename + path to generallogo.homebutton
  strcpy (templogopath,logopath); // empty the templogopath so we can re-use it

  //Get logos for screen 0 buttons
  const char* logo00 = doc["screen0"]["logo0"];
  const char* logo01 = doc["screen0"]["logo1"];
  const char* logo02 = doc["screen0"]["logo2"];
  const char* logo03 = doc["screen0"]["logo3"];
  const char* logo04 = doc["screen0"]["logo4"];
  const char* logo05 = doc["screen0"]["logo5"]; // Only screen 0 has 6 buttons

  strcpy (templogopath,logopath);
  strcat(templogopath,logo00);
  strcpy (screen0.logo0,templogopath);
  strcpy (templogopath,logopath);

  strcpy (templogopath,logopath);
  strcat(templogopath,logo01);
  strcpy (screen0.logo1,templogopath);
  strcpy (templogopath,logopath);

  strcpy (templogopath,logopath);
  strcat(templogopath,logo02);
  strcpy (screen0.logo2,templogopath);
  strcpy (templogopath,logopath);

  strcpy (templogopath,logopath);
  strcat(templogopath,logo03);
  strcpy (screen0.logo3,templogopath);
  strcpy (templogopath,logopath);

  strcpy (templogopath,logopath);
  strcat(templogopath,logo04);
  strcpy (screen0.logo4,templogopath);
  strcpy (templogopath,logopath);

  strcpy (templogopath,logopath);
  strcat(templogopath,logo05);
  strcpy (screen0.logo5,templogopath);
  strcpy (templogopath,logopath);

  // Get logos for screen 1 buttons
  const char* logo10 = doc["screen1"]["logo0"];
  const char* logo11 = doc["screen1"]["logo1"];
  const char* logo12 = doc["screen1"]["logo2"];
  const char* logo13 = doc["screen1"]["logo3"];
  const char* logo14 = doc["screen1"]["logo4"];

  strcpy (templogopath,logopath);
  strcat(templogopath,logo10);
  strcpy (screen1.logo0,templogopath);
  strcpy (templogopath,logopath);

  strcpy (templogopath,logopath);
  strcat(templogopath,logo11);
  strcpy (screen1.logo1,templogopath);
  strcpy (templogopath,logopath);

  strcpy (templogopath,logopath);
  strcat(templogopath,logo12);
  strcpy (screen1.logo2,templogopath);
  strcpy (templogopath,logopath);

  strcpy (templogopath,logopath);
  strcat(templogopath,logo13);
  strcpy (screen1.logo3,templogopath);
  strcpy (templogopath,logopath);

  strcpy (templogopath,logopath);
  strcat(templogopath,logo14);
  strcpy (screen1.logo4,templogopath);
  strcpy (templogopath,logopath);


  // Get logos for screen 2 buttons
  const char* logo20 = doc["screen2"]["logo0"];
  const char* logo21 = doc["screen2"]["logo1"];
  const char* logo22 = doc["screen2"]["logo2"];
  const char* logo23 = doc["screen2"]["logo3"];
  const char* logo24 = doc["screen2"]["logo4"];

  strcpy (templogopath,logopath);
  strcat(templogopath,logo20);
  strcpy (screen2.logo0,templogopath);
  strcpy (templogopath,logopath);

  strcpy (templogopath,logopath);
  strcat(templogopath,logo21);
  strcpy (screen2.logo1,templogopath);
  strcpy (templogopath,logopath);

  strcpy (templogopath,logopath);
  strcat(templogopath,logo22);
  strcpy (screen2.logo2,templogopath);
  strcpy (templogopath,logopath);

  strcpy (templogopath,logopath);
  strcat(templogopath,logo23);
  strcpy (screen2.logo3,templogopath);
  strcpy (templogopath,logopath);

  strcpy (templogopath,logopath);
  strcat(templogopath,logo24);
  strcpy (screen2.logo4,templogopath);
  strcpy (templogopath,logopath);

  // Get logos for screen 3 buttons
  const char* logo30 = doc["screen3"]["logo0"];
  const char* logo31 = doc["screen3"]["logo1"];
  const char* logo32 = doc["screen3"]["logo2"];
  const char* logo33 = doc["screen3"]["logo3"];
  const char* logo34 = doc["screen3"]["logo4"];

  strcpy (templogopath,logopath);
  strcat(templogopath,logo30);
  strcpy (screen3.logo0,templogopath);
  strcpy (templogopath,logopath);

  strcpy (templogopath,logopath);
  strcat(templogopath,logo31);
  strcpy (screen3.logo1,templogopath);
  strcpy (templogopath,logopath);

  strcpy (templogopath,logopath);
  strcat(templogopath,logo32);
  strcpy (screen3.logo2,templogopath);
  strcpy (templogopath,logopath);

  strcpy (templogopath,logopath);
  strcat(templogopath,logo33);
  strcpy (screen3.logo3,templogopath);
  strcpy (templogopath,logopath);

  strcpy (templogopath,logopath);
  strcat(templogopath,logo34);
  strcpy (screen3.logo4,templogopath);
  strcpy (templogopath,logopath);

  // Get logos for screen 4 buttons
  const char* logo40 = doc["screen4"]["logo0"];
  const char* logo41 = doc["screen4"]["logo1"];
  const char* logo42 = doc["screen4"]["logo2"];
  const char* logo43 = doc["screen4"]["logo3"];
  const char* logo44 = doc["screen4"]["logo4"];

  strcpy (templogopath,logopath);
  strcat(templogopath,logo40);
  strcpy (screen4.logo0,templogopath);
  strcpy (templogopath,logopath);

  strcpy (templogopath,logopath);
  strcat(templogopath,logo41);
  strcpy (screen4.logo1,templogopath);
  strcpy (templogopath,logopath);

  strcpy (templogopath,logopath);
  strcat(templogopath,logo42);
  strcpy (screen4.logo2,templogopath);
  strcpy (templogopath,logopath);

  strcpy (templogopath,logopath);
  strcat(templogopath,logo43);
  strcpy (screen4.logo3,templogopath);
  strcpy (templogopath,logopath);

  strcpy (templogopath,logopath);
  strcat(templogopath,logo44);
  strcpy (screen4.logo4,templogopath);
  strcpy (templogopath,logopath);

// Get logos for screen 5 buttons
  const char* logo50 = doc["screen5"]["logo0"];
  const char* logo51 = doc["screen5"]["logo1"];
  const char* logo52 = doc["screen5"]["logo2"];
  const char* logo53 = doc["screen5"]["logo3"];
  const char* logo54 = doc["screen5"]["logo4"];

  strcpy (templogopath,logopath);
  strcat(templogopath,logo50);
  strcpy (screen5.logo0,templogopath);
  strcpy (templogopath,logopath);

  strcpy (templogopath,logopath);
  strcat(templogopath,logo51);
  strcpy (screen5.logo1,templogopath);
  strcpy (templogopath,logopath);

  strcpy (templogopath,logopath);
  strcat(templogopath,logo52);
  strcpy (screen5.logo2,templogopath);
  strcpy (templogopath,logopath);

  strcpy (templogopath,logopath);
  strcat(templogopath,logo53);
  strcpy (screen5.logo3,templogopath);
  strcpy (templogopath,logopath);

  strcpy (templogopath,logopath);
  strcat(templogopath,logo54);
  strcpy (screen5.logo4,templogopath);
  strcpy (templogopath,logopath);

// Get logos for screen 6 buttons
  const char* logo60 = doc["screen6"]["logo0"];
  const char* logo61 = doc["screen6"]["logo1"];
  const char* logo62 = doc["screen6"]["logo2"];
  const char* logo63 = doc["screen6"]["logo3"];
  const char* logo64 = doc["screen6"]["logo4"];

  strcpy (templogopath,logopath);
  strcat(templogopath,logo60);
  strcpy (screen6.logo0,templogopath);
  strcpy (templogopath,logopath);

  strcpy (templogopath,logopath);
  strcat(templogopath,logo61);
  strcpy (screen6.logo1,templogopath);
  strcpy (templogopath,logopath);

  strcpy (templogopath,logopath);
  strcat(templogopath,logo62);
  strcpy (screen6.logo2,templogopath);
  strcpy (templogopath,logopath);

  strcpy (templogopath,logopath);
  strcat(templogopath,logo63);
  strcpy (screen6.logo3,templogopath);
  strcpy (templogopath,logopath);

  strcpy (templogopath,logopath);
  strcat(templogopath,logo64);
  strcpy (screen6.logo4,templogopath);
  strcpy (templogopath,logopath);



  generalconfigfile.close();

  }else{
      
    Serial.println("Confing file not found! Using default colours.");
    drawErrorMessage("Confing file not found! Using default colours...");
    delay(5000);
    tft.fillScreen(TFT_BLACK);
    generalconfig.menuButtonColour = 0xF81F; // Use default colour if failed to parse json
    generalconfig.functionButtonColour = 0x0000; // Use default colour if failed to parse json
    generalconfig.logoColour = 0xFFFF; // Use default colour if failed to parse json
    generalconfig.backgroundColour = 0x7BEF; // Use default colour if failed to parse json
    return;
    
  }
  
}

//----------- Draw Error Message -------------------------------------------------------------------------------

void drawErrorMessage(String message){

    tft.fillScreen(TFT_BLACK);
    tft.setCursor(20, 20);
    tft.setTextFont(2);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.println(message);
  
}

// ------------------ WIFI Stuff ----------------------------------------

void configmode() {

  // Stop BLE from interfering with our WIFI signal
  btStop();
  esp_bt_controller_disable();
  esp_bt_controller_deinit();
  esp_bt_controller_mem_release(ESP_BT_MODE_IDLE);

  // Set pageNum to 7 so no buttons are displayed and touches are ignored
  pageNum = 7;
  Serial.println("Entering Config Mode");

  // Start the webserver
  server.begin();
  Serial.println("HTTP Server Started");
  
}

void handleRestart(){ 

  // First send some text to the browser otherwise an ugly browser error shows up
  server.send(200, "text/plain", "FreeTouchDeck is restarting...");
  // Then restart the ESP
  Serial.println("ESP Restarting...");
  ESP.restart();

}

//void saveconfig(){ OLD!
//
//  // Delete existing file, otherwise the configuration is appended to the file
//  SPIFFS.remove("/generalconfig.json");
//
//  // Open file for writing
//  File file = SPIFFS.open("/generalconfig.json", "w");
//  if (!file) {
//    Serial.println(F("Failed to create file"));
//    return;
//  }
//
//  //Create a JSON document and a JSON object
//
//  StaticJsonDocument<256> doc;
//  JsonObject obj1 = doc.to<JsonObject>();
//
//  // Loop trough all agruments received and append them to our object
//  for (int i = 0; i < server.args(); i++) {
//    uint16_t value = server.arg(i).toInt();
//    obj1[server.argName(i)] = value;
//  }
//
//  // Serialize JSON to file
//  if (serializeJsonPretty(doc, file) == 0) {
//    Serial.println(F("Failed to write to file"));
//  }
//
//  // Close the file
//  file.close();
//
//  // Display "Configuration saved" page
//  handleFileRead("/saveconfig.htm");
//  
//}

void saveconfig(){

  // Delete existing file, otherwise the configuration is appended to the file
  SPIFFS.remove("/generalconfig.json");

  // Open file for writing
  File file = SPIFFS.open("/generalconfig.json", "w");
  if (!file) {
    Serial.println(F("Failed to create file"));
    return;
  }

  //Create a JSON document and a JSON object
  StaticJsonDocument<1700> doc;

  JsonObject obj1 = doc.to<JsonObject>();
  JsonObject screen0  = doc.createNestedObject("screen0");
  JsonObject screen1  = doc.createNestedObject("screen1");
  JsonObject screen2  = doc.createNestedObject("screen2");
  JsonObject screen3  = doc.createNestedObject("screen3");
  JsonObject screen4  = doc.createNestedObject("screen4");
  JsonObject screen5  = doc.createNestedObject("screen5");
  JsonObject screen6  = doc.createNestedObject("screen6");

  // Because the first 4 arguments are the colors, we loop
  for (int i = 0; i < 4; i++) {
    obj1[server.argName(i)] = server.arg(i);
  }

  // Create the homebuttonlogo JSON object
  obj1["homebuttonlogo"] = "home.bmp";

  // Now we parse the rest of the JSON here

  screen0["logo0"] = server.arg("screen0image0");
  screen0["logo1"] = server.arg("screen0image1");
  screen0["logo2"] = server.arg("screen0image2");
  screen0["logo3"] = server.arg("screen0image3");
  screen0["logo4"] = server.arg("screen0image4");
  screen0["logo5"] = server.arg("screen0image5");

  screen1["logo0"] = server.arg("screen1image0");
  screen1["logo1"] = server.arg("screen1image1");
  screen1["logo2"] = server.arg("screen1image2");
  screen1["logo3"] = server.arg("screen1image3");
  screen1["logo4"] = server.arg("screen1image4");

  screen2["logo0"] = server.arg("screen2image0");
  screen2["logo1"] = server.arg("screen2image1");
  screen2["logo2"] = server.arg("screen2image2");
  screen2["logo3"] = server.arg("screen2image3");
  screen2["logo4"] = server.arg("screen2image4");

  screen3["logo0"] = server.arg("screen3image0");
  screen3["logo1"] = server.arg("screen3image1");
  screen3["logo2"] = server.arg("screen3image2");
  screen3["logo3"] = server.arg("screen3image3");
  screen3["logo4"] = server.arg("screen3image4");

  screen4["logo0"] = server.arg("screen4image0");
  screen4["logo1"] = server.arg("screen4image1");
  screen4["logo2"] = server.arg("screen4image2");
  screen4["logo3"] = server.arg("screen4image3");
  screen4["logo4"] = server.arg("screen4image4");

  screen5["logo0"] = server.arg("screen5image0");
  screen5["logo1"] = server.arg("screen5image1");
  screen5["logo2"] = server.arg("screen5image2");
  screen5["logo3"] = server.arg("screen5image3");
  screen5["logo4"] = server.arg("screen5image4");

  screen6["logo0"] = server.arg("screen6image0");
  screen6["logo1"] = server.arg("screen6image1");
  screen6["logo2"] = server.arg("screen6image2");
  screen6["logo3"] = server.arg("screen6image3");
  screen6["logo4"] = server.arg("screen6image4");

  // Serialize JSON to file
  if (serializeJsonPretty(doc, file) == 0) {
    Serial.println(F("Failed to write to file"));
  }

  // Close the file
  file.close();

  // Display "Configuration saved" page
  handleFileRead("/saveconfig.htm");
  
}

//----------- TOUCH Calibration -------------------------------------------------------------------------------

void touch_calibrate()
{
  uint16_t calData[5];
  uint8_t calDataOK = 0;

  // check file system exists
  if (!SPIFFS.begin()) {
    Serial.println("Formating file system");
    SPIFFS.format();
    SPIFFS.begin();
  }

  // check if calibration file exists and size is correct
  if (SPIFFS.exists(CALIBRATION_FILE)) {
    if (REPEAT_CAL)
    {
      // Delete if we want to re-calibrate
      SPIFFS.remove(CALIBRATION_FILE);
    }
    else
    {
      File f = SPIFFS.open(CALIBRATION_FILE, "r");
      if (f) {
        if (f.readBytes((char *)calData, 14) == 14)
          calDataOK = 1;
        f.close();
      }
    }
  }

  if (calDataOK && !REPEAT_CAL) {
    // calibration data valid
    tft.setTouch(calData);
  } else {
    // data not valid so recalibrate
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(20, 0);
    tft.setTextFont(2);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    tft.println("Touch corners as indicated");

    tft.setTextFont(1);
    tft.println();

    if (REPEAT_CAL) {
      tft.setTextColor(TFT_RED, TFT_BLACK);
      tft.println("Set REPEAT_CAL to false to stop this running again!");
    }

    tft.calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);

    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.println("Calibration complete!");

    // store data
    File f = SPIFFS.open(CALIBRATION_FILE, "w");
    if (f) {
      f.write((const unsigned char *)calData, 14);
      f.close();
    }
  }
}

//----------- Functions used by Webserver -------------------------------------------------------------------------------

String getContentType(String filename) {
  if (server.hasArg("download")) {
    return "application/octet-stream";
  } else if (filename.endsWith(".htm")) {
    return "text/html";
  } else if (filename.endsWith(".html")) {
    return "text/html";
  } else if (filename.endsWith(".css")) {
    return "text/css";
  } else if (filename.endsWith(".js")) {
    return "application/javascript";
  } else if (filename.endsWith(".png")) {
    return "image/png";
  } else if (filename.endsWith(".gif")) {
    return "image/gif";
  } else if (filename.endsWith(".jpg")) {
    return "image/jpeg";
  } else if (filename.endsWith(".ico")) {
    return "image/x-icon";
  } else if (filename.endsWith(".xml")) {
    return "text/xml";
  } else if (filename.endsWith(".pdf")) {
    return "application/x-pdf";
  } else if (filename.endsWith(".zip")) {
    return "application/x-zip";
  } else if (filename.endsWith(".gz")) {
    return "application/x-gzip";
  } else if (filename.endsWith(".json")) {
    return "application/json";
  } else if (filename.endsWith(".bmp")) {
    return "image/bmp";
  }
  return "text/plain";
}

bool exists(String path){
  bool yes = false;
  File file = SPIFFS.open(path, "r");
  if(!file.isDirectory()){
    yes = true;
  }
  file.close();
  return yes;
}

bool handleFileRead(String path) {
  Serial.println("handleFileRead: " + path);
  if (path.endsWith("/")) {
    path += "index.htm";
  }
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  if (exists(pathWithGz) || exists(path)) {
    if (exists(pathWithGz)) {
      path += ".gz";
    }
    File file = SPIFFS.open(path, "r");
    server.streamFile(file, contentType);
    file.close();
    return true;
  }
  return false;
}

void faviconhandle(){

    File file = SPIFFS.open("/favicon.ico", "r");
    server.streamFile(file, "image/x-icon");
    file.close();
  
}

void handleFileList() {
  if (!server.hasArg("dir")) {
    server.send(500, "text/plain", "BAD ARGS");
    return;
  }

  String path = server.arg("dir");
  Serial.println("handleFileList: " + path);


  File root = SPIFFS.open(path);
  path = String();
  int filecount = 0;

  String output = "[";
  if(root.isDirectory()){
      File file = root.openNextFile();
      while(file){
          if (output != "[") {
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

//--------------------RBG888 to RGB565 conversion ------------------------

unsigned long convertHTMLtoRGB888(char* html){ // convert HTML (#xxxxxx to RGB888)

    //char* input="#61dfff";
    char* hex = html + 1; 
    unsigned long rgb = strtoul (hex, NULL, 16);
    return rgb;
}

unsigned int convertRGB888ToRGB565(unsigned long rgb){ //convert 24 bit RGB to 16bit 5:6:5 RGB
 
  return(((rgb&0xf80000)>>8)|((rgb&0xfc00)>>5)|((rgb&0xf8)>>3));
}

// ----------------- BMP Drawing ----------------------------------------

void drawBmpTransparent(const char *filename, int16_t x, int16_t y) {

  if ((x >= tft.width()) || (y >= tft.height())) return;

  fs::File bmpFS;

  // Open requested file on SD card
  bmpFS = SPIFFS.open(filename, "r");

  //Serial.print("Opening file: ");
  //Serial.print(filename);

  if (!bmpFS)
  {
    Serial.print("Bitmap not found: ");
    Serial.print(filename);
    return;
  }

  uint32_t seekOffset;
  uint16_t w, h, row, col;
  uint8_t  r, g, b;

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

      for (row = 0; row < h; row++) {
        
        bmpFS.read(lineBuffer, sizeof(lineBuffer));
        uint8_t*  bptr = lineBuffer;
        uint16_t* tptr = (uint16_t*)lineBuffer;
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
        tft.pushImage(x, y--, w, 1, (uint16_t*)lineBuffer, TFT_BLACK);
      }
      tft.setSwapBytes(oldSwapBytes);
      //Serial.print("Loaded in "); Serial.print(millis() - startTime);
      //Serial.println(" ms");
    }
    else Serial.println("BMP format not recognized.");
  }
  bmpFS.close();
}


void drawBmp(const char *filename, int16_t x, int16_t y) {

  if ((x >= tft.width()) || (y >= tft.height())) return;

  fs::File bmpFS;

  // Open requested file on SD card
  bmpFS = SPIFFS.open(filename, "r");

  if (!bmpFS)
  {
    Serial.print("File not found");
    return;
  }

  uint32_t seekOffset;
  uint16_t w, h, row, col;
  uint8_t  r, g, b;

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

      for (row = 0; row < h; row++) {
        
        bmpFS.read(lineBuffer, sizeof(lineBuffer));
        uint8_t*  bptr = lineBuffer;
        uint16_t* tptr = (uint16_t*)lineBuffer;
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
        tft.pushImage(x, y--, w, 1, (uint16_t*)lineBuffer);
      }
      tft.setSwapBytes(oldSwapBytes);
      //Serial.print("Loaded in "); Serial.print(millis() - startTime);
      //Serial.println(" ms");
    }
    else Serial.println("BMP format not recognized.");
  }
  bmpFS.close();
}

// These read 16- and 32-bit types from the SD card file.
// BMP data is stored little-endian, Arduino is little-endian too.
// May need to reverse subscript order if porting elsewhere.

uint16_t read16(fs::File &f) {
  uint16_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read(); // MSB
  return result;
}

uint32_t read32(fs::File &f) {
  uint32_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read();
  ((uint8_t *)&result)[2] = f.read();
  ((uint8_t *)&result)[3] = f.read(); // MSB
  return result;
}
