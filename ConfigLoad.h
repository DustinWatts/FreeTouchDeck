/**
* @brief This function opens wificonfig.json and fills the wificonfig
*        struct accordingly.
*
* @param none
*
* @return True when succeeded. False otherwise.
*
* @note This is also where the sleep configuration lives.
*/
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
  strlcpy(wificonfig.wifimode, doc["wifimode"] | "FAILED", sizeof(wificonfig.wifimode));
  strlcpy(wificonfig.hostname, doc["wifihostname"] | "freetouchdeck", sizeof(wificonfig.hostname));

  uint8_t attempts = doc["attempts"] | 10 ;
  wificonfig.attempts = attempts;

  uint16_t attemptdelay = doc["attemptdelay"] | 500 ;
  wificonfig.attemptdelay = attemptdelay;

  configfile.close();

  if (error)
  {
    Serial.println("[ERROR]: deserializeJson() error");
    Serial.println(error.c_str());
    return false;
  }

  return true;
}

/**
* @brief update the rows and cols property of provided Screen from
*        properties in the json document or by guesing at a balanced
*        ratio.
* 
* @param Screen* the screen object to update
* @param int the total number of buttons on the screen
* @param DynamicJsonDocument& the json document with possible rows/cols values.
*
* @return none
*/
void setColsRows(Screen * s, int len, DynamicJsonDocument & doc) {
  if (doc.containsKey("cols")) {
    s->cols = doc["cols"];
    if (doc.containsKey("rows")) {
      s->rows = doc["rows"];
    } else {
      s->rows = int(len / s->cols);
    }
    return;
  }

  // somewhat arbitrary algorithm to find a best fit for row/cols given the 
  // current button count.  Roughly this will try to have more or equal cols
  // compared to rows, but tries to keep it roughly squarish.
  int lastRemainder = -1;
  for (int i = 1; i < 10; i++) {
    int colsGuess = i;
    int rowsGuess = ceil(len/i);
    int remainder = (colsGuess * rowsGuess) - len;

    // more rows than cols is a reject
    if (rowsGuess > colsGuess) continue;
    
    // would have a whole row of empty buttons so reject
    if (remainder > colsGuess) continue;

    // skip long skinny options (ie prefer 3x2 instead of 6x1
    if (rowsGuess*2 < colsGuess) continue;

    // skip if this is not a perfect fit or if would results in more
    // empty buttons than previous solutions.
    if (remainder != 0 && remainder > lastRemainder) continue;

    // found a workable solution
    s->cols = colsGuess;
    s->rows = rowsGuess;
    lastRemainder = remainder;

    // if we found a perfect solution quit
    if (remainder == 0) break;
  }
}

