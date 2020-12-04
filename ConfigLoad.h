/* --------------------- Loading main config ---------------- 
Purpose: This function opens wificonfig.json and fills the wificonfig
         object accordingly.
Input  : none
Output : boolean
Note   : This is also where the sleep configuration lives.
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
  strlcpy(wificonfig.hostname, doc["wifihostname"] | "freetouchdeck", sizeof(wificonfig.hostname));

  bool sleepenable = doc["sleepenable"] | false;
  if (sleepenable)
  {
    wificonfig.sleepenable = true;
    islatched[28] = 1;
  }
  else
  {
    wificonfig.sleepenable = false;
  }

  uint16_t sleeptimer = doc["sleeptimer"];
  wificonfig.sleeptimer = sleeptimer;

  configfile.close();

  if (error)
  {
    Serial.println("[ERROR]: deserializeJson() error");
    Serial.println(error.c_str());
    return false;
  }

  return true;
}

/* -------------------- Loading menu config ---------------- 
Purpose: This function loads the menu configuration. 
Input  : String value
Output : none
Note   : Options for value are: colors, menu1, menu2, menu3
         menu4, and menu5
*/

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
    const char *latchcolor = doc["latchcolor"];                   // Get the colour to use when latching.
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

    if (error)
    {
      Serial.println("[ERROR]: deserializeJson() error");
      Serial.println(error.c_str());
    }
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

    if (error)
    {
      Serial.println("[ERROR]: deserializeJson() error");
      Serial.println(error.c_str());
    }

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

    if (error)
    {
      Serial.println("[ERROR]: deserializeJson() error");
      Serial.println(error.c_str());
    }

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

    if (error)
    {
      Serial.println("[ERROR]: deserializeJson() error");
      Serial.println(error.c_str());
    }

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

    if (error)
    {
      Serial.println("[ERROR]: deserializeJson() error");
      Serial.println(error.c_str());
    }

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

    if (error)
    {
      Serial.println("[ERROR]: deserializeJson() error");
      Serial.println(error.c_str());
    }

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

    if (error)
    {
      Serial.println("[ERROR]: deserializeJson() error");
      Serial.println(error.c_str());
    }
  }
}
