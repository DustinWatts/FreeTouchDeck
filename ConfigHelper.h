// Start as WiFi station

bool startWifiStation(){
  
  Serial.printf("[INFO]: Connecting to %s", wificonfig.ssid);
  if (String(WiFi.SSID()) != String(wificonfig.ssid))
  {
      WiFi.mode(WIFI_STA);
      WiFi.begin(wificonfig.ssid, wificonfig.password);
      uint8_t attempts = wificonfig.attempts;
      while (WiFi.status() != WL_CONNECTED)
      {
        if(attempts == 0) {
          WiFi.disconnect();
          Serial.println("");
          return false;
          
        }
        delay(wificonfig.attemptdelay);
        Serial.print(".");
        attempts--;

      }
    }

    // Delete the task bleKeyboard had create to free memory and to not interfere with AsyncWebServer
      bleKeyboard.end();
    
      // Stop BLE from interfering with our WIFI signal
      btStop();
      esp_bt_controller_disable();
      esp_bt_controller_deinit();
      esp_bt_controller_mem_release(ESP_BT_MODE_BTDM);

      Serial.println("");
      Serial.println("[INFO]: BLE Stopped");  
      Serial.print("[INFO]: Connected! IP address: ");
      Serial.println(WiFi.localIP());

      MDNS.begin(wificonfig.hostname);
      MDNS.addService("http", "tcp", 80);
    
      // Set pageNum to 7 so no buttons are displayed and touches are ignored
      pageNum = 7;
    
      // Start the webserver
      webserver.begin();
      Serial.println("[INFO]: Webserver started");
      return true;
}

// Start as WiFi AP

void startWifiAP(){

  WiFi.mode(WIFI_AP);
  WiFi.softAP(wificonfig.ssid, wificonfig.password);
  Serial.println("");
  Serial.print("[INFO]: Access Point Started! IP address: ");
  Serial.println(WiFi.softAPIP());

  // Delete the task bleKeyboard had create to free memory and to not interfere with AsyncWebServer
  bleKeyboard.end();

  // Stop BLE from interfering with our WIFI signal
  btStop();
  esp_bt_controller_disable();
  esp_bt_controller_deinit();
  esp_bt_controller_mem_release(ESP_BT_MODE_BTDM);

  Serial.println("");
  Serial.println("[INFO]: BLE Stopped");

   MDNS.begin(wificonfig.hostname);
  MDNS.addService("http", "tcp", 80);

  // Set pageNum to 7 so no buttons are displayed and touches are ignored
  pageNum = 7;

  // Start the webserver
  webserver.begin();
  Serial.println("[INFO]: Webserver started");
}

// Start the default AP

void startDefaultAP(){

  const char* ssid = "FreeTouchDeck";
  const char* password = "defaultpass";

  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
  Serial.print("[INFO]: Access Point Started! IP address: ");
  Serial.println(WiFi.softAPIP());

  // Delete the task bleKeyboard had create to free memory and to not interfere with AsyncWebServer
  bleKeyboard.end();

  // Stop BLE from interfering with our WIFI signal
  btStop();
  esp_bt_controller_disable();
  esp_bt_controller_deinit();
  esp_bt_controller_mem_release(ESP_BT_MODE_BTDM);

  Serial.println("[INFO]: BLE Stopped");

  MDNS.begin("freetouchdeck");
  MDNS.addService("http", "tcp", 80);

  // Set pageNum to 7 so no buttons are displayed and touches are ignored
  pageNum = 7;

  // Start the webserver
  webserver.begin();
  Serial.println("[INFO]: Webserver started");
          
}

