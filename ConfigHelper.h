// Start as WiFi station


#include "globals.hpp"
#include "Webserver.h"
void stopBT()
{
  PrintMemInfo();
  ESP_LOGD(module,"Terminating BLE Keyboard");
    // Delete the task bleKeyboard had create to free memory and to not interfere with AsyncWebServer
  bleKeyboard.end();
  PrintMemInfo();
  ESP_LOGD(module,"Terminating BT");
  // Stop BLE from interfering with our WIFI signal
  btStop();
  // esp_bt_controller_disable();
  // esp_bt_controller_deinit();
  // esp_bt_controller_mem_release(ESP_BT_MODE_BTDM);
  ESP_LOGI(module, "BLE Stopped");
}
bool startWebServer()
{

  if (WiFi.getMode() == WIFI_MODE_STA && WiFi.isConnected())
  {
    ESP_LOGI(module, "Connected to Access point! IP address: %s", WiFi.localIP().toString().c_str());
  }
  else if (WiFi.getMode() == WIFI_MODE_AP)
  {
    ESP_LOGI(module, "Access point ready. Address is : %s", WiFi.softAPIP().toString().c_str());
  }
  if(!MDNS.begin(STRING_OR_DEFAULT(wificonfig.hostname, "freetouchdeck")) || !MDNS.addService("http", "tcp", 80))
  {
    PrintMemInfo();
    drawErrorMessage(true,module,"Unable to start MDNS server");

  }
  PrintMemInfo();
  ESP_LOGD(module,"MDNS started");

  // ----------------- Load webserver ---------------------
  handlerSetup();
  PrintMemInfo();
  ESP_LOGD(module,"Http handlers configured, starting web server");
  // Start the webserver
  webserver.begin();
  
  ESP_LOGI(module, "Webserver started");
  return true;
}
bool startWifiStation()
{
  ESP_LOGI(module, "Connecting to %s", wificonfig.ssid);
  if (String(WiFi.SSID()) != String(wificonfig.ssid))
  {
    WiFi.mode(WIFI_STA);
    WiFi.begin(wificonfig.ssid, wificonfig.password);
    uint8_t attempts = wificonfig.attempts;
    while (WiFi.status() != WL_CONNECTED)
    {
      if (attempts == 0)
      {
        ESP_LOGW(module, "Unable to connect to WiFi %s", wificonfig.ssid);
        WiFi.disconnect();
        return false;
      }
      delay(wificonfig.attemptdelay);

      attempts--;
      ESP_LOGI(module, "WiFi connect waiting %d seconds", attempts * wificonfig.attemptdelay / 1000);
    }
  }

  startWebServer();
  ESP_LOGD(module, "Done Starting server in STA mode");
  return true;
}

// Start as WiFi AP

// Start the default AP
bool startAP(const char *SSID, const char *PASSWORD)
{
  if (!WiFi.mode(WIFI_AP))
  {
    ESP_LOGE(module, "Unable to set WiFi mode to access point");
  }else if (!WiFi.softAP(SSID, PASSWORD))
  {
    ESP_LOGE(module, "Unable to start Wifi Access Point");
  }
  else
  {
    startWebServer();
    ESP_LOGD(module, "Done Starting Wifi in Access Point mode");
    return true;
  }
  
  return false;
}

