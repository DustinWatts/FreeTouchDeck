/**
* @brief This function draws the a "latched" dot. it uses the logonumber, column and row to
*        determine where.
*
* @param col int
* @param row int
*
* @return none
*
* @note none
*/
void drawlatched(int col, int row)
{
  int markWidth = 18;
  int colOffset = col*(keyW()+keySpacingX());
  int rowOffset = row*(keyH()+keySpacingY());
  int markOffset = markWidth;
  int keyHeightOffset = keyH()/2;
  int keyWidthOffset= keyW()/2;
  tft.fillRoundRect(keyX()+colOffset-keyWidthOffset+markOffset, keyY()+rowOffset-keyHeightOffset+markOffset, markWidth, markWidth, markWidth/4, generalconfig.latchedColour);
}


/**
* @brief This function draws the logos according to the page
         we are currently on. The pagenumber is a global variable
         and doesn't need to be passed. 
*
* @param logonumber int 
* @param col int
* @param row int
* @param transparent boolean
*
* @return none
*
* @note Logos start at the top left and are 0 indexed. The same goes 
         for the colomn and the row.
*/
void drawlogo(int logonumber, int col, int row, bool transparent, bool latch)
{

  Screen* s = currentScreen();
  if (pageNum == HomePage) 
  {
    if (logonumber >= s->logo.size()) return;

    // Draw Home screen logo's
    if (transparent == true)
    {
      drawBmpTransparent(s->logo[logonumber], keyX() + col * (keyW() + keySpacingX()), keyY() + row * (keyH() + keySpacingY()));
    }
    else
    {
      drawBmp(s->logo[logonumber], keyX() + col * (keyW() + keySpacingX()), keyY() + row * (keyH() + keySpacingY()));
    }
  }
  else if (pageNum > HomePage && pageNum < SettingsPage)
  {
    if (logonumber == s->cols * s->rows - 1) { // Home key
      if (transparent == true)
      {
        drawBmpTransparent(generallogo.homebutton, keyX() + col * (keyW() + keySpacingX()), keyY() + row * (keyH() + keySpacingY()));
      }
      else
      {
        drawBmp(generallogo.homebutton, keyX() + col * (keyW() + keySpacingX()), keyY() + row * (keyH() + keySpacingY()));
      }
    }
    else {
      if (transparent == true)
      {
        if (logonumber >= s->logo.size() || logonumber >= s->button.size()) return;
        if (latch == true)
        {
          if (s->button[logonumber]->latchlogo != logopath)
          {
            drawBmpTransparent(s->button[logonumber]->latchlogo, keyX() + col * (keyW() + keySpacingX()), keyY() + row * (keyH() + keySpacingY()));
          }
          else
          {
            drawBmpTransparent(s->logo[logonumber], keyX() + col * (keyW() + keySpacingX()), keyY() + row * (keyH() + keySpacingY()));
            drawlatched(col, row);
          }
        }
        else
        {
         drawBmpTransparent(s->logo[logonumber], keyX() + col * (keyW() + keySpacingX()), keyY() + row * (keyH() + keySpacingY()));
        }
      }
      else
      {
        if (latch == true)
        {

          if (s->button[logonumber]->latchlogo != logopath) 
          {
            drawBmp(s->button[logonumber]->latchlogo, keyX() + col * (keyW() + keySpacingX()), keyY() + row * (keyH() + keySpacingY()));
          }
          else
          {
            drawBmp(s->logo[logonumber], keyX() + col * (keyW() + keySpacingX()), keyY() + row * (keyH() + keySpacingY()));
            drawlatched(col, row);
          }
        }
        else
        {
          drawBmp(s->logo[logonumber], keyX() + col * (keyW() + keySpacingX()), keyY() + row * (keyH() + keySpacingY()));
        }
      }
    }

  }
  else if (pageNum == SettingsPage)
  {
    // hardcoded logos for settings page
    if (logonumber == 0)
    {
      drawBmpTransparent(generallogo.configurator, keyX() + col * (keyW() + keySpacingX()), keyY() + row * (keyH() + keySpacingY()));
    }
    else if (logonumber == 1)
    {
      drawBmpTransparent("/logos/brightnessdown.bmp", keyX() + col * (keyW() + keySpacingX()), keyY() + row * (keyH() + keySpacingY()));
    }
    else if (logonumber == 2)
    {
      drawBmpTransparent("/logos/brightnessup.bmp", keyX() + col * (keyW() + keySpacingX()), keyY() + row * (keyH() + keySpacingY()));
    }
    else if (logonumber == 3)
    {
      drawBmpTransparent("/logos/sleep.bmp", keyX() + col * (keyW() + keySpacingX()), keyY() + row * (keyH() + keySpacingY()));
      if (latch)
      {
        drawlatched(col, row);
      }
    }
    else if (logonumber == 4)
    {
      drawBmpTransparent("/logos/info.bmp", keyX() + col * (keyW() + keySpacingX()), keyY() + row * (keyH() + keySpacingY()));
    }
    else if (logonumber == 5)
    {
      if (transparent == true)
      {
        drawBmpTransparent(generallogo.homebutton, keyX() + col * (keyW() + keySpacingX()), keyY() + row * (keyH() + keySpacingY()));
      }
      else
      {
        drawBmp(generallogo.homebutton, keyX() + col * (keyW() + keySpacingX()), keyY() + row * (keyH() + keySpacingY()));
      }
    }
  }
}

