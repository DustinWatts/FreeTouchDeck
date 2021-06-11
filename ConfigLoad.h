#pragma once
#include "globals.hpp"
#include "UserConfig.h"
#include "FTAction.h"
namespace FreeTouchDeck {
    
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
        int ledBrightness;
        LogLevels LogLevel;
    };
    extern Config generalconfig;

bool GetValueOrDefault(cJSON *value, char **valuePointer, const char *defaultValue);
bool GetValueOrDefault(cJSON *value, uint16_t *valuePointer, uint16_t defaultValue);
bool GetValueOrDefault(cJSON *value, uint8_t *valuePointer, uint8_t defaultValue);
bool GetValueOrDefault(cJSON *value, bool *valuePointer, bool defaultValue);
bool GetValueOrDefault(cJSON *doc, const char *name, char **valuePointer, const char *defaultValue);
bool GetValueOrDefault(cJSON *doc, const char *name, uint16_t *valuePointer, uint16_t defaultValue);
bool GetValueOrDefault(cJSON *doc, const char *name, uint8_t *valuePointer, uint8_t defaultValue);
void GetValueOrDefault(cJSON *doc, const char *name, bool *valuePointer, bool defaultValue);
bool GetColorOrDefault(cJSON *doc, const char *name, uint16_t *valuePointer, uint16_t defaultValue);
bool GetColorOrDefault(cJSON *doc, const char *name, uint32_t *valuePointer, uint32_t defaultValue);
char *GetModifierFromNumber(int modifier);
void SetGeneralConfigDefaults();
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
bool loadConfig(const char *name);
bool loadGeneralConfig();
bool saveConfig(bool serial);

}