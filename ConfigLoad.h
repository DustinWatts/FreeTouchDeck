#include "globals.hpp"
#include "UserConfig.h"
#include "FTAction.h"
const char * defaultDeviceName = "FreeTouchDeck";
const char * defaultManufacturerName = "Made by me";
bool GetValueOrDefault(cJSON *value, char **valuePointer, const char *defaultValue)
{
  bool success = false;
  char *tempValue = NULL;
  FREE_AND_NULL((*valuePointer));
  if (value && cJSON_IsString(value))
  {
    tempValue = cJSON_GetStringValue(value);
    if (tempValue && strlen(tempValue) > 0)
    {
      LOC_LOGV(module, "Assigning value %s", tempValue);
      (*valuePointer) = ps_strdup(tempValue);
      success = true;
    }
    else
    {
      LOC_LOGV(module, "Empty string value was found");
    }
  }
  else
  {
    if (!value)
    {
      LOC_LOGV(module, "Value was not found");
    }
    else if (!cJSON_IsString(value))
    {
      LOC_LOGV(module, "Value is not a string");
    }
    else
    {
      LOC_LOGV(module, "Unknown value type");
    }
  }
  if (!(*valuePointer))
  {
    LOC_LOGW(module, "Copying default value [%s]", defaultValue);
    (*valuePointer) = ps_strdup(defaultValue);
  }
  return success;
}
bool GetValueOrDefault(cJSON *value, uint16_t *valuePointer, uint16_t defaultValue)
{
  uint16_t tempValue = 0;
  bool success = false;
  if (value && cJSON_IsNumber(value))
  {
    LOC_LOGV(module, "Value %d found, and it is a number", value->valueint);
    (*valuePointer) = (uint16_t)value->valuedouble;
    success = true;
  }
  else
  {
    if (!value)
    {
      LOC_LOGV(module, "Numeric value not found");
    }
    else
    {
      LOC_LOGV(module, "Value was found, but is not a number");
    }
    (*valuePointer) = defaultValue;
  }
  return success;
}
bool GetValueOrDefault(cJSON *value, uint8_t *valuePointer, uint8_t defaultValue)
{
  uint16_t tempValue = 0;
  bool success = false;
  if (value && cJSON_IsNumber(value))
  {
    LOC_LOGV(module, "Value %d found, and it is a number", value->valueint);
    success = true;
    (*valuePointer) = (uint8_t)value->valueint;
  }
  else
  {
    if (!value)
    {
      LOC_LOGV(module, "Numeric value not found");
    }
    else
    {
      LOC_LOGV(module, "Value was found, but is not a number");
    }
    (*valuePointer) = defaultValue;
  }
  return success;
}
bool GetValueOrDefault(cJSON *value, bool *valuePointer, bool defaultValue)
{
  bool tempValue = 0;
  bool success = false;
  if (value && cJSON_IsBool(value))
  {

    LOC_LOGV(module, "Value found, and it is a boolean", value->valueint);

    success = true;
    (*valuePointer) = cJSON_IsTrue(value);
  }
  else
  {
    if (!value)
    {
      LOC_LOGV(module, "Value was not found");
    }
    else
    {
      LOC_LOGV(module, "Value was found but is not a boolean");
    }

    (*valuePointer) = defaultValue;
  }
  return success;
}

bool GetValueOrDefault(cJSON *doc, const char *name, char **valuePointer, const char *defaultValue)
{
  LOC_LOGV(module, "Looking for char value %s", name);

  if (!GetValueOrDefault(cJSON_GetObjectItem(doc, name), valuePointer, defaultValue))
  {
    DumpCJson(doc);
  }
}
bool GetValueOrDefault(cJSON *doc, const char *name, uint16_t *valuePointer, uint16_t defaultValue)
{
  LOC_LOGV(module, "Looking for uint16_t value %s", name);

  if (!GetValueOrDefault(cJSON_GetObjectItem(doc, name), valuePointer, defaultValue))
  {
    DumpCJson(doc);
  }
}
bool GetValueOrDefault(cJSON *doc, const char *name, uint8_t *valuePointer, uint8_t defaultValue)
{
  LOC_LOGV(module, "Looking for uint8_t value %s", name);

  if (!GetValueOrDefault(cJSON_GetObjectItem(doc, name), valuePointer, defaultValue))
  {
    DumpCJson(doc);
  }
}