/**
* @brief This function stops Bluetooth and connects to the given 
         WiFi network. It the starts mDNS and starts the Async
         Webserver.
*
* @param none
*
* @return none
*
* @note none
*/
void configmode()
{

  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 0);
  tft.setTextFont(2);
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  Serial.println("[INFO]: Entering Config Mode");
  tft.println("Connecting to Wifi...");

  if (String(wificonfig.ssid) == "YOUR_WIFI_SSID" || String(wificonfig.password) == "YOUR_WIFI_PASSWORD") // Still default
  {
    tft.println("WiFi Config still set to default! Starting as AP.");
    Serial.println("[WARNING]: WiFi Config still set to default! Configurator started as AP.");
    startDefaultAP();
    tft.println("Started as AP because WiFi settings are still set to default.");
    tft.println("To configure, connect to 'FreeTouchDeck' with password 'defaultpass'");
    tft.println("Then go to http://freetouchdeck.local");
    tft.print("The IP is: ");
    tft.println(WiFi.softAPIP());
    return;
  }

  if (String(wificonfig.ssid) == "FAILED" || String(wificonfig.password) == "FAILED" || String(wificonfig.wifimode) == "FAILED") // The wificonfig.json failed to load
  {
    tft.println("WiFi Config Failed to load! Starting as AP.");
    Serial.println("[WARNING]: WiFi Config Failed to load! Configurator started as AP.");
    startDefaultAP();
    tft.println("Started as AP because WiFi settings failed to load.");
    tft.println("To configure, connect to 'FreeTouchDeck' with password 'defaultpass'");
    tft.println("Then go to http://freetouchdeck.local");
    tft.print("The IP is: ");
    tft.println(WiFi.softAPIP());
    return;
  }

  if (strcmp(wificonfig.wifimode, "WIFI_STA") == 0)
  {
    if(!startWifiStation()){
      startDefaultAP();
      Serial.println("[WARNING]: Could not connect to AP, so started as AP.");
      tft.println("Started as AP because WiFi connection failed.");
      tft.println("To configure, connect to 'FreeTouchDeck' with password 'defaultpass'");
      tft.println("Then go to http://freetouchdeck.local");
      tft.print("The IP is: ");
      tft.println(WiFi.softAPIP());
    }
    else
    {
      tft.println("Started as STA and in config mode.");
      tft.println("To configure:");
      tft.println("http://freetouchdeck.local");
      tft.print("The IP is: ");
      tft.println(WiFi.localIP());
    }

  }
  else if (strcmp(wificonfig.wifimode, "WIFI_AP") == 0)
  {
    startWifiAP();
    tft.println("Started as AP and in config mode.");
    tft.println("To configure:");
    tft.println("http://freetouchdeck.local");
    tft.print("The IP is: ");
    tft.println(WiFi.softAPIP());
  }
}


/**
* @brief This function allows for saving (updating) the WiFi SSID
*
* @param String ssid
*
* @return boolean True if succeeded. False otherwise.
*
* @note Returns true if successful. To enable the new set SSID, you must reload the the 
         configuration using loadMainConfig()
*/
bool saveWifiSSID(String ssid)
{

  FILESYSTEM.remove("/config/wificonfig.json");
  File file = FILESYSTEM.open("/config/wificonfig.json", "w");

  DynamicJsonDocument doc(384);

  JsonObject wificonfigobject = doc.to<JsonObject>();

  wificonfigobject["ssid"] = ssid;
  wificonfigobject["password"] = wificonfig.password;
  wificonfigobject["wifimode"] = wificonfig.wifimode;
  wificonfigobject["wifihostname"] = wificonfig.hostname;
  wificonfigobject["attempts"] = wificonfig.attempts;
  wificonfigobject["attemptdelay"] = wificonfig.attemptdelay;


  if (serializeJsonPretty(doc, file) == 0)
  {
    Serial.println("[WARNING]: Failed to write to file");
    return false;
  }
  file.close();
  return true;
}

/**
* @brief This function allows for saving (updating) the WiFi Password
*
* @param String password
*
* @return boolean True if succeeded. False otherwise.
*
* @note Returns true if successful. To enable the new set password, you must reload the the 
         configuration using loadMainConfig()
*/
bool saveWifiPW(String password)
{

  FILESYSTEM.remove("/config/wificonfig.json");
  File file = FILESYSTEM.open("/config/wificonfig.json", "w");

  DynamicJsonDocument doc(384);

  JsonObject wificonfigobject = doc.to<JsonObject>();

  wificonfigobject["ssid"] = wificonfig.ssid;
  wificonfigobject["password"] = password;
  wificonfigobject["wifimode"] = wificonfig.wifimode;
  wificonfigobject["wifihostname"] = wificonfig.hostname;
  wificonfigobject["attempts"] = wificonfig.attempts;
  wificonfigobject["attemptdelay"] = wificonfig.attemptdelay;


  if (serializeJsonPretty(doc, file) == 0)
  {
    Serial.println("[WARNING]: Failed to write to file");
    return false;
  }
  file.close();
  return true;
}

/**
* @brief This function allows for saving (updating) the WiFi Mode
*
* @param String wifimode "WIFI_STA" of "WIFI_AP"
*
* @return boolean True if succeeded. False otherwise.
*
* @note Returns true if successful. To enable the new set WiFi Mode, you must reload the the 
         configuration using loadMainConfig()
*/
bool saveWifiMode(String wifimode)
{

  if (wifimode != "WIFI_STA" && wifimode != "WIFI_AP")
  {
    Serial.println("[WARNING]: WiFi Mode not supported. Try WIFI_STA or WIFI_AP.");
    return false;
  }

  FILESYSTEM.remove("/config/wificonfig.json");
  File file = FILESYSTEM.open("/config/wificonfig.json", "w");

  DynamicJsonDocument doc(384);

  JsonObject wificonfigobject = doc.to<JsonObject>();

  wificonfigobject["ssid"] = wificonfig.ssid;
  wificonfigobject["password"] = wificonfig.password;
  wificonfigobject["wifimode"] = wifimode;
  wificonfigobject["wifihostname"] = wificonfig.hostname;
  wificonfigobject["attempts"] = wificonfig.attempts;
  wificonfigobject["attemptdelay"] = wificonfig.attemptdelay;


  if (serializeJsonPretty(doc, file) == 0)
  {
    Serial.println("[WARNING]: Failed to write to file");
    return false;
  }
  file.close();
  return true;
}

