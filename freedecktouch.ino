/*
  TouchDeck (secret codename) based on the FreeDeck idea by Koriwi.
  It uses the TFT_eSPI library by Bodmer for the display and touch functionality and
  the ESP32-BLE-Keyboard library by T-vK.

  The following libraries must be installed:
  
  - https://github.com/adafruit/Adafruit-GFX-Library (also available trough Library Manager)
  - https://github.com/Bodmer/TFT_eSPI
  - https://github.com/T-vK/ESP32-BLE-Keyboard

  As this is an early Pre-alpha version, the code is ugly and sometimes way more complicated
  then necessary. It also lacks good documentation and comments in the code.

  tft_config.h holds the configuration for the TFT display and the touch controller. This is where
  SPI pins are defined. Uncomment the appropriate lines.

  The SPIFFS (FLASH filing system) is used to hold touch screen calibration data.
  It has to be runs at least once. After that you can set REPEAT_CAL to false.
  If you change the screen rotation, you need to run calibration data again.
  
*/

#include <pgmspace.h>  // PROGMEM support header

#include "FS.h" // File System header
#include "homelogos.h" // Logos for the home screen are here
#include "musiclogos.h" // Logos for the music screen are here
#include "obslogos.h" // Logos for the OBS screen are here
#include "browserlogos.h" // Logos for the browers screen are here

#include "tft_config.h" // Configuration data for TFT_eSPI

#define USER_SETUP_LOADED // Let TFT_eSPI know that we have our own config file

#include <SPI.h>
#include <TFT_eSPI.h>
#include <BleKeyboard.h>
#include <ArduinoJson.h>

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

struct Homepage {
  const unsigned char *logo;
};

struct Config {
  uint16_t menuButtonColour;
  uint16_t functionButtonColour;
  uint16_t logoColour;
  uint16_t backgroundColour;

};

Config generalconfig;

Homepage homepage; 

// Invoke the TFT_eSPI button class and create all the button objects
TFT_eSPI_Button key[6];

//------------------------------------------------------------------------------------------

void setup() {
  
  // Use serial port
  Serial.begin(9600);

  bleKeyboard.begin();

  // Initialise the TFT screen
  tft.init();

  // Set the rotation before we calibrate
  tft.setRotation(1);

  // Calibrate the touch screen and retrieve the scaling factors
  touch_calibrate();

  // Clear the screen
  tft.fillScreen(TFT_BLACK);

  // Load the General Config
  loadGeneralConfig();

  // Draw background
  tft.fillScreen(generalconfig.backgroundColour);

  // Setup the Font used for plain text
  tft.setFreeFont(LABEL_FONT);
  
  //readButtonConfig();

  // Draw keypad
  drawKeypad();

}

//------------------------------------------------------------------------------------------

