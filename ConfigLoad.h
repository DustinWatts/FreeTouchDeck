#include "globals.hpp"
#include "FTAction.h"
void GetValueOrDefault(cJSON *value, char **valuePointer, const char *defaultValue)
{
  char *tempValue = NULL;
  FREE_AND_NULL((*valuePointer));
  if (value && cJSON_IsString(value))
  {
    tempValue = cJSON_GetStringValue(value);
    if (tempValue && strlen(tempValue) > 0)
    {
      (*valuePointer) = ps_strdup(tempValue);
    }
  }
  if (!(*valuePointer))
  {
    (*valuePointer) = ps_strdup(defaultValue);
  }
}
void GetValueOrDefault(cJSON *value, uint16_t *valuePointer, uint16_t defaultValue)
{
  uint16_t tempValue = 0;
  if (value && cJSON_IsNumber(value))
  {
    (*valuePointer) = (uint16_t)value->valuedouble;
  }
  else
  {
    (*valuePointer) = defaultValue;
  }
}
void GetValueOrDefault(cJSON *value, uint8_t *valuePointer, uint8_t defaultValue)
{
  uint8_t tempValue = 0;
  if (value && cJSON_IsNumber(value))
  {
    (*valuePointer) = (uint8_t)value->valueint;
  }
  else
  {
    (*valuePointer) = defaultValue;
  }
}
void GetValueOrDefault(cJSON *value, bool *valuePointer, bool defaultValue)
{
  bool tempValue = 0;
  if (value && cJSON_IsBool(value))
  {
    (*valuePointer) = cJSON_IsTrue(value);
  }
  else
  {
    (*valuePointer) = defaultValue;
  }
}

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
  char buffer[501] = {0};
  if (!FILESYSTEM.exists("/config/wificonfig.json"))
  {
    ESP_LOGD(module, "Config file not found!");
    return false;
  }
  File configfile = FILESYSTEM.open("/config/wificonfig.json");
  if(!configfile && configfile.size()==0)
  {
    ESP_LOGE(module,"Could not find file /config/wificonfig.json");
    return false;
  }  
  size_t bytesRead = configfile.readBytes(buffer, sizeof(buffer));
  if (bytesRead != configfile.size())
  {
    drawErrorMessage(false, module, "Could not fully read wifi config in buffer. Read %d of %d bytes.", bytesRead, configfile.size());
    return false;
  }
  configfile.close();
  cJSON *doc = cJSON_Parse(buffer);
  if (!doc)
  {
    const char *error = cJSON_GetErrorPtr();
    drawErrorMessage(false, module, "Unable to parse json string : %s", error);
    return false;
  }
  else
  {
    GetValueOrDefault(cJSON_GetObjectItem(doc, "ssid"), &wificonfig.ssid, "FAILED");
    GetValueOrDefault(cJSON_GetObjectItem(doc, "password"), &wificonfig.password, "FAILED");
    GetValueOrDefault(cJSON_GetObjectItem(doc, "wifimode"), &wificonfig.wifimode, "FAILED");
    GetValueOrDefault(cJSON_GetObjectItem(doc, "wifihostname"), &wificonfig.hostname, "freetouchdeck");
    GetValueOrDefault(cJSON_GetObjectItem(doc, "attempts"), &wificonfig.attempts, 10);
    GetValueOrDefault(cJSON_GetObjectItem(doc, "attemptdelay"), &wificonfig.attemptdelay, 500);
    cJSON_Delete(doc);
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
  char buffer[501] = {0};
  File configfile = FILESYSTEM.open("/config/general.json", "r");
  if(!configfile && configfile.size()==0)
  {
    ESP_LOGE(module,"Could not find file /config/general.json");
    return false;
  }

  size_t bytesRead = configfile.readBytes(buffer, sizeof(buffer));
  if (bytesRead != configfile.size())
  {
    drawErrorMessage(false, module, "Could not fully read general.json config in buffer. Read %d of %d bytes.", bytesRead, configfile.size());
    return false;
  }
  configfile.close();
  cJSON *doc = cJSON_Parse(buffer);
  if (!doc)
  {
    const char *error = cJSON_GetErrorPtr();
    drawErrorMessage(false, module, "Unable to parse json string : %s", error);
    return false;
  }

  // Parsing colors
  // Get the color for the menu and back home buttons.
  char * valBuffer=NULL;
  GetValueOrDefault(cJSON_GetObjectItem(doc, "menubuttoncolor"), &valBuffer, "#009bf4");
  generalconfig.menuButtonColour = convertRGB888ToRGB565(convertHTMLtoRGB888(valBuffer));

  // Get the color for the function buttons.
  GetValueOrDefault(cJSON_GetObjectItem(doc, "functionbuttoncolor"), &valBuffer, "#00efcb");
  generalconfig.functionButtonColour = convertRGB888ToRGB565(convertHTMLtoRGB888(valBuffer));

  // Get the color to use when latching.
  GetValueOrDefault(cJSON_GetObjectItem(doc, "latchcolor"), &valBuffer, "#fe0149");
  generalconfig.latchedColour = convertRGB888ToRGB565(convertHTMLtoRGB888(valBuffer));
  // Get the color for the background.
  GetValueOrDefault(cJSON_GetObjectItem(doc, "background"), &valBuffer, "#000000");
  generalconfig.backgroundColour = convertRGB888ToRGB565(convertHTMLtoRGB888(valBuffer));

  // Loading general settings

  GetValueOrDefault(cJSON_GetObjectItem(doc, "sleepenable"),&generalconfig.sleepenable,false);
  GetValueOrDefault(cJSON_GetObjectItem(doc, "sleeptimer"),&generalconfig.sleeptimer,60);
  GetValueOrDefault(cJSON_GetObjectItem(doc, "beep"),&generalconfig.beep,false);
  GetValueOrDefault(cJSON_GetObjectItem(doc, "modifier1"),&generalconfig.modifier1,0);
  GetValueOrDefault(cJSON_GetObjectItem(doc, "modifier2"),&generalconfig.modifier2,0);
  GetValueOrDefault(cJSON_GetObjectItem(doc, "modifier3"),&generalconfig.modifier3,0);
  GetValueOrDefault(cJSON_GetObjectItem(doc, "helperdelay"),&generalconfig.helperdelay,0);
  return true;
}