/**
* @brief This function checks if a file exists and returns a boolean accordingly.
         It then prints a debug message to the serial as wel as the tft.
*
* @param filename (const char *)
*
* @return boolean True if succeeded. False otherwise.
*
* @note Pass the filename including a leading /
*/
bool checkfile(const char *filename)
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
  else
  {
    return true;
  }
}

bool resetconfig(String file){

 if (file != "menu1" && file != "menu2" && file != "menu3" && file != "menu4" && file != "menu5" && file != "homescreen" && file != "general")
  {
    Serial.println("[WARNING]: Invalid reset option. Choose: menu1, menu2, menu3, menu4, menu5, homescreen, or general");
    return false;
  }

 if (file == "menu1" || file == "menu2" || file == "menu3" || file == "menu4" || file == "menu5")
 {
   // Reset a menu config
  
  
   // Delete the corrupted json file
   String filetoremove = "/config/" + file;
   if(!filetoremove.endsWith(".json")){
    filetoremove = filetoremove + ".json";
   }
   
   FILESYSTEM.remove(filetoremove);
   
   // Copy default.json to the new config file
   File defaultfile = FILESYSTEM.open("/config/default.json", "r");
  
    size_t n; 
    uint8_t buf[64];
  
     if (defaultfile) {
      File newfile = FILESYSTEM.open(filetoremove, "w");
      if (newfile) {
        while ((n = defaultfile.read(buf, sizeof(buf))) > 0) {
          newfile.write(buf, n);
        }
        // Close the newly created file
        newfile.close();
      }
      Serial.println("[INFO]: Done resetting.");
      Serial.println("[INFO]: Type \"restart\" to reload configuration.");
      
      // Close the default.json file
      defaultfile.close();
       return true;
      } 
      
    }  
    else if(file == "homescreen")
    {

    // Reset the homescreen
    // For this we do not need to open a default file because we can easily write it ourselfs
    String filetoremove = "/config/" + file;
    if(!filetoremove.endsWith(".json")){
      filetoremove = filetoremove + ".json";
    }
    
    FILESYSTEM.remove(filetoremove);
    
    File newfile = FILESYSTEM.open(filetoremove, "w");
    newfile.println("{");
    newfile.println("\"logo0\": \"question.bmp\",");
    newfile.println("\"logo1\": \"question.bmp\",");
    newfile.println("\"logo2\": \"question.bmp\",");
    newfile.println("\"logo3\": \"question.bmp\",");
    newfile.println("\"logo4\": \"question.bmp\",");
    newfile.println("\"logo5\": \"settings.bmp\"");
    newfile.println("}");

    newfile.close();
    Serial.println("[INFO]: Done resetting homescreen.");
    Serial.println("[INFO]: Type \"restart\" to reload configuration.");
    return true;
      
    }
    else if(file == "general")
    {

      // Reset the general config
      // For this we do not need to open a default file because we can easily write it ourselfs

      String filetoremove = "/config/" + file;
    if(!filetoremove.endsWith(".json")){
      filetoremove = filetoremove + ".json";
    }
    
    FILESYSTEM.remove(filetoremove);
    
    File newfile = FILESYSTEM.open(filetoremove, "w");
    newfile.println("{");
    newfile.println("\"menubuttoncolor\": \"#009bf4\",");
    newfile.println("\"functionbuttoncolor\": \"#00efcb\",");
    newfile.println("\"latchcolor\": \"#fe0149\",");
    newfile.println("\"background\": \"#000000\",");
    newfile.println("\"sleepenable\": true,");
    newfile.println("\"sleeptimer\": 10,");
    newfile.println("\"beep\": true,");
    newfile.println("\"modifier1\": 130,");
    newfile.println("\"modifier2\": 129,");
    newfile.println("\"modifier3\": 0,");
    newfile.println("\"helperdelay\": 500");
    newfile.println("}");

    newfile.close();
    Serial.println("[INFO]: Done resetting general config.");
    Serial.println("[INFO]: Type \"restart\" to reload configuration.");
    return true;
      
    }
    else
    {
      return false;
    }

  return false;

}