void loop(void) {
  // To store the touch coordinates
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

        // draw normal button space (non inverted)
        key[b].drawButton();     
        // Call the drawLogo function with generalconfig.logoColour foreground (normal state)
        drawlogo(b,colArray[b],rowArray[b],generalconfig.logoColour);
    }

    if (key[b].justPressed()) {
        key[b].drawButton(true);  // draw inverted button space (white)
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
          //Page 0 button function
          pageNum = 1;    // By setting pageNum to 1
          drawKeypad();   // and calling drawKeypad() a new keypad is drawn with pageNum 1
        }else if(pageNum == 1){
          //Page 1 button function
          bleKeyboard.write(KEY_MEDIA_MUTE);
        }else if(pageNum == 2){
          //Page 2 button function
          bleKeyboard.write(KEY_MEDIA_MUTE); 
        }else if(pageNum == 3){
          //Page 3 button function
          bleKeyboard.press(KEY_LEFT_CTRL);
          bleKeyboard.print("t");
          delay(50);
          bleKeyboard.releaseAll();
        }else if(pageNum == 4){
          //Page 4 button function
        }else if(pageNum == 5){
          //Page 5 button function
        }
      }
      
      if (b == 1) {
        //Button 1 functions
        if(pageNum == 0){
          //Page 0 button function
          pageNum = 2;    // By setting pageNum to 2
          drawKeypad();   // and calling drawKeypad() a new keypad is drawn with pageNum 2
        }else if(pageNum == 1){
          //Page 1 button function
          bleKeyboard.write(KEY_MEDIA_VOLUME_DOWN);
        }else if(pageNum == 2){
          //Page 2 button function
        }else if(pageNum == 3){
          //Page 3 button function
          bleKeyboard.press(KEY_LEFT_CTRL);
          bleKeyboard.press(KEY_LEFT_SHIFT);
          bleKeyboard.press(KEY_TAB);
          delay(50);
          bleKeyboard.releaseAll();   
        }else if(pageNum == 4){
          //Page 4 button function
        }else if(pageNum == 5){
          //Page 5 button function
        }
      }

      if (b == 2) {
        //Button 2 functions
        if(pageNum == 0){
          //Page 0 button function
          pageNum = 3;    // By setting pageNum to 3
          drawKeypad();   // and calling drawKeypad(), a new keypad is drawn with pageNum 3    
        }else if(pageNum == 1){
          //Page 1 button function
          bleKeyboard.write(KEY_MEDIA_VOLUME_UP);
        }else if(pageNum == 2){
          //Page 2 button function
        }else if(pageNum == 3){
          //Page 3 button function
          bleKeyboard.press(KEY_LEFT_CTRL);
          bleKeyboard.press(KEY_TAB);
          delay(50);
          bleKeyboard.releaseAll();
        }else if(pageNum == 4){
          //Page 4 button function
        }else if(pageNum == 5){
          //Page 5 button function
        }
      }

      if (b == 3) {
        //Button 3 functions
        if(pageNum == 0){
          //Page 0 button function
          pageNum = 4;    // By setting pageNum to 4
          drawKeypad();   // and calling drawKeypad() a new keypad is drawn with pageNum 4
        }else if(pageNum == 1){
          //Page 1 button function
          bleKeyboard.write(KEY_MEDIA_PLAY_PAUSE);
        }else if(pageNum == 2){
          //Page 2 button function
        }else if(pageNum == 3){
          //Page 3 button function
        }else if(pageNum == 4){
          //Page 4 button function
        }else if(pageNum == 5){
          //Page 5 button function
        }
      }

      if (b == 4) {
        //Button 4 functions
        if(pageNum == 0){
          //Page 0 button function
          pageNum = 5;    // By setting pageNum to 5
          drawKeypad();   // and calling drawKeypad() a new keypad is drawn with pageNum 5
        }else if(pageNum == 1){
          //Page 1 button function
        }else if(pageNum == 2){
          //Page 2 button function
          bleKeyboard.write(KEY_MEDIA_STOP);
        }else if(pageNum == 3){
          //Page 3 button function
        }else if(pageNum == 4){
          //Page 4 button function
        }else if(pageNum == 5){
          //Page 5 button function
        }
      }

      if (b == 5) {
        //Button 5 functions
        if(pageNum == 0){
          //Page 0 button function
          pageNum = 6;    // By setting pageNum to 6
          drawKeypad();   // and calling drawKeypad() a new keypad is drawn with pageNum 6
        }else{
          pageNum = 0; // Button 5 is always the "back home button"
          drawKeypad();
        }
      }

      delay(10); // UI debouncing
      
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

  // Draw the function button outlines and fill them with colours   
  }else{
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
    tft.drawXBitmap(KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y), music_home, logoWidth, logoHeight, fgcolor);
    }else if(logonumber == 1){
    tft.drawXBitmap(KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y), obs_home, logoWidth, logoHeight, fgcolor);
    }else if(logonumber == 2){
    tft.drawXBitmap(KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y), ff_home, logoWidth, logoHeight, fgcolor);
    }else if(logonumber == 3){
    tft.drawXBitmap(KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y), mail, logoWidth, logoHeight, fgcolor);
    }else if(logonumber == 4){
    tft.drawXBitmap(KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y), discord, logoWidth, logoHeight, fgcolor);
    }else if(logonumber == 5){
    tft.drawXBitmap(KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y), youtube, logoWidth, logoHeight, fgcolor);
    }
  }else if (pageNum == 1){
    // pageNum 1 contains the Music logo's
    if(logonumber == 0){
    tft.drawXBitmap(KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y), mute, logoWidth, logoHeight, fgcolor);
    }else if(logonumber == 1){
    tft.drawXBitmap(KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y), volume_down, logoWidth, logoHeight, fgcolor);
    }else if(logonumber == 2){
    tft.drawXBitmap(KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y), volume_up, logoWidth, logoHeight, fgcolor);
    }else if(logonumber == 3){
    tft.drawXBitmap(KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y), play, logoWidth, logoHeight, fgcolor);
    }else if(logonumber == 4){
    tft.drawXBitmap(KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y), stop_button, logoWidth, logoHeight, fgcolor);
    }else if(logonumber == 5){
    tft.drawXBitmap(KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y), home_button, logoWidth, logoHeight, fgcolor);
    }
  }else if (pageNum == 2){
    // pageNum 2 contains the obs logo's
    if(logonumber == 0){
    tft.drawXBitmap(KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y), scene1, logoWidth, logoHeight, fgcolor);
    }else if(logonumber == 1){
    tft.drawXBitmap(KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y), empty_button, logoWidth, logoHeight, fgcolor);
    }else if(logonumber == 2){
    tft.drawXBitmap(KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y), empty_button, logoWidth, logoHeight, fgcolor);
    }else if(logonumber == 3){
    tft.drawXBitmap(KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y), empty_button, logoWidth, logoHeight, fgcolor);
    }else if(logonumber == 4){
    tft.drawXBitmap(KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y), empty_button, logoWidth, logoHeight, fgcolor);
    }else if(logonumber == 5){
    tft.drawXBitmap(KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y), home_button, logoWidth, logoHeight, fgcolor);
    }
  }else if (pageNum == 3){
    // pageNum 3 contains the Firefox logo's
    if(logonumber == 0){
    tft.drawXBitmap(KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y), newtab, logoWidth, logoHeight, fgcolor);
    }else if(logonumber == 1){
    tft.drawXBitmap(KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y), empty_button, logoWidth, logoHeight, fgcolor);
    }else if(logonumber == 2){
    tft.drawXBitmap(KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y), empty_button, logoWidth, logoHeight, fgcolor);
    }else if(logonumber == 3){
    tft.drawXBitmap(KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y), empty_button, logoWidth, logoHeight, fgcolor);
    }else if(logonumber == 4){
    tft.drawXBitmap(KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y), empty_button, logoWidth, logoHeight, fgcolor);
    }else if(logonumber == 5){
    tft.drawXBitmap(KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y), home_button, logoWidth, logoHeight, fgcolor);
    }
  }else if (pageNum == 4){
    // pageNum 4 contains the Mail logo's
    if(logonumber == 0){
    tft.drawXBitmap(KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y), empty_button, logoWidth, logoHeight, fgcolor);
    }else if(logonumber == 1){
    tft.drawXBitmap(KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y), empty_button, logoWidth, logoHeight, fgcolor);
    }else if(logonumber == 2){
    tft.drawXBitmap(KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y), empty_button, logoWidth, logoHeight, fgcolor);
    }else if(logonumber == 3){
    tft.drawXBitmap(KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y), empty_button, logoWidth, logoHeight, fgcolor);
    }else if(logonumber == 4){
    tft.drawXBitmap(KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y), empty_button, logoWidth, logoHeight, fgcolor);
    }else if(logonumber == 5){
    tft.drawXBitmap(KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y), home_button, logoWidth, logoHeight, fgcolor);
    }
  }else if (pageNum == 5){
    // pageNum 5 contains the Discord logo's
    if(logonumber == 0){
    tft.drawXBitmap(KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y), empty_button, logoWidth, logoHeight, fgcolor);
    }else if(logonumber == 1){
    tft.drawXBitmap(KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y), empty_button, logoWidth, logoHeight, fgcolor);
    }else if(logonumber == 2){
    tft.drawXBitmap(KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y), empty_button, logoWidth, logoHeight, fgcolor);
    }else if(logonumber == 3){
    tft.drawXBitmap(KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y), empty_button, logoWidth, logoHeight, fgcolor);
    }else if(logonumber == 4){
    tft.drawXBitmap(KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y), empty_button, logoWidth, logoHeight, fgcolor);
    }else if(logonumber == 5){
    tft.drawXBitmap(KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y), home_button, logoWidth, logoHeight, fgcolor);
    }
  }else if (pageNum == 6){
    // pageNum 6 contains the Youtube logo's
    if(logonumber == 0){
    tft.drawXBitmap(KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y), empty_button, logoWidth, logoHeight, fgcolor);
    }else if(logonumber == 1){
    tft.drawXBitmap(KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y), empty_button, logoWidth, logoHeight, fgcolor);
    }else if(logonumber == 2){
    tft.drawXBitmap(KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y), empty_button, logoWidth, logoHeight, fgcolor);
    }else if(logonumber == 3){
    tft.drawXBitmap(KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y), empty_button, logoWidth, logoHeight, fgcolor);
    }else if(logonumber == 4){
    tft.drawXBitmap(KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y), empty_button, logoWidth, logoHeight, fgcolor);
    }else if(logonumber == 5){
    tft.drawXBitmap(KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y), home_button, logoWidth, logoHeight, fgcolor);
    }
  }
}

