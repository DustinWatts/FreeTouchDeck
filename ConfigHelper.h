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
bool configmode()
{

  Serial.println("[INFO]: Entering Config Mode");

  if (String(wificonfig.ssid) == "YOUR_WIFI_SSID" || String(wificonfig.password) == "YOUR_WIFI_PASSWORD") // Still default
  {
    drawErrorMessage("WiFi Config still set to default!");
    Serial.println("[ERROR]: WiFi Config still set to default!");
    while (1)
      yield(); // Stop!
  }

  if (String(wificonfig.ssid) == "FAILED" || String(wificonfig.password) == "FAILED" || String(wificonfig.wifimode) == "FAILED") // The wificonfig.json failed to load
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
    if (strcmp(wificonfig.wifimode, "WIFI_STA") == 0)
    {
      WiFi.mode(WIFI_STA);
      WiFi.begin(wificonfig.ssid, wificonfig.password);
      uint8_t attempts = 10;
      while (WiFi.status() != WL_CONNECTED)
      {
        if(attempts == 0) {
          WiFi.disconnect();
          Serial.println("");
          Serial.printf("[WARNING]: Could not connect to: %s\n", wificonfig.ssid);
          return false;
        }
        delay(500);
        Serial.print(".");
        attempts--;
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
    }
    else if (strcmp(wificonfig.wifimode, "WIFI_AP") == 0)
    {
      
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
    }
  }

  MDNS.begin(wificonfig.hostname);
  MDNS.addService("http", "tcp", 80);

  // Set pageNum to 7 so no buttons are displayed and touches are ignored
  pageNum = 7;

  // Start the webserver
  webserver.begin();
  Serial.println("[INFO]: Webserver started");
  return true;
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

  DynamicJsonDocument doc(256);

  JsonObject wificonfigobject = doc.to<JsonObject>();

  wificonfigobject["ssid"] = ssid;
  wificonfigobject["password"] = wificonfig.password;
  wificonfigobject["wifimode"] = wificonfig.wifimode;
  wificonfigobject["wifihostname"] = wificonfig.hostname;
  wificonfigobject["sleepenable"] = wificonfig.sleepenable;
  wificonfigobject["sleeptimer"] = wificonfig.sleeptimer;
  wificonfigobject["beep"] = wificonfig.beep;

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

  DynamicJsonDocument doc(256);

  JsonObject wificonfigobject = doc.to<JsonObject>();

  wificonfigobject["ssid"] = wificonfig.ssid;
  wificonfigobject["password"] = password;
  wificonfigobject["wifimode"] = wificonfig.wifimode;
  wificonfigobject["wifihostname"] = wificonfig.hostname;
  wificonfigobject["sleepenable"] = wificonfig.sleepenable;
  wificonfigobject["sleeptimer"] = wificonfig.sleeptimer;
  wificonfigobject["beep"] = wificonfig.beep;

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

  DynamicJsonDocument doc(256);

  JsonObject wificonfigobject = doc.to<JsonObject>();

  wificonfigobject["ssid"] = wificonfig.ssid;
  wificonfigobject["password"] = wificonfig.password;
  wificonfigobject["wifimode"] = wifimode;
  wificonfigobject["wifihostname"] = wificonfig.hostname;
  wificonfigobject["sleepenable"] = wificonfig.sleepenable;
  wificonfigobject["sleeptimer"] = wificonfig.sleeptimer;
  wificonfigobject["beep"] = wificonfig.beep;

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

 if (file != "menu1" && file != "menu2" && file != "menu3" && file != "menu4" && file != "menu5" && file != "homescreen" && file != "colors")
  {
    Serial.println("[WARNING]: Invalid reset option. Choose: menu1, menu2, menu3, menu4, menu5, homescreen, or colors");
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
    else if(file == "colors")
    {

      // Reset the colors
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
    newfile.println("\"background\": \"#000000\"");
    newfile.println("}");

    newfile.close();
    Serial.println("[INFO]: Done resetting colors.");
    Serial.println("[INFO]: Type \"restart\" to reload configuration.");
    return true;
      
    }
    else
    {
      return false;
    }

  return false;

}
