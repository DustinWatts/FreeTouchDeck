// Start as WiFi station

#include "ConfigHelper.h"
#include "globals.hpp"
#include "Webserver.h"
#include <WiFi.h>    // Wifi support
#include <ESPmDNS.h> // DNS functionality
#include "ConfigLoad.h"
#include <AsyncTCP.h> //Async Webserver support header
#include "Webserver.h"
#include "WString.h"
#include "Storage.h"
namespace FreeTouchDeck
{
  static const char *module = "ConfigHelper";
  AsyncWebServer webserver(80);
  Wificonfig wificonfig;
  using namespace fs;
  using namespace std;
  
  void stopBT()
  {
    PrintMemInfo(__FUNCTION__, __LINE__);
    LOC_LOGD(module, "Terminating BLE Keyboard");
    // Delete the task bleKeyboard had create to free memory and to not interfere with AsyncWebServer
    StopBluetooth();
    PrintMemInfo(__FUNCTION__, __LINE__);

  }

  bool startWebServer()
  {

    if (WiFi.getMode() == WIFI_MODE_STA && WiFi.isConnected())
    {
      LOC_LOGI(module, "Connected to Access point! IP address: %s", WiFi.localIP().toString().c_str());
    }
    else if (WiFi.getMode() == WIFI_MODE_AP)
    {
      LOC_LOGI(module, "Access point ready. Address is : %s", WiFi.softAPIP().toString().c_str());
    }
    if (!MDNS.begin(STRING_OR_DEFAULT(wificonfig.hostname, "freetouchdeck")) || !MDNS.addService("http", "tcp", 80))
    {
      PrintMemInfo(__FUNCTION__, __LINE__);
      drawErrorMessage(true, module, "Unable to start MDNS server");
    }
    PrintMemInfo(__FUNCTION__, __LINE__);
    LOC_LOGD(module, "MDNS started");

    // ----------------- Load webserver ---------------------
    handlerSetup();
    PrintMemInfo(__FUNCTION__, __LINE__);
    LOC_LOGD(module, "Http handlers configured, starting web server");
    // Start the webserver
    webserver.begin();

    LOC_LOGI(module, "Webserver started");
    return true;
  }

  bool startWifiStation()
  {
    LOC_LOGI(module, "Connecting to %s", wificonfig.ssid);
    if (String(WiFi.SSID()) != String(wificonfig.ssid))
    {
      WiFi.mode(WIFI_STA);
      WiFi.begin(wificonfig.ssid, wificonfig.password);
      uint8_t attempts = wificonfig.attempts;
      while (WiFi.status() != WL_CONNECTED)
      {
        if (attempts == 0)
        {
          LOC_LOGW(module, "Unable to connect to WiFi %s", wificonfig.ssid);
          WiFi.disconnect();
          return false;
        }
        delay(wificonfig.attemptdelay);

        attempts--;
        LOC_LOGI(module, "WiFi connect waiting %d seconds", attempts * wificonfig.attemptdelay / 1000);
      }
    }

    startWebServer();
    LOC_LOGD(module, "Done Starting server in STA mode");
    return true;
  }

  // Start as WiFi AP

  // Start the default AP
  bool startAP(const char *SSID, const char *PASSWORD)
  {
    if (!WiFi.mode(WIFI_AP))
    {
      LOC_LOGE(module, "Unable to set WiFi mode to access point");
    }
    else if (!WiFi.softAP(SSID, PASSWORD))
    {
      LOC_LOGE(module, "Unable to start Wifi Access Point");
    }
    else
    {
      startWebServer();
      LOC_LOGD(module, "Done Starting Wifi in Access Point mode");
      return true;
    }

    return false;
  }

  bool startDefaultAP()
  {
    static const char *ssid = "FreeTouchDeck";
    static const char *password = "defaultpass";
    LOC_LOGD(module, "Starting default AP");
    return startAP(ssid, password);
  }
  bool startWifiAP()
  {
    return startAP(wificonfig.ssid, wificonfig.password);
  }