/**
* @brief This function draws the 6 buttons that are on every page.
         Pagenumber is global and doesn't need to be passed.
*
* @param none
*
* @return none
*
* @note Three possibilities: pagenumber = 0 means homescreen,
         pagenumber = 7 means config mode, anything else is a menu.
*/
void drawKeypad()
{
  if (pageNum == ConfigErrorPage)
  {
    // Pagenum 10 means that a JSON config failed to load completely.
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0, 0);
    tft.setTextFont(2);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    tft.printf("  %s failed to load and might be corrupted.\n", jsonfilefail);
    tft.println("  You can reset that specific file to default by opening the serial monitor");
    tft.printf("  and typing \"reset %s\"\n", jsonfilefail);
    tft.println("  If you don't do this, the configurator will fail to load.");
    return;
  } else if (pageNum < HomePage) {
    // skip different error page
    return;
  }
  // reset background in case the screen uses different layout.
  tft.fillScreen(generalconfig.backgroundColour);
  
  Screen* s = currentScreen();
  // Draw the home screen button outlines and fill them with colours
  if (pageNum == HomePage)
  {
    uint8_t b = -1;
    for (uint8_t row = 0; row < s->rows; row++)
    {
      for (uint8_t col = 0; col < s->cols; col++)
      {
        b++;
        uint16_t buttonBG;
        bool drawTransparent;
        uint16_t imageBGColor = getImageBG(b);
        if (imageBGColor > 0)
        {
          buttonBG = imageBGColor;
          drawTransparent = false;
        }
        else
        {
          buttonBG = generalconfig.menuButtonColour;
          drawTransparent = true;
        }
        tft.setFreeFont(LABEL_FONT);
        key[b]->initButton(&tft, keyX() + col * (keyW() + keySpacingX()),
                          keyY() + row * (keyH() + keySpacingY()), // x, y, w, h, outline, fill, text
                          keyW(), keyH(), TFT_WHITE, buttonBG, TFT_WHITE,
                          "", KEY_TEXTSIZE);
        key[b]->drawButton();
        drawlogo(b, col, row, drawTransparent, false); // After drawing the button outline we call this to draw a logo.
      }
    }
  }
  else
  {
    uint8_t b = -1;
    // Draw the button outlines and fill them with colours
    for (uint8_t row = 0; row < s->rows; row++)
    {
      for (uint8_t col = 0; col < s->cols; col++)
      {
        b++;
        if (b == s->cols * s->rows - 1)
        {

          // Check if "home.bmp" is a transparent one

          uint16_t buttonBG;
          bool drawTransparent;
          uint16_t imageBGColor;

          imageBGColor = getImageBG(b);

          if (imageBGColor > 0)
          {
            buttonBG = imageBGColor;
            drawTransparent = false;
          }
          else
          {
            buttonBG = generalconfig.menuButtonColour;
            drawTransparent = true;
          }
          
          tft.setFreeFont(LABEL_FONT);
          key[b]->initButton(&tft, keyX() + col * (keyW() + keySpacingX()),
                            keyY() + row * (keyH() + keySpacingY()), // x, y, w, h, outline, fill, text
                            keyW(), keyH(), TFT_WHITE, buttonBG, TFT_WHITE,
                            "", KEY_TEXTSIZE);
          key[b]->drawButton();
          drawlogo(b, col, row, drawTransparent, false);
        }
        else
        {
          // Otherwise use functionButtonColour
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
            buttonBG = generalconfig.functionButtonColour;
            drawTransparent = true;
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
      }
    }
  }
}