bool startDefaultAP()
{
  static const char *ssid = "FreeTouchDeck";
  static const char *password = "defaultpass";
  ESP_LOGD(module,"Starting default AP");
  return startAP(ssid, password);
}
bool startWifiAP()
{
  return startAP(wificonfig.ssid, wificonfig.password);
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
bool ConfigMode()
{

  bool result = false;
  //tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 0);
  tft.setTextFont(2);
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  ESP_LOGI(module, "Entering Config Mode");
  tft.println("Connecting to Wifi...");
  PrintMemInfo();

  if (String(wificonfig.ssid) == "YOUR_WIFI_SSID" || String(wificonfig.password) == "YOUR_WIFI_PASSWORD") // Still default
  {
    
    tft.println("WiFi Config still set to default! Starting as AP.");
    ESP_LOGW(module, "WiFi Config still set to default! Configurator started as AP.");
    if (!startDefaultAP())
    {
      drawErrorMessage(true, module, "Unale to start config Access Point");
    }
    else
    {
      ESP_LOGD(module,"Default AP Started. Printing instructions to screen");
      tft.println("Started as AP because WiFi settings are still set to default.");
      tft.println("To configure, connect to 'FreeTouchDeck' with password 'defaultpass'");
      tft.println("Then go to http://freetouchdeck.local");
      tft.print("The IP is: ");
      tft.println(WiFi.softAPIP().toString().c_str());
      result = true;
    }
  }
  else if (String(wificonfig.ssid) == "FAILED" || String(wificonfig.password) == "FAILED" || String(wificonfig.wifimode) == "FAILED") // The wificonfig.json failed to load
  {
    tft.println("WiFi Config Failed to load! Starting as AP.");
    ESP_LOGW(module, "WiFi Config Failed to load! Configurator started as AP.");
    if (!startDefaultAP())
    {
      drawErrorMessage(true, module, "Unale to start config Access Point");
    }
    else
    {
      ESP_LOGD(module,"Default AP Started. Printing instructions to screen");
      tft.println("Started as AP because WiFi settings failed to load.");
      tft.println("To configure, connect to 'FreeTouchDeck' with password 'defaultpass'");
      tft.println("Then go to http://freetouchdeck.local");
      tft.print("The IP is: ");
      tft.println(WiFi.softAPIP().toString().c_str());
      result = true;
    }
  }
  else if (strcmp(wificonfig.wifimode, "WIFI_STA") == 0)
  {
    if (!startWifiStation())
    {
      if (!startDefaultAP())
      {
        drawErrorMessage(true, module, "Unale to fallback to Access Point mode");
      }
      else
      {
        ESP_LOGW(module, "Could not connect to AP, so started as AP.");
        tft.println("Started as AP because WiFi connection failed.");
        tft.println("To configure, connect to 'FreeTouchDeck' with password 'defaultpass'");
        tft.println("Then go to http://freetouchdeck.local");
        tft.print("The IP is: ");
        tft.println(WiFi.softAPIP().toString().c_str());
        result = true;
      }
    }
    else
    {
      ESP_LOGD(module,"Connected to Wifi. Printing instructions to screen");
      tft.println("Started as STA and in config mode.");
      tft.println("To configure:");
      tft.println("http://freetouchdeck.local");
      tft.print("The IP is: ");
      tft.println(WiFi.localIP());
      result = true;
    }
  }
  else if (strcmp(wificonfig.wifimode, "WIFI_AP") == 0)
  {
    if (startWifiAP())
    {
      ESP_LOGD(module,"Done starting wifi in AP mode. Printing instructions to screen");
      tft.println("Started as AP and in config mode.");
      tft.println("To configure:");
      tft.println("http://freetouchdeck.local");
      tft.print("The IP is: ");
      tft.println(WiFi.softAPIP().toString().c_str());
      result = true;
    }
  }
  return result;
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
    ESP_LOGW(module, "Failed to write to file");
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
    ESP_LOGW(module, "Failed to write to file");
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
    ESP_LOGW(module, "WiFi Mode not supported. Try WIFI_STA or WIFI_AP.");
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
    ESP_LOGW(module, "Failed to write to file");
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

bool resetconfig(String file)
{

  if (file != "menu1" && file != "menu2" && file != "menu3" && file != "menu4" && file != "menu5" && file != "homescreen" && file != "general")
  {
    ESP_LOGW(module, "Invalid reset option. Choose: menu1, menu2, menu3, menu4, menu5, homescreen, or general");
    return false;
  }

  if (file == "menu1" || file == "menu2" || file == "menu3" || file == "menu4" || file == "menu5")
  {
    // Reset a menu config

    // Delete the corrupted json file
    String filetoremove = "/config/" + file;
    if (!filetoremove.endsWith(".json"))
    {
      filetoremove = filetoremove + ".json";
    }

    FILESYSTEM.remove(filetoremove);

    // Copy default.json to the new config file
    File defaultfile = FILESYSTEM.open("/config/default.json", "r");

    size_t n;
    uint8_t buf[64];

    if (defaultfile)
    {
      File newfile = FILESYSTEM.open(filetoremove, "w");
      if (newfile)
      {
        while ((n = defaultfile.read(buf, sizeof(buf))) > 0)
        {
          newfile.write(buf, n);
        }
        // Close the newly created file
        newfile.close();
      }
      ESP_LOGI(module, "Done resetting.");
      ESP_LOGI(module, "Type \"restart\" to reload configuration.");

      // Close the default.json file
      defaultfile.close();
      return true;
    }
  }
  else if (file == "homescreen")
  {

    // Reset the homescreen
    // For this we do not need to open a default file because we can easily write it ourselfs
    String filetoremove = "/config/" + file;
    if (!filetoremove.endsWith(".json"))
    {
      filetoremove = filetoremove + ".json";
    }

    FILESYSTEM.remove(filetoremove);

    File newfile = FILESYSTEM.open(filetoremove, "w");
    newfile.print(R"(
{
"logo0: "question.bmp",
"logo1: "question.bmp",
"logo2: "question.bmp",
"logo3: "question.bmp",
"logo4: "question.bmp",
"logo5: "settings.bmp"
})");

    newfile.close();
    ESP_LOGI(module, "Done resetting homescreen.");
    ESP_LOGI(module, "Type \"restart\" to reload configuration.");
    return true;
  }
  else if (file == "general")
  {

    // Reset the general config
    // For this we do not need to open a default file because we can easily write it ourselfs

    String filetoremove = "/config/" + file;
    if (!filetoremove.endsWith(".json"))
    {
      filetoremove = filetoremove + ".json";
    }

    FILESYSTEM.remove(filetoremove);

    File newfile = FILESYSTEM.open(filetoremove, "w");
    newfile.print(R"(
{
  "menubuttoncolor": "#009bf4",
  "functionbuttoncolor": #00efcb,
  "latchcolor: #fe0149,
  "background: #000000,
  "sleepenable: true,
  "sleeptimer: 10,
  "beep: true,
  "modifier1: 130,
  "modifier2: 129,
  "modifier3: 0,
  "helperdelay: 500
})");
    newfile.close();
    ESP_LOGI(module, "Done resetting general config.");
    ESP_LOGI(module, "Type \"restart\" to reload configuration.");
    return true;
  }
  else
  {
    return false;
  }

  return false;
}