//----------- Load General Config -------------------------------------------------------------------------------

void loadGeneralConfig(){

  if(SPIFFS.exists("/generalconfig.json") == true){

  File generalconfigfile = SPIFFS.open("/generalconfig.json", "r");

  const size_t capacity = JSON_ARRAY_SIZE(6) + 113;
  DynamicJsonDocument doc(capacity);

  DeserializationError error = deserializeJson(doc, generalconfigfile);

  // Test if parsing succeeds.
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

  int menubuttoncolor = doc["menubuttoncolor"]; // Get the colour for the menu and back home buttons. 
  int functionbuttoncolor = doc["functionbuttoncolor"]; //Get the colour for the function buttons.
  int logocolor = doc["logocolor"]; //Get the colour for the function buttons.
  int bgcolor = doc["background"]; //Get the colour for the function buttons.

  generalconfig.menuButtonColour = menubuttoncolor; // Pass the colour to generalconfig
  generalconfig.functionButtonColour = functionbuttoncolor; // Pass the colour to generalconfig
  generalconfig.logoColour = logocolor; // Pass the colour to generalconfig
  generalconfig.backgroundColour = bgcolor; // Use default colour if failed to parse json

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

//----------- Load Button Config -------------------------------------------------------------------------------

void readButtonConfig(){

  File buttonconfig = SPIFFS.open("/buttons.json", "r");

  const size_t capacity = 2*JSON_OBJECT_SIZE(1) + 11*JSON_OBJECT_SIZE(2) + 2*JSON_OBJECT_SIZE(5) + 363;
  DynamicJsonDocument doc(capacity);

  deserializeJson(doc, buttonconfig);

  const char* logo = doc["homepage"]["buttons"]["0"]["logo"]; //Get the logo
  const char* function = doc["homepage"]["buttons"]["0"]["function"]; //Get the function

  homepage.logo = (const unsigned char*)logo;

  buttonconfig.close();
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