/* ------------- Print an error message the TFT screen  ---------------- 
Purpose: This function prints an message to the TFT screen on a black 
         background. 
Input  : String message
Output : none
Note   : none
*/

void drawErrorMessage(String message)
{

  tft.fillScreen(TFT_BLACK);
  tft.setCursor(20, 20);
  tft.setTextFont(2);
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.println(message);
}

/**
* @brief This function gets the Bluetooth device address and prints it to the serial monitor
         and the TFT screen in a 6 byte format, seperating each byte by ":".
*
* @param none
*
* @return none
*
* @note e.g. 00:11:22:33:22:EE
*/
void printDeviceAddress()
{

  const uint8_t *point = esp_bt_dev_get_address();

  for (int i = 0; i < 6; i++)
  {

    char str[3];

    sprintf(str, "%02X", (int)point[i]);
    //Serial.print(str);
    tft.print(str);

    if (i < 5)
    {
     // Serial.print(":");
      tft.print(":");
    }
  }
}

/**
* @brief This function prints some information about the current version 
         and setup of FreetouchDeck to the TFT screen.
*
* @param none
*
* @return none
*
* @note none
*/
void printinfo()
{
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(1, 3);
  tft.setTextFont(2);
  if(SCREEN_WIDTH < 480)
  {
    tft.setTextSize(1);
  }
  else
  {
    tft.setTextSize(2);
  }
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.printf("Version: %s\n", versionnumber);

#ifdef touchInterruptPin
  if (generalconfig.sleepenable)
  {
    tft.println("Sleep: Enabled");
    tft.printf("Sleep timer: %u minutes\n", generalconfig.sleeptimer);
  }
  else
  {
    tft.println("Sleep: Disabled");
  }
#else
  tft.println("Sleep: Disabled");
#endif

#ifdef speakerPin
  if(generalconfig.beep){
    tft.println("Speaker: Enabled");
  }
  else
  {
    tft.println("Speaker: Disabled");
  }
#else
  tft.println("Speaker: Disabled");
#endif

  tft.print("Free Storage: ");
  float freemem = SPIFFS.totalBytes() - SPIFFS.usedBytes();
  tft.print(freemem / 1000);
  tft.println(" kB");
  tft.print("BLE Keyboard version: ");
  tft.println(BLE_KEYBOARD_VERSION);
  tft.print("ArduinoJson version: ");
  tft.println(ARDUINOJSON_VERSION);
  tft.print("TFT_eSPI version: ");
  tft.println(TFT_ESPI_VERSION);
  tft.println("ESP-IDF: ");
  tft.println(esp_get_idf_version());

  displayinginfo = true;
}
