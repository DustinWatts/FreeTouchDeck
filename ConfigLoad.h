#pragma once
#include "globals.hpp"
#include "UserConfig.h"
#include "FTAction.h"
#include <type_traits>
namespace FreeTouchDeck
{

    enum class LogLevels
    {
        NONE = 0,
        ERROR,
        WARN,
        INFO,
        DEBUG,
        VERBOSE
    };
    struct Config
    {
        uint32_t menuButtonColour;
        uint32_t functionButtonColour;
        uint32_t backgroundColour;
        uint32_t latchedColour;
        uint32_t DefaultOutline;
        uint32_t DefaultTextColor;
        uint8_t DefaultTextSize;
        uint8_t colscount;
        uint8_t rowscount;
        bool sleepenable;
        uint16_t keyDelay;
        uint16_t sleeptimer;
        bool beep;
        bool flip_touch_axis;
        bool reverse_x_touch;
        bool reverse_y_touch;
        uint8_t screenrotation;
        char *deviceName;
        char *manufacturer;
        uint16_t helperdelay;
        uint8_t ledBrightness;
        LogLevels LogLevel;
    };
    extern Config generalconfig;
    bool GetValueOrDefault(cJSON *value, char **valuePointer, const char *defaultValue);
    bool GetValueOrDefault(cJSON *value, std::string &valuePointer, const char *defaultValue);
    bool GetValueOrDefault(cJSON *value, uint16_t *valuePointer, uint16_t defaultValue);
    bool GetValueOrDefault(cJSON *value, uint8_t *valuePointer, uint8_t defaultValue);
    bool GetValueOrDefault(cJSON *value, bool *valuePointer, bool defaultValue);
    bool GetValueOrDefault(cJSON *doc, const char *name, char **valuePointer, const char *defaultValue);
    bool GetValueOrDefault(cJSON *doc, const char *name, std::string &valuePointer, const char *defaultValue);
    bool GetValueOrDefault(cJSON *doc, const char *name, uint16_t *valuePointer, uint16_t defaultValue);
    bool GetValueOrDefault(cJSON *doc, const char *name, uint8_t *valuePointer, uint8_t defaultValue);
    void GetValueOrDefault(cJSON *doc, const char *name, bool *valuePointer, bool defaultValue);
    bool GetColorOrDefault(cJSON *doc, const char *name, uint16_t *valuePointer, uint16_t defaultValue);
    bool GetColorOrDefault(cJSON *doc, const char *name, uint32_t *valuePointer, uint32_t defaultValue);
    char *GetModifierFromNumber(int modifier);
    void SetGeneralConfigDefaults();
    void QueueSaving();
    /**
* @brief This function loads the menu configuration.
*
* @param[in] const char * the config to be loaded
* @param[in] bool save if changes were found
*
* @return none
*
* @note Options for values are: colors, homescreen, menu1, menu2, menu3
         menu4, and menu5
*/
    bool loadConfig(const char *name, bool saveifchanged=false);
    bool loadGeneralConfig();
    bool saveConfig(bool serial);
    cJSON *GetConfigJson();
    extern const char *generalConfigFile;
    template <typename T, typename std::enable_if<std::is_arithmetic<T>::value>::type* = nullptr>
    void HasConfigElementChanged(const char *name, T currentVal, T newVal, bool &result, const char * module)
  {
    if (currentVal != newVal)
    {
      LOC_LOGI(module, "Configuration element %s change from %d to %d", name, currentVal, newVal);
      result = true;
    }
  }
}