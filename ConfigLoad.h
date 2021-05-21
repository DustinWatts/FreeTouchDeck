#include "globals.hpp"
#include "FTAction.h"
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
    ESP_LOGD(module,"Config file not found!");
    return false;
  }
  File configfile = FILESYSTEM.open("/config/wificonfig.json");

  DynamicJsonDocument doc(256);

  DeserializationError error = deserializeJson(doc, configfile);
  FREE_AND_NULL(wificonfig.ssid);
  FREE_AND_NULL(wificonfig.password);
  FREE_AND_NULL(wificonfig.wifimode);
  FREE_AND_NULL(wificonfig.hostname);

  wificonfig.ssid = ps_strdup( doc["ssid"] | "FAILED");
  wificonfig.password = ps_strdup( doc["password"] | "FAILED");
  wificonfig.wifimode = ps_strdup( doc["wifimode"] | "FAILED");
  wificonfig.hostname=ps_strdup(doc["wifihostname"] | "freetouchdeck");
  wificonfig.attempts = (uint8_t)doc["attempts"] | 10 ;
  wificonfig.attemptdelay = (uint16_t)doc["attemptdelay"] | 500 ;
  configfile.close();

  if (error)
  {
    ESP_LOGE(module,"deserializeJson() error: %s",error.c_str());
    return false;
  }
  return true;
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
bool loadConfig()
{

  File configfile = FILESYSTEM.open("/config/general.json", "r");

  DynamicJsonDocument doc(384);

  DeserializationError error = deserializeJson(doc, configfile);

  // Parsing colors
  // Get the color for the menu and back home buttons.
  generalconfig.menuButtonColour = convertRGB888ToRGB565(convertHTMLtoRGB888(doc["menubuttoncolor"] | "#009bf4"));
  // Get the color for the function buttons.
  generalconfig.functionButtonColour = convertRGB888ToRGB565(convertHTMLtoRGB888(doc["functionbuttoncolor"] | "#00efcb"));
  // Get the color to use when latching.
  generalconfig.latchedColour = convertRGB888ToRGB565(convertHTMLtoRGB888(doc["latchcolor"] | "#fe0149"));
  // Get the color for the background.
  generalconfig.backgroundColour = convertRGB888ToRGB565(convertHTMLtoRGB888(doc["background"] | "#000000"));

  // Loading general settings

  bool sleepenable = doc["sleepenable"] | false;
  if (sleepenable)
  {
    generalconfig.sleepenable = true;
  }
  else
  {
    generalconfig.sleepenable = false;
  }
  generalconfig.sleeptimer = (uint16_t)doc["sleeptimer"] | 60;
  generalconfig.beep = (bool)doc["beep"] | false;
  generalconfig.modifier1 = (uint8_t)doc["modifier1"] | 0;
  generalconfig.modifier2 = (uint8_t)doc["modifier2"] | 0;
  generalconfig.modifier3 = (uint8_t)doc["modifier3"] | 0;
  generalconfig.helperdelay = (uint16_t)doc["helperdelay"] | 250;

  configfile.close();

  if (error)
  {
    ESP_LOGE(module,"deserializeJson() error: %s",error.c_str());
    return false;
  }
  return true;
}
