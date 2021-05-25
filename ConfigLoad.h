#include "globals.hpp"
#include "UserConfig.h"
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
  if (!configfile && configfile.size() == 0)
  {
    ESP_LOGE(module, "Could not find file /config/wificonfig.json");
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
  if (!configfile && configfile.size() == 0)
  {
    ESP_LOGE(module, "Could not find file /config/general.json");
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
  char *valBuffer = NULL;
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
  FREE_AND_NULL(valBuffer);
  // Loading general settings

  GetValueOrDefault(cJSON_GetObjectItem(doc, "sleepenable"), &generalconfig.sleepenable, false);
  GetValueOrDefault(cJSON_GetObjectItem(doc, "sleeptimer"), &generalconfig.sleeptimer, 60);
  GetValueOrDefault(cJSON_GetObjectItem(doc, "beep"), &generalconfig.beep, false);
  GetValueOrDefault(cJSON_GetObjectItem(doc, "modifier1"), &generalconfig.modifier1, 0);
  GetValueOrDefault(cJSON_GetObjectItem(doc, "modifier2"), &generalconfig.modifier2, 0);
  GetValueOrDefault(cJSON_GetObjectItem(doc, "modifier3"), &generalconfig.modifier3, 0);
  GetValueOrDefault(cJSON_GetObjectItem(doc, "helperdelay"), &generalconfig.helperdelay, 0);
  GetValueOrDefault(cJSON_GetObjectItem(doc, "screenrotation"), &generalconfig.screen_rotation, SCREEN_ROTATION);
  GetValueOrDefault(cJSON_GetObjectItem(doc, "flip_touch_axis"), &generalconfig.flip_touch_axis, FLIP_TOUCH_AXIS);
  GetValueOrDefault(cJSON_GetObjectItem(doc, "reverse_x_touch"), &generalconfig.reverse_x_touch, INVERSE_X_TOUCH);
  GetValueOrDefault(cJSON_GetObjectItem(doc, "reverse_y_touch"), &generalconfig.reverse_y_touch, INVERSE_Y_TOUCH);



  return true;
}
#define ADD_COLOR_TO_JSON(x, y)                                        \
  snprintf(colorBuffer, sizeof(colorBuffer), "#%06x", generalconfig.x); \
  cJSON_AddStringToObject(doc, y, colorBuffer)
bool saveConfig(bool serial)
{

  char colorBuffer[21] = {0};
  cJSON *doc = cJSON_CreateObject();
  if (!doc)
  {
    return false;
  }

  cJSON_AddBoolToObject(doc, "sleepenable", generalconfig.sleepenable);
  cJSON_AddBoolToObject(doc, "flip_touch_axis", generalconfig.flip_touch_axis);
  cJSON_AddBoolToObject(doc, "reverse_x_touch", generalconfig.reverse_x_touch);
  cJSON_AddBoolToObject(doc, "reverse_y_touch", generalconfig.reverse_y_touch);
  cJSON_AddBoolToObject(doc, "beep", generalconfig.beep);

  ADD_COLOR_TO_JSON(menuButtonColour, "menubuttoncolor");
  ADD_COLOR_TO_JSON(functionButtonColour, "functionbuttoncolor");
  ADD_COLOR_TO_JSON(latchedColour, "latchcolor");
  ADD_COLOR_TO_JSON(backgroundColour, "background");

  // Loading general settings

  cJSON_AddNumberToObject(doc, "screenrotation", generalconfig.screen_rotation);
  cJSON_AddNumberToObject(doc, "sleeptimer", generalconfig.sleeptimer);
  cJSON_AddNumberToObject(doc, "modifier1", generalconfig.modifier1);
  cJSON_AddNumberToObject(doc, "modifier2", generalconfig.modifier2);
  cJSON_AddNumberToObject(doc, "modifier3", generalconfig.modifier3);
  cJSON_AddNumberToObject(doc, "helperdelay", generalconfig.helperdelay);
  char *json = cJSON_Print(doc);
  if (json)
  {
    if(serial)
    {
      ESP_LOGI(module,"Configuration: \n%s \n", json);
    }
    else 
    {
      File configfile = FILESYSTEM.open("/config/general.json", "w");
      if (!configfile)
      {
        ESP_LOGE(module, "Could not write to file /config/general.json");
      }
      else
      {
        size_t bytesWritten = configfile.write((const uint8_t *)json,strlen(json));
        if (bytesWritten != strlen(json))
        {
          drawErrorMessage(false, module, "Could not fully write general.json config file. Written %d of %d bytes.", bytesWritten, strlen(json));
        }
        configfile.close();
      }
    }
    FREE_AND_NULL(json);
  }
  else{
    ESP_LOGE(module, "Error generating the config structure");
  }
  cJSON_Delete(doc);
  
  


  return true;
}