/**
* @brief This function loads the menu configuration.
*
* @param String the config to be loaded
*
* @return none
*
* @note Options for values are: colors, homescreen, menu1, menu2, menu3
         menu4, and menu5
*/
bool loadConfig(String value)
{
  const String logoKeyPrefix = "logo";
  const String buttonKeyPrefix = "button";
  const String menuPrefix = "menu";

  if (value == "general")
  {
    File configfile = FILESYSTEM.open("/config/general.json", "r");

    DynamicJsonDocument doc(384);

    DeserializationError error = deserializeJson(doc, configfile);

    // Parsing colors
    const char *menubuttoncolor = doc["menubuttoncolor"] | "#009bf4";         // Get the colour for the menu and back home buttons.
    const char *functionbuttoncolor = doc["functionbuttoncolor"] | "#00efcb"; // Get the colour for the function buttons.
    const char *latchcolor = doc["latchcolor"] | "#fe0149";                   // Get the colour to use when latching.
    const char *bgcolor = doc["background"] | "#000000";                      // Get the colour for the background.

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

    // Loading general settings

    generalconfig.sleepenable = doc["sleepenable"] | false;
  
    //uint16_t sleeptimer = doc["sleeptimer"];
    uint16_t sleeptimer = doc["sleeptimer"] | 60 ;
    generalconfig.sleeptimer = sleeptimer;
  
    bool beep = doc["beep"] | false;
    generalconfig.beep = beep;
  
    uint8_t modifier1 = doc["modifier1"] | 0 ;
    generalconfig.modifier1 = modifier1;
  
    uint8_t modifier2 = doc["modifier2"] | 0 ;
    generalconfig.modifier2 = modifier2;
  
    uint8_t modifier3 = doc["modifier3"] | 0 ;
    generalconfig.modifier3 = modifier3;
  
    uint16_t helperdelay = doc["helperdelay"] | 250 ;
    generalconfig.helperdelay = helperdelay;

    configfile.close();

    if (error)
    {
      Serial.println("[ERROR]: deserializeJson() error");
      Serial.println(error.c_str());
      return false;
    }
    return true;
  }
  else if (value == "homescreen")
  {
    File configfile = FILESYSTEM.open("/config/homescreen.json", "r");
    int size = configfile.size();
    if (size > MAX_CONFIG_SIZE) {
      Serial.printf("[ERROR]: /config/homescreen.json too big: %d > %d", size, MAX_CONFIG_SIZE);
      return false;
    }
    // MAX_CONFIG_SIZE is 32k, we will allow up to allocatting 64k to process document.
    DynamicJsonDocument doc(size*2);

    DeserializationError error = deserializeJson(doc, configfile);

    // allocate homescreen
    screen.push_back(new Screen());
    for( int k=0; k < MAX_BUTTONS; k++) {
      // look for "logo0", "logo1" etc, stop looping on first
      // missing key.
      if (!doc.containsKey(logoKeyPrefix + k)) {
        break;
      }
      screen[0]->logo.push_back(logopath + String(doc[logoKeyPrefix + k] | "question.bmp"));
      // each key on the homescreen represents another screen, ensure
      // it is allocated now
      screen.push_back(new Screen());
    }
    // ensure cols/rows set for homescreen
    setColsRows(screen.front(), screen.size()-1, doc);
    configfile.close();

    if (error)
    {
      Serial.println("[ERROR]: deserializeJson() error");
      Serial.println(error.c_str());
      return false;
    }
    return true;
  }
  else if (value.startsWith(menuPrefix))
  {
    // decode menu number which is 4 chars after string start
    // (ie skip the "menu" part)
    int menuNum = atoi(value.c_str()+menuPrefix.length());
    Screen* s = screen[menuNum];
    String filename = configpath + menuPrefix + menuNum + ".json";
    File configfile = FILESYSTEM.open(filename, "r");

    int size = configfile.size();
    if (size > MAX_CONFIG_SIZE) {
      Serial.printf("[ERROR]: %s too big: %d > %d", filename.c_str(), size, MAX_CONFIG_SIZE);
      return false;
    }
    // MAX_CONFIG_SIZE is 32k, we will allow up to allocatting 64k to process document.
    DynamicJsonDocument doc(size*2);

    DeserializationError error = deserializeJson(doc, configfile);
    for (int k = 0; k < MAX_BUTTONS; k++) {
      if (!doc.containsKey(logoKeyPrefix + k)) {
        break;
      }
      s->logo.push_back(logopath + String(doc[logoKeyPrefix + k] | "question.bmp"));
      s->button.push_back(new Button());
      String buttonKey = buttonKeyPrefix + k;
      s->button[k]->latchlogo = logopath + String(doc[buttonKey]["latchlogo"] | "question.bmp");
      s->button[k]->latch = doc[buttonKey]["latch"] | false;

      JsonArray button_actionarray = doc[buttonKey]["actionarray"];

      int button_actionarray_0 = button_actionarray[0];
      int button_actionarray_1 = button_actionarray[1];
      int button_actionarray_2 = button_actionarray[2];

      JsonArray button_valuearray = doc[buttonKey]["valuearray"];

      if (button_actionarray_0 == CharAction || button_actionarray_0 == SpecialCharAction)
      {
        const char *button_symbolarray_0 = button_valuearray[0];
        strcpy(s->button[k]->actions.symbol0, button_symbolarray_0);
      }
      else
      {
        int button_valuearray_0 = button_valuearray[0];
        s->button[k]->actions.value0 = button_valuearray_0;
      }

      if (button_actionarray_1 == CharAction || button_actionarray_1 == SpecialCharAction)
      {
        const char *button_symbolarray_1 = button_valuearray[1];
        strcpy(s->button[k]->actions.symbol1, button_symbolarray_1);
      }
      else
      {
        int button_valuearray_1 = button_valuearray[1];
        s->button[k]->actions.value1 = button_valuearray_1;
      }

      if (button_actionarray_2 == CharAction || button_actionarray_2 == SpecialCharAction)
      {
        const char *button_symbolarray_2 = button_valuearray[2];
        strcpy(s->button[k]->actions.symbol2, button_symbolarray_2);
      }
      else
      {
        int button_valuearray_2 = button_valuearray[2];
        s->button[k]->actions.value2 = button_valuearray_2;
      }

      s->button[k]->actions.action0 = button_actionarray_0;
      s->button[k]->actions.action1 = button_actionarray_1;
      s->button[k]->actions.action2 = button_actionarray_2;
    }

    setColsRows(s, s->button.size()+1, doc);

    configfile.close();

    if (error)
    {
      Serial.println("[ERROR]: deserializeJson() error");
      Serial.println(error.c_str());
      return false;
    }
    return true;
  }
  else
  {
    return false;
  }
}