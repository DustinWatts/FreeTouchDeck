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
      drawBmpTransparent("/logos/info.bmp", KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y));
    }
    else if (logonumber == 5)
    {
      drawBmpTransparent(generallogo.homebutton, KEY_X - 37 + col * (KEY_W + KEY_SPACING_X), KEY_Y - 37 + row * (KEY_H + KEY_SPACING_Y));
    }
  }
}

// -------------------- Draw Latched Button ------------------------

void drawlatched(int b, int col, int row, bool latched)
{
  if (latched)
  {
    tft.setFreeFont(LABEL_FONT);
    key[b].initButton(&tft, KEY_X + col * (KEY_W + KEY_SPACING_X),
                      KEY_Y + row * (KEY_H + KEY_SPACING_Y), // x, y, w, h, outline, fill, text
                      KEY_W, KEY_H, TFT_WHITE, generalconfig.latchedColour, TFT_WHITE,
                      "", KEY_TEXTSIZE);
    key[b].drawButton();
  }
  else
  {
    tft.setFreeFont(LABEL_FONT);
    key[b].initButton(&tft, KEY_X + col * (KEY_W + KEY_SPACING_X),
                      KEY_Y + row * (KEY_H + KEY_SPACING_Y), // x, y, w, h, outline, fill, text
                      KEY_W, KEY_H, TFT_WHITE, generalconfig.functionButtonColour, TFT_WHITE,
                      "", KEY_TEXTSIZE);
    key[b].drawButton();
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
    if (SCREEN_WIDTH == 480)
    {
      tft.setTextSize(2);
    }
    else
    {
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

          if (islatched[index])
          {
            drawlatched(b, col, row, true);
          }
          else
          {
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

/* ------------------------ Print Bluetooth device address function ---------------- 
Purpose: This function gets the Bluetooth device address and prints it to the serial monitor
         and the TFT screen in a 6 byte format, seperating each byte by ":".
Input  : none
Output : none
Note   : e.g. 00:11:22:33:FF:EE
*/

void printDeviceAddress()
{

  const uint8_t *point = esp_bt_dev_get_address();

  for (int i = 0; i < 6; i++)
  {

    char str[3];

    sprintf(str, "%02X", (int)point[i]);
    Serial.print(str);
    tft.print(str);

    if (i < 5)
    {
      Serial.print(":");
      tft.print(":");
    }
  }
}

// -------------------------- Display info to TFT ---------------------

void printinfo()
{
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(1, 3);
  tft.setTextFont(2);
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.printf("Version: %s\n", versionnumber);

#ifdef touchInterruptPin
  if (wificonfig.sleepenable)
  {
    tft.println("Sleep: Enabled");
    tft.printf("Sleep timer: %u minutes\n", wificonfig.sleeptimer);
  }
  else
  {
    tft.println("Sleep: Disabled");
  }
#else

  tft.println("Sleep: Disabled");

#endif
  tft.print("BLE MAC: ");
  printDeviceAddress();
  tft.println("");
  tft.print("WiFi MAC: ");
  tft.println(WiFi.macAddress());
  tft.println("ESP-IDF: ");
  tft.println(esp_get_idf_version());

  displayinginfo = true;
}