  bool ConfigMode()
  {

    bool result = false;
    SetSmallestFont(1);
    tft.setTextSize(1);
    tft.setCursor(0, tft.fontHeight() + 1);
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    LOC_LOGI(module, "Entering Config Mode");
    tft.println("Connecting to Wifi...");
    PrintMemInfo(__FUNCTION__, __LINE__);

    if (String(wificonfig.ssid) == "YOUR_WIFI_SSID" || String(wificonfig.password) == "YOUR_WIFI_PASSWORD") // Still default
    {

      tft.println("WiFi Config still set to default! Starting as AP.");
      LOC_LOGW(module, "WiFi Config still set to default! Configurator started as AP.");
      if (!startDefaultAP())
      {
        drawErrorMessage(true, module, "Unale to start config Access Point");
      }
      else
      {
        LOC_LOGD(module, "Default AP Started. Printing instructions to screen");
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
      LOC_LOGW(module, "WiFi Config Failed to load! Configurator started as AP.");
      if (!startDefaultAP())
      {
        drawErrorMessage(true, module, "Unale to start config Access Point");
      }
      else
      {
        LOC_LOGD(module, "Default AP Started. Printing instructions to screen");
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
          LOC_LOGW(module, "Could not connect to AP, so started as AP.");
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
        LOC_LOGD(module, "Connected to Wifi. Printing instructions to screen");
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
        LOC_LOGD(module, "Done starting wifi in AP mode. Printing instructions to screen");
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
         configuration using loadWifiConfig()
*/
  bool saveWifiSSID(String ssid)
  {
    FREE_AND_NULL(wificonfig.ssid);
    wificonfig.ssid = ps_strdup(ssid.c_str());
    return Commit(&wificonfig);
  }

  bool saveWifiPW(String password)
  {
    FREE_AND_NULL(wificonfig.password);
    wificonfig.ssid = ps_strdup(password.c_str());
    return Commit(&wificonfig);
  }

  bool saveWifiMode(String wifimode)
  {

    if (wifimode != "WIFI_STA" && wifimode != "WIFI_AP")
    {
      LOC_LOGW(module, "WiFi Mode not supported. Try WIFI_STA or WIFI_AP.");
      return false;
    }
    FREE_AND_NULL(wificonfig.wifimode);
    wificonfig.ssid = ps_strdup(wifimode.c_str());
    return Commit(&wificonfig);
  }

  bool resetconfig(String file)
  {

    if (file != "menus" && file != "general")
    {
      LOC_LOGW(module, "Invalid reset option. Choose: menus or general");
      return false;
    }

    if (file == "menus")
    {
      // Reset menu config
      // Delete the json file

      ftdfs->remove("/config/menus.json");
      if (!CopyFile("/config/default.json", "/config/menus.json", ftdfs, ftdfs))
      {
        LOC_LOGE(module, "Error copying default menu structure");
        return false;
      }
      return true;
    }
    else if (file == "general")
    {
      ftdfs->remove("/config/general.json");
      SetGeneralConfigDefaults();
      saveConfig(false);
      LOC_LOGI(module, "Done resetting general configuration.");
      LOC_LOGI(module, "Type \"restart\" to reload configuration.");
      return true;
    }
    else
    {
      return false;
    }

    return false;
  }
  cJSON *toJson(Wificonfig *wificonfig)
  {
    cJSON *doc = cJSON_CreateObject();
    cJSON_AddStringToObject(doc, "ssid", wificonfig->ssid);
    cJSON_AddStringToObject(doc, "password", wificonfig->password);
    cJSON_AddStringToObject(doc, "wifimode", wificonfig->wifimode);
    cJSON_AddStringToObject(doc, "wifihostname", wificonfig->hostname);
    cJSON_AddNumberToObject(doc, "attempts", wificonfig->attempts);
    cJSON_AddNumberToObject(doc, "attemptdelay", wificonfig->attemptdelay);
    return doc;
  }
  bool Commit(Wificonfig *wificonfig)
  {
    cJSON *doc = toJson(wificonfig);
    if (doc)
    {
      return SaveJsonToFile("/config/wificonfig.json", doc, true);
    }
    else
    {
      LOC_LOGE(module, "Unable to save wifi configuration");
    }
    return false;
  }
  void EraseWifiConfig(Wificonfig *config)
  {
    FREE_AND_NULL(config->ssid);
    FREE_AND_NULL(config->wifimode);
    FREE_AND_NULL(config->password);
    FREE_AND_NULL(config->hostname);
    config->attemptdelay = 0;
    config->attempts = 0;
  }
}
