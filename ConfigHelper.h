/**
* @brief This function start the WiFi Connection. Return true if success.
*
* @param Wificonfig
*
* @return bool
*
* @note none
*/
bool startWifiConnection(Wificonfig &wificonfig)
{
  int timeout = 10;
  if (String(wificonfig.ssid) == "YOUR_WIFI_SSID" || String(wificonfig.password) == "YOUR_WIFI_PASSWORD") // Still default
  {
    drawErrorMessage("WiFi Config still set to default!");
    Serial.println("[ERROR]: WiFi Config still set to default!");
    return false;
  }

  if (String(wificonfig.ssid) == FAILED || String(wificonfig.password) == FAILED || String(wificonfig.wifimode) == FAILED) // The config.json failed to load
  {
    drawErrorMessage("WiFi Config Failed to load!");
    Serial.println("[ERROR]: WiFi Config Failed to load!");
    return false;
  }

  drawErrorMessage("Connecting to Wifi...");

  Serial.printf("[INFO]: Connecting to %s", wificonfig.ssid);
  if (String(WiFi.SSID()) != String(wificonfig.ssid))
  {
    if (strcmp(wificonfig.wifimode, "WIFI_STA") == 0)
    {
      WiFi.mode(WIFI_STA);
      WiFi.begin(wificonfig.ssid, wificonfig.password);
      while (WiFi.status() != WL_CONNECTED)
      {
        if(timeout == 0) {
          WiFi.disconnect();
          return false;
        }
        delay(500);
        Serial.print(".");
        timeout--;
      }
      Serial.println("");
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
    }
  }

  return true;
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

  Serial.println("[INFO]: Entering Config Mode");

  // Delete the task bleKeyboard had create to free memory and to not interfere with AsyncWebServer
  bleKeyboard.end();

  // Stop BLE from interfering with our WIFI signal
  btStop();
  esp_bt_controller_disable();
  esp_bt_controller_deinit();
  esp_bt_controller_mem_release(ESP_BT_MODE_BTDM);

  Serial.println("[INFO]: BLE Stopped");

  if(startWifiConnection(systemconfig.wificonfig) == false) {
    Serial.println("[WARNING]: Failed to connect to the default WiFi network");
    if(startWifiConnection(systemconfig.wificonfigfallback) == false) {
      Serial.println("[ERROR]: Failed to connect to the fallback WiFi network");
      return;
    }
  }

  MDNS.begin(systemconfig.hostname);
  MDNS.addService("http", "tcp", 80);

  // Set pageNum to 7 so no buttons are displayed and touches are ignored
  pageNum = 7;

  // Start the webserver
  webserver.begin();
  Serial.println("[INFO]: Webserver started");
}

JsonObject wifiConfigToJSONObject(Wificonfig &wificonfig)
{
  DynamicJsonDocument doc(CONFIG_JSON_SIZE);
  
  JsonObject configobject = doc.to<JsonObject>();

  configobject["ssid"] = wificonfig.ssid;
  configobject["wifimode"] = wificonfig.wifimode;
  configobject["password"] = wificonfig.password;

  return configobject;
}

bool saveConfig()
{
  Serial.println("[INFO]: Saving Config");
  DynamicJsonDocument doc(CONFIG_JSON_SIZE);
  DynamicJsonDocument wifidoc(CONFIG_JSON_SIZE);
  
  JsonObject configobject = doc.to<JsonObject>();

  configobject["sleepenable"] = systemconfig.sleepenable;
  configobject["sleeptimer"] = systemconfig.sleeptimer;
  configobject["hostname"] = systemconfig.hostname;

  JsonObject wificonfigobject = wifidoc.to<JsonObject>();
  wificonfigobject["default"] = wifiConfigToJSONObject(systemconfig.wificonfig);
  wificonfigobject["fallback"] = wifiConfigToJSONObject(systemconfig.wificonfigfallback);
  configobject["wifi"] = wificonfigobject;

  File file = FILESYSTEM.open(CONFIG_FILE_PATH, "w");

  if (serializeJsonPretty(doc, file) == 0)
  {
    Serial.println("[WARNING]: Failed to write to file");
    return false;
  }

  serializeJsonPretty(doc, Serial);

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
