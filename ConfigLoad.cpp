#include "ConfigLoad.h"
#include "FTButton.h"
#include "System.h"
#include "Storage.h"
#include "ConfigHelper.h"
#include "FTAction.h"
namespace FreeTouchDeck
{
  FTAction *saveConfigAction = new FTAction(ParametersList_t({"SAVECONFIG"}));
  using namespace fs;
  const char *defaultDeviceName = "FreeTouchDeck";
  const char *defaultManufacturerName = "Made by me";
  Config generalconfig;
  static const char *module = "ConfigLoad";
  const char *generalConfigFile = "/config/general.json";

  void HasConfigElementChanged(const char *name, const char *currentVal, const char *newVal, bool &result, const char * module)
  {
    if (strcmp(currentVal, newVal) != 0)
    {
      LOC_LOGI(module, "Configuration element %s change from %s to %s", name, currentVal, newVal);
      result = true;
    }
  }
   void HasConfigElementChanged(const char *name, bool currentVal, bool newVal, bool &result, const char * module)
  {
    if (currentVal != newVal)
    {
      LOC_LOGI(module, "Configuration element %s change from %s to %s", name, currentVal ? "TRUE" : "FALSE", newVal ? "TRUE" : "FALSE");
      result = true;
    }
  }
   void HasConfigElementChanged(const char *name, LogLevels currentVal, LogLevels  newVal, bool &result, const char * module )
  {
    if (currentVal != newVal)
    {
      LOC_LOGI(module, "Configuration element %s change from %d to %d", name, currentVal, newVal);
      result = true;
    }
  }  
#define HAS_CONFIG_ELEMENT_CHANGED(e) HasConfigElementChanged(QUOTE(e), currentConfig.e, newConfig.e, result, module)
  bool WasConfigChanged(Config &currentConfig, Config &newConfig)
  {
    bool result = false;
    HAS_CONFIG_ELEMENT_CHANGED(menuButtonColour);
    HAS_CONFIG_ELEMENT_CHANGED(functionButtonColour);
    HAS_CONFIG_ELEMENT_CHANGED(backgroundColour);
    HAS_CONFIG_ELEMENT_CHANGED(latchedColour);
    HAS_CONFIG_ELEMENT_CHANGED(DefaultOutline);
    HAS_CONFIG_ELEMENT_CHANGED(DefaultTextColor);
    HAS_CONFIG_ELEMENT_CHANGED(DefaultTextSize);
    HAS_CONFIG_ELEMENT_CHANGED(colscount);
    HAS_CONFIG_ELEMENT_CHANGED(rowscount);
    HAS_CONFIG_ELEMENT_CHANGED(sleepenable);
    HAS_CONFIG_ELEMENT_CHANGED(keyDelay);
    HAS_CONFIG_ELEMENT_CHANGED(sleeptimer);
    HAS_CONFIG_ELEMENT_CHANGED(beep);
    HAS_CONFIG_ELEMENT_CHANGED(flip_touch_axis);
    HAS_CONFIG_ELEMENT_CHANGED(reverse_x_touch);
    HAS_CONFIG_ELEMENT_CHANGED(reverse_y_touch);
    HAS_CONFIG_ELEMENT_CHANGED(screenrotation);
    HAS_CONFIG_ELEMENT_CHANGED(deviceName);
    HAS_CONFIG_ELEMENT_CHANGED(manufacturer);
    HAS_CONFIG_ELEMENT_CHANGED(helperdelay);
    HAS_CONFIG_ELEMENT_CHANGED(ledBrightness);
    HAS_CONFIG_ELEMENT_CHANGED(LogLevel);

    return result;
  }
  void SetGeneralConfigDefaults()
  {
    generalconfig.menuButtonColour = 0x009bf4;
    generalconfig.functionButtonColour = 0x00efcb;
    generalconfig.latchedColour = 0xfe0149;
    generalconfig.backgroundColour = TFT_BLACK;
    generalconfig.sleepenable = false;
    generalconfig.sleeptimer = 60;
    generalconfig.beep = false;
#ifdef DEFAULT_LOG_LEVEL
    generalconfig.LogLevel = DEFAULT_LOG_LEVEL;
#else
    generalconfig.LogLevel = LogLevels::INFO;
#endif

    generalconfig.helperdelay = 0;
    generalconfig.screenrotation = SCREEN_ROTATION;
    generalconfig.flip_touch_axis = FLIP_TOUCH_AXIS;
    generalconfig.reverse_x_touch = INVERSE_X_TOUCH;
    generalconfig.reverse_y_touch = INVERSE_Y_TOUCH;
    generalconfig.rowscount = 3;
    generalconfig.colscount = 3;
    generalconfig.DefaultOutline = 0xffffff;
    generalconfig.DefaultTextColor = 0xffffff;
    generalconfig.keyDelay = 20;
    generalconfig.DefaultTextSize = KEY_TEXTSIZE;
    generalconfig.ledBrightness = 255;
    FREE_AND_NULL(generalconfig.deviceName);
    generalconfig.deviceName = ps_strdup(defaultDeviceName);
    FREE_AND_NULL(generalconfig.manufacturer);
    generalconfig.manufacturer = ps_strdup(defaultManufacturerName);
    generalconfig.memStatsDelay = 5000;
  }

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
      if (defaultValue)
      {
        LOC_LOGV(module, "Copying default value [%s]", defaultValue);
        (*valuePointer) = ps_strdup(defaultValue);
      }
      else
      {
        (*valuePointer) = NULL;
      }
    }
    return success;
  }
  bool GetValueOrDefault(cJSON *value, std::string &valuePointer, const char *defaultValue)
  {
    bool success = false;
    char *tempValue = NULL;
    LOC_LOGV(module, "Clearing string value");
    valuePointer.clear();
    if (value && cJSON_IsString(value))
    {
      tempValue = cJSON_GetStringValue(value);
      if (tempValue && strlen(tempValue) > 0)
      {
        LOC_LOGV(module, "Assigning value %s", tempValue);
        valuePointer = tempValue;
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
    if (valuePointer.empty())
    {
      if (defaultValue)
      {
        LOC_LOGV(module, "Copying default value [%s]", defaultValue);
        valuePointer = defaultValue;
      }
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
  bool GetValueOrDefault(cJSON *doc, const char *name, std::string &valuePointer, const char *defaultValue)
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
  bool GetColorOrDefault(cJSON *doc, const char *name, uint16_t *valuePointer, uint16_t defaultValue)
  {
    LOC_LOGV(module, "Looking for uint16_t value %s", name);
    char *color = NULL;
    cJSON *colorJson = cJSON_GetObjectItem(doc, name);
    if (!colorJson || !cJSON_IsString(colorJson) || ISNULLSTRING(cJSON_GetStringValue(colorJson)))
    {
      *valuePointer = defaultValue;
      return false;
    }
    else
    {
      *valuePointer = convertHTMLtoRGB888(cJSON_GetStringValue(colorJson));
      return true;
    }
  }
  bool GetColorOrDefault(cJSON *doc, const char *name, uint32_t *valuePointer, uint32_t defaultValue)
  {
    LOC_LOGV(module, "Looking for uint16_t value %s", name);
    char *color = NULL;
    cJSON *colorJson = cJSON_GetObjectItem(doc, name);
    if (!colorJson || !cJSON_IsString(colorJson) || ISNULLSTRING(cJSON_GetStringValue(colorJson)))
    {
      *valuePointer = defaultValue;
      return false;
    }
    else
    {
      *valuePointer = convertHTMLtoRGB888(cJSON_GetStringValue(colorJson));
      return true;
    }
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
  bool loadConfig(const char *name, bool saveifchanged)
  {
    Config currentConfig;
    memcpy(&currentConfig, &generalconfig, sizeof(currentConfig));

    if (ISNULLSTRING(name))
    {
      LOC_LOGE(module, "Invalid configuration file passed. ");
      return false;
    }
    LOC_LOGI(module, "Loading configuration from %s - %s", STRING_OR_DEFAULT(ftdfs->Name, "?"), STRING_OR_DEFAULT(name, ""));
    File configfile = ftdfs->open(name, "r");

    if (!configfile && configfile.size() == 0)
    {
      LOC_LOGE(module, "Could not find file %s", name);
      return false;
    }

    size_t bufferSize = configfile.size() + 1;
    char *buffer = (char *)malloc_fn(bufferSize);
    if (!buffer)
    {
      LOC_LOGE(module, "Could not allocate %d bytes for reading file %s", bufferSize, name);
      return false;
    }
    size_t bytesRead = configfile.readBytes(buffer, bufferSize);
    if (bytesRead != configfile.size())
    {
      LOC_LOGE(module, "Could not fully read %s config in buffer. Read %d of %d bytes.", name, bytesRead, configfile.size());
      configfile.close();
      ShowDir(ftdfs);
      drawErrorMessage(false, module, "Could not fully read %s config in buffer. Read %d of %d bytes.", name, bytesRead, configfile.size());
      return false;
    }
    configfile.close();
    LOC_LOGD(module, "Parsing configuration file:\n%s", buffer);
    cJSON *doc = cJSON_Parse(buffer);
    if (!doc)
    {
      const char *error = cJSON_GetErrorPtr();
      drawErrorMessage(!ftdfs->External, module, "Unable to parse json string while loading file %s. Error: %s\nContent:\n%s", name, error, buffer);
      FREE_AND_NULL(buffer);
      return false;
    }
    else
    {
      if (generalconfig.LogLevel >= LogLevels::VERBOSE)
      {
        char *docstr = cJSON_Print(doc);
        if (docstr)
        {
          LOC_LOGD(module, "Configuration is : \n%s", docstr);
          FREE_AND_NULL(docstr);
        }
      }
    }
    FREE_AND_NULL(buffer);
    GetValueOrDefault(cJSON_GetObjectItem(doc, "manufacturer"), &generalconfig.manufacturer, defaultManufacturerName);
    LOC_LOGD(module, "Manufacturer name : %s", generalconfig.manufacturer);
    GetValueOrDefault(cJSON_GetObjectItem(doc, "devicename"), &generalconfig.deviceName, defaultDeviceName);
    LOC_LOGD(module, "Device name : %s", generalconfig.deviceName);
    // Parsing colors
    // Get the color for the menu and back home buttons.

    GetColorOrDefault(doc, "menubuttoncolor", &generalconfig.menuButtonColour, 0x9bf4);
    LOC_LOGD(module, "menuButtonColour : %06x", generalconfig.menuButtonColour);
    // Get the color for the function buttons.
    GetColorOrDefault(doc, "functionbuttoncolor", &generalconfig.functionButtonColour, 0x00efcb);
    LOC_LOGD(module, "functionbuttoncolor : %06x", generalconfig.functionButtonColour);

    GetColorOrDefault(doc, "latchcolor", &generalconfig.latchedColour, 0xfe0149);
    LOC_LOGD(module, "latchcolor : %06x", generalconfig.latchedColour);

    GetColorOrDefault(doc, "backgroundcolor", &generalconfig.backgroundColour, 0x00);
    LOC_LOGD(module, "backgroundColour : %06x", generalconfig.backgroundColour);
    GetColorOrDefault(doc, "outline", &generalconfig.DefaultOutline, TFT_WHITE);
    LOC_LOGD(module, "outline : %06x", generalconfig.DefaultOutline);
    GetColorOrDefault(doc, "textcolor", &generalconfig.DefaultTextColor, TFT_WHITE);
    LOC_LOGD(module, "textcolor : %06x", generalconfig.DefaultTextColor);

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

    GetValueOrDefault(cJSON_GetObjectItem(doc, "keydelay"), &generalconfig.keyDelay, 0);

    GetValueOrDefault(cJSON_GetObjectItem(doc, "ledbrightness"), (uint8_t *)&generalconfig.ledBrightness, 255);
    GetValueOrDefault(cJSON_GetObjectItem(doc, "textsize"), &generalconfig.DefaultTextSize, KEY_TEXTSIZE);

    GetValueOrDefault(cJSON_GetObjectItem(doc, "memstatsdelay"), &generalconfig.memStatsDelay, 5000);

    cJSON_Delete(doc);

    if (generalconfig.LogLevel >= LogLevels::VERBOSE)
    {
      LOC_LOGD(module, "Configuration after load");
      saveConfig(true);
    }
    if (WasConfigChanged(currentConfig, generalconfig))
    {
      LOC_LOGI(module, "Queuing saving of the updated configuration");
      
    }
    FTButton::BackButton->BackgroundColor = generalconfig.functionButtonColour;
    FTButton::BackButton->TextColor = generalconfig.DefaultTextColor;
    FTButton::BackButton->TextSize = generalconfig.DefaultTextSize;
    FTButton::BackButton->Outline = generalconfig.DefaultOutline;
    FTButton::HomeButton->BackgroundColor = generalconfig.functionButtonColour;
    FTButton::HomeButton->TextColor = generalconfig.DefaultTextColor;
    FTButton::HomeButton->TextSize = generalconfig.DefaultTextSize;
    FTButton::HomeButton->Outline = generalconfig.DefaultOutline;

    return true;
  }
  void QueueSaving()
  {
    QueueAction(saveConfigAction);
  }

  bool loadGeneralConfig()
  {
    if (ftdfs->External)
    {
      LOC_LOGD(module, "Booting from external storage. Checking if config exists");
      bool needsInit = false;
      if (!ftdfs->exists(generalConfigFile))
      {
        PrintScreenMessage(true, "Reading from %s, config file not found", ftdfs->Name);
        needsInit = true;
      }
      else if (GetFileSize(generalConfigFile, ftdfs) == 0)
      {
        PrintScreenMessage(false, "Configuration file is empty on device %s", ftdfs->Name);
        needsInit = true;
      }

      if (needsInit)
      {
        PrintScreenMessage(false, "Initializing %s from internal storage", ftdfs->Name);
        if (!InitializeStorage())
        {
          drawErrorMessage(true, module, "Could not initialize storage %s", ftdfs->Name);
        }
        else
        {
          PrintScreenMessage(false, "Finished initializing storage %s", ftdfs->Name);
          ftdfs->end();
          WaitTouchReboot();
        }
      }
    }
    if (!loadConfig(generalConfigFile))
    {
      drawErrorMessage(true, module, "Error loading configuration from %s", ftdfs->Name);
    }
    return true;
  }

  cJSON *GetConfigJson()
  {
    cJSON *doc = cJSON_CreateObject();
    if (!doc)
    {
      return NULL;
    }

    cJSON_AddBoolToObject(doc, "sleepenable", generalconfig.sleepenable);
    cJSON_AddBoolToObject(doc, "flip_touch_axis", generalconfig.flip_touch_axis);
    cJSON_AddBoolToObject(doc, "reverse_x_touch", generalconfig.reverse_x_touch);
    cJSON_AddBoolToObject(doc, "reverse_y_touch", generalconfig.reverse_y_touch);
    cJSON_AddBoolToObject(doc, "beep", generalconfig.beep);
    cJSON_AddStringToObject(doc, "menubuttoncolor", convertRGB888oHTMLRGB888(generalconfig.menuButtonColour));
    cJSON_AddStringToObject(doc, "functionbuttoncolor", convertRGB888oHTMLRGB888(generalconfig.functionButtonColour));
    cJSON_AddStringToObject(doc, "latchcolor", convertRGB888oHTMLRGB888(generalconfig.latchedColour));
    cJSON_AddStringToObject(doc, "backgroundcolor", convertRGB888oHTMLRGB888(generalconfig.backgroundColour));
    cJSON_AddStringToObject(doc, "outline", convertRGB888oHTMLRGB888(generalconfig.DefaultOutline));
    cJSON_AddStringToObject(doc, "textcolor", convertRGB888oHTMLRGB888(generalconfig.DefaultTextColor));

    // Loading general settings
    cJSON_AddNumberToObject(doc, "loglevel", static_cast<int>(generalconfig.LogLevel));
    cJSON_AddNumberToObject(doc, "screenrotation", generalconfig.screenrotation);
    cJSON_AddNumberToObject(doc, "sleeptimer", generalconfig.sleeptimer);

    if (!ISNULLSTRING(generalconfig.manufacturer))
      cJSON_AddStringToObject(doc, "manufacturer", generalconfig.manufacturer);
    if (!ISNULLSTRING(generalconfig.deviceName))
      cJSON_AddStringToObject(doc, "devicename", generalconfig.deviceName);
    cJSON_AddNumberToObject(doc, "helperdelay", generalconfig.helperdelay);
    cJSON_AddNumberToObject(doc, "rowscount", generalconfig.rowscount);
    cJSON_AddNumberToObject(doc, "colscount", generalconfig.colscount);

    cJSON_AddNumberToObject(doc, "keydelay", generalconfig.keyDelay);
    cJSON_AddNumberToObject(doc, "ledbrightness", generalconfig.ledBrightness);
    cJSON_AddNumberToObject(doc, "textsize", generalconfig.DefaultTextSize);
    cJSON_AddNumberToObject(doc, "memstatsdelay", generalconfig.memStatsDelay);
    return doc;
  }
  bool saveConfig(bool serial)
  {
    cJSON *doc = GetConfigJson();
    SaveJsonToFile("/config/general.json", doc);
    if (serial)
    {
      PrintMemInfo(__FUNCTION__, __LINE__);
      ShowFileContent("/config/general.json", ftdfs);
      PrintMemInfo(__FUNCTION__, __LINE__);
    }

    return true;
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
    if (!ftdfs->exists("/config/wificonfig.json"))
    {
      LOC_LOGD(module, "Config file not found!");
      return false;
    }
    File configfile = ftdfs->open("/config/wificonfig.json", FILE_READ);
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
}