void GetValueOrDefault(cJSON *doc, const char *name, bool *valuePointer, bool defaultValue)
{
  LOC_LOGV(module, "Looking for boolean value %s", name);

  if (!GetValueOrDefault(cJSON_GetObjectItem(doc, name), valuePointer, defaultValue))
  {
    DumpCJson(doc);
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

bool loadWifiConfig()
{
  char buffer[501] = {0};
  if (!FILESYSTEM.exists("/config/wificonfig.json"))
  {
    LOC_LOGD(module, "Config file not found!");
    return false;
  }
  File configfile = FILESYSTEM.open("/config/wificonfig.json");
  if (!configfile && configfile.size() == 0)
  {
    LOC_LOGE(module, "Could not find file /config/wificonfig.json");
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
    GetValueOrDefault(doc, "ssid", &wificonfig.ssid, "FAILED");
    GetValueOrDefault(doc, "password", &wificonfig.password, "FAILED");
    GetValueOrDefault(doc, "wifimode", &wificonfig.wifimode, "FAILED");
    GetValueOrDefault(doc, "wifihostname", &wificonfig.hostname, "freetouchdeck");
    GetValueOrDefault(doc, "attempts", &wificonfig.attempts, 10);
    GetValueOrDefault(doc, "attemptdelay", &wificonfig.attemptdelay, 500);
    cJSON_Delete(doc);
  }
  return true;
}
char * GetModifierFromNumber(int modifier)
{
    switch (modifier)
    {
    case 0:
      /* code */
      break;
    case 0x80:
      return  ps_strdup("LEFT_CTRL");
      break;
    case 0x81:
      return ps_strdup("LEFT_SHIFT");
      break;      
    case 0x82:
      return ps_strdup("LEFT_ALT");
      break;      
    case 0x83:
      return ps_strdup("LEFT_GUI");
      break;      
    default:
      LOC_LOGE(module,"Unknown numeric modifier %d",modifier);
      break;
    }
    return NULL;
}
bool HandleModifier(const char * modifier)
{
    char * foundModifier=NULL;
    KeyValue_t KeyValue;
    if(ISNULLSTRING(modifier))
    {
      return false;
    }
    if(modifier[0]>='0' && modifier[0]<='9' )
    {
      char * numericMod=GetModifierFromNumber(atol(modifier));
      if(!ISNULLSTRING(numericMod))
      {
        generalconfig.modifiers.push_back(numericMod);
        return true;
      }
    }
    if(FTAction::parse(modifier, ActionTypes::OPTIONKEYS, &KeyValue, &foundModifier))
    {
      LOC_LOGD(module,"Adding modifier %s",STRING_OR_DEFAULT(foundModifier,"") );
      generalconfig.modifiers.push_back(foundModifier);
    }
    else
    {
      LOC_LOGD(module,"Invalid modifier string %s",STRING_OR_DEFAULT(modifier,"") );
    }
}

bool HandleModifier(cJSON *doc, const char *name)
{

  bool changed=false;

  KeySequence_t Sequence;
  char numBuffer[21] = {0};

  cJSON *JsonModifier = cJSON_GetObjectItem(doc, name);
  if (!JsonModifier)
  {
    LOC_LOGD(module, "No value found for %s", name);
    return false;
  }
  if (cJSON_IsString(JsonModifier))
  {
    
    if (!ISNULLSTRING(cJSON_GetStringValue(JsonModifier)))
    {
      ESP_LOGD(module,"Parsing modifier %s",cJSON_GetStringValue(JsonModifier) );
      HandleModifier(cJSON_GetStringValue(JsonModifier));
    }
    else
    {
      LOC_LOGD(module, "Empty value found for %s", name);
    }
  }
  else if (cJSON_IsNumber(JsonModifier))
  {
    // From the old configuration style
    HandleModifier(GetModifierFromNumber(JsonModifier->valueint));
  }
  return changed;
}
void SetGeneralConfigDefaults()
{
  for(auto m : generalconfig.modifiers)
  {
    FREE_AND_NULL(m);
  }
  generalconfig.modifiers.clear();
  generalconfig.modifiers.push_back(ps_strdup("LEFT_ALT"));
  generalconfig.modifiers.push_back(ps_strdup("LEFT_SHIFT"));
  generalconfig.menuButtonColour = convertRGB888ToRGB565(0x009bf4);
  generalconfig.functionButtonColour = convertRGB888ToRGB565(0x00efcb);
  generalconfig.latchedColour = convertRGB888ToRGB565(0xfe0149);
  generalconfig.backgroundColour = convertRGB888ToRGB565(0x000000);
  generalconfig.sleepenable= false;
  generalconfig.sleeptimer=60;
  generalconfig.beep=true;
  #ifdef DEFAULT_LOG_LEVEL
  generalconfig.LogLevel = DEFAULT_LOG_LEVEL;
  #else
  generalconfig.LogLevel = LogLevels::INFO;
  #endif

  generalconfig.helperdelay= 0;
  generalconfig.screenrotation= SCREEN_ROTATION;
  generalconfig.flip_touch_axis= FLIP_TOUCH_AXIS;
  generalconfig.reverse_x_touch= INVERSE_X_TOUCH;
  generalconfig.reverse_y_touch= INVERSE_Y_TOUCH;
  generalconfig.rowscount= 3;
  generalconfig.colscount= 3;
  generalconfig.DefaultOutline= TFT_WHITE;
  generalconfig.DefaultTextColor= TFT_WHITE;
  generalconfig.keyDelay=0;
  generalconfig.DefaultTextSize= KEY_TEXTSIZE;
  generalconfig.backgroundColour= TFT_BLACK;
  generalconfig.ledBrightness = 255;
  FREE_AND_NULL(generalconfig.deviceName);
  generalconfig.deviceName = ps_strdup(defaultDeviceName);
  FREE_AND_NULL(generalconfig.manufacturer);
  generalconfig.manufacturer = ps_strdup(defaultManufacturerName);
}

void ProcessModifier(cJSON * modifier)
{
  if(!cJSON_IsString(modifier) )
      {
        LOC_LOGE(module,"Modifiers should be strings. Cannot load modifiers");
      }
      else
      {
        char * foundKey=NULL;
        if(FTAction::IsValidKey(ActionTypes::OPTIONKEYS, cJSON_GetStringValue(modifier), &foundKey))
        {
            LOC_LOGD(module,"Parsed modifier with key %s", foundKey);
            generalconfig.modifiers.push_back(foundKey);
        }
        else 
        {
          if(!ISNULLSTRING(cJSON_GetStringValue(modifier)))
          {
            LOC_LOGE(module,"Invalid modifier key %s",cJSON_GetStringValue(modifier));
          }
        } 
      }
}
void ProcessModifier(cJSON * doc, const char * name)
{
    ProcessModifier(cJSON_GetObjectItem(doc,name));
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
bool loadConfig(const char * name)
{
  bool needsSave=false;
  if(ISNULLSTRING(name))
  {
    LOC_LOGE(module, "Invalid configuration file passed. ");
    return false;
  }
  File configfile = FILESYSTEM.open(name, "r");
  LOC_LOGI(module,"Loading configuration from file name: %s",STRING_OR_DEFAULT(name,""));
  if (!configfile && configfile.size() == 0)
  {
    LOC_LOGE(module, "Could not find file %s",name);
    return false;
  }
  char * buffer = (char *)malloc_fn(configfile.size()+1);
  if(!buffer)
  {
    LOC_LOGE(module, "Could not allocate memory for reading file %s", name);
    return false;
  }
  size_t bytesRead = configfile.readBytes(buffer,configfile.size()+1 );
  if (bytesRead != configfile.size())
  {
    drawErrorMessage(false, module, "Could not fully read %s config in buffer. Read %d of %d bytes.",name, bytesRead, configfile.size());
    return false;
  }
  configfile.close();
  cJSON *doc = cJSON_Parse(buffer);
  FREE_AND_NULL(buffer);
  if (!doc)
  {
    const char *error = cJSON_GetErrorPtr();
    drawErrorMessage(false, module, "Unable to parse json string : %s", error);
    return false;
  }
  else
  {
    char * docstr=cJSON_Print(doc);
    if(docstr)
    {
      LOC_LOGD(module,"Configuration is : \n%s",docstr);
      FREE_AND_NULL(docstr);
    }
  }
  cJSON * modifiers = cJSON_GetObjectItem(doc,"modifiers");
  if(modifiers && cJSON_IsArray(modifiers))
  {
    cJSON * modifier=NULL;
    cJSON_ArrayForEach(modifier,modifiers)
    {
      ProcessModifier(modifier);
    }
  }
  else
  {
    ProcessModifier(doc, "modifier1");
    ProcessModifier(doc, "modifier2");
    ProcessModifier(doc, "modifier3");
    needsSave=generalconfig.modifiers.size()>0;
  }
    
  GetValueOrDefault(cJSON_GetObjectItem(doc, "manufacturer"), &generalconfig.manufacturer, defaultManufacturerName);
  LOC_LOGD(module,"Manufacturer name : %s",generalconfig.manufacturer);
  GetValueOrDefault(cJSON_GetObjectItem(doc, "devicename"), &generalconfig.deviceName, defaultDeviceName);
  LOC_LOGD(module,"Device name : %s",generalconfig.deviceName);
  // Parsing colors
  // Get the color for the menu and back home buttons.
  char *valBuffer = NULL;
  GetValueOrDefault(cJSON_GetObjectItem(doc, "menubuttoncolor"), &valBuffer, "#009bf4");
  
  generalconfig.menuButtonColour = convertHTMLRGB888ToRGB565(valBuffer);
  LOC_LOGD(module,"menuButtonColour : %s, numeric: %6x",valBuffer, generalconfig.menuButtonColour);
  // Get the color for the function buttons.
  GetValueOrDefault(cJSON_GetObjectItem(doc, "functionbuttoncolor"), &valBuffer, "#00efcb");
  generalconfig.functionButtonColour = convertHTMLRGB888ToRGB565(valBuffer);
  LOC_LOGD(module,"functionButtonColour : %s, numeric: %6x",valBuffer, generalconfig.functionButtonColour);

  // Get the color to use when latching.
  GetValueOrDefault(cJSON_GetObjectItem(doc, "latchcolor"), &valBuffer, "#fe0149");
  generalconfig.latchedColour = convertHTMLRGB888ToRGB565(valBuffer);
LOC_LOGD(module,"latchcolor : %s, numeric: %6x",valBuffer, generalconfig.latchedColour);
  // Get the color for the background.
  GetValueOrDefault(cJSON_GetObjectItem(doc, "background"), &valBuffer, "#000000");
  generalconfig.backgroundColour = convertHTMLRGB888ToRGB565(valBuffer);
  LOC_LOGD(module,"background : %s, numeric: %6x",valBuffer, generalconfig.backgroundColour);
  FREE_AND_NULL(valBuffer);
  // Loading general settings

  GetValueOrDefault(cJSON_GetObjectItem(doc, "sleepenable"), &generalconfig.sleepenable, false);
  GetValueOrDefault(cJSON_GetObjectItem(doc, "sleeptimer"), &generalconfig.sleeptimer, 60);
  GetValueOrDefault(cJSON_GetObjectItem(doc, "beep"), &generalconfig.beep, false);
  uint8_t logLevel;
  GetValueOrDefault(cJSON_GetObjectItem(doc, "loglevel"), &logLevel, static_cast<uint8_t>(LogLevels::INFO));
  generalconfig.LogLevel = static_cast<LogLevels>(logLevel);

  GetValueOrDefault(cJSON_GetObjectItem(doc, "helperdelay"), &generalconfig.helperdelay, 0);
  GetValueOrDefault(cJSON_GetObjectItem(doc, "screenrotation"), &generalconfig.screenrotation, SCREEN_ROTATION);
  GetValueOrDefault(cJSON_GetObjectItem(doc, "flip_touch_axis"), &generalconfig.flip_touch_axis, FLIP_TOUCH_AXIS);
  GetValueOrDefault(cJSON_GetObjectItem(doc, "reverse_x_touch"), &generalconfig.reverse_x_touch, INVERSE_X_TOUCH);
  GetValueOrDefault(cJSON_GetObjectItem(doc, "reverse_y_touch"), &generalconfig.reverse_y_touch, INVERSE_Y_TOUCH);
  GetValueOrDefault(cJSON_GetObjectItem(doc, "rowscount"), &generalconfig.rowscount, 3);
  GetValueOrDefault(cJSON_GetObjectItem(doc, "colscount"), &generalconfig.colscount, 3);
  GetValueOrDefault(cJSON_GetObjectItem(doc, "outline"), &generalconfig.DefaultOutline, TFT_WHITE);
  GetValueOrDefault(cJSON_GetObjectItem(doc, "textcolor"), &generalconfig.DefaultTextColor, TFT_WHITE);
  GetValueOrDefault(cJSON_GetObjectItem(doc, "keydelay"), &generalconfig.keyDelay, 0);
  
  GetValueOrDefault(cJSON_GetObjectItem(doc, "ledbrightness"), (uint8_t *)&generalconfig.ledBrightness, 255);
  GetValueOrDefault(cJSON_GetObjectItem(doc, "textsize"), &generalconfig.DefaultTextSize, KEY_TEXTSIZE);
  GetValueOrDefault(cJSON_GetObjectItem(doc, "backgroundcolor"), &generalconfig.backgroundColour, TFT_BLACK);
  
  cJSON_Delete(doc);
  LOC_LOGD(module,"Configuration after load");
  saveConfig(true);
  if(needsSave)
  {
    LOC_LOGI(module,"Saving updated configuration");
    saveConfig(false);
  }
  return true;
}
bool loadGeneralConfig()
{
 return  loadConfig("/config/general.json");
}
#define ADD_COLOR_TO_JSON(x, y)                                         \
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
  cJSON_AddNumberToObject(doc, "loglevel", static_cast<int>(generalconfig.LogLevel));
  cJSON_AddNumberToObject(doc, "screenrotation", generalconfig.screenrotation);
  cJSON_AddNumberToObject(doc, "sleeptimer", generalconfig.sleeptimer);
  if(generalconfig.modifiers.size()>0)
  {
    LOC_LOGV(module,"Found %d modifiers",generalconfig.modifiers.size());
    cJSON * modifiers = cJSON_CreateArray();
    for(auto m : generalconfig.modifiers)
    {
      cJSON_AddItemToArray(modifiers,cJSON_CreateString(m));
    }  
    cJSON_AddItemToObject(doc,"modifiers",modifiers);
  
  }
  
  if(!ISNULLSTRING(generalconfig.manufacturer))
    cJSON_AddStringToObject(doc, "manufacturer", generalconfig.manufacturer);
  if(!ISNULLSTRING(generalconfig.deviceName))
    cJSON_AddStringToObject(doc, "devicename", generalconfig.deviceName);    
  cJSON_AddNumberToObject(doc, "helperdelay", generalconfig.helperdelay);
  cJSON_AddNumberToObject(doc, "rowscount", generalconfig.rowscount);
  cJSON_AddNumberToObject(doc, "colscount", generalconfig.colscount);
  cJSON_AddNumberToObject(doc, "outline", generalconfig.DefaultOutline);
  cJSON_AddNumberToObject(doc, "textcolor", generalconfig.DefaultTextColor);
  cJSON_AddNumberToObject(doc, "keydelay", generalconfig.keyDelay);
  cJSON_AddNumberToObject(doc, "ledbrightness", generalconfig.ledBrightness);
  cJSON_AddNumberToObject(doc, "textsize", generalconfig.DefaultTextSize);
  cJSON_AddNumberToObject(doc, "backgroundcolor", generalconfig.backgroundColour);
  char *json = cJSON_Print(doc);
  if (json)
  {
    if (serial)
    {
      LOC_LOGI(module, "Configuration: \n%s \n", json);
    }
    else
    {
      File configfile = FILESYSTEM.open("/config/general.json", "w");
      if (!configfile)
      {
        LOC_LOGE(module, "Could not write to file /config/general.json");
      }
      else
      {
        size_t bytesWritten = configfile.write((const uint8_t *)json, strlen(json));
        if (bytesWritten != strlen(json))
        {
          drawErrorMessage(false, module, "Could not fully write general.json config file. Written %d of %d bytes.", bytesWritten, strlen(json));
        }
        configfile.close();
      }
    }
    FREE_AND_NULL(json);
  }
  else
  {
    LOC_LOGE(module, "Error generating the config structure");
  }
  cJSON_Delete(doc);

  return true;
}