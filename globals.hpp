#pragma once
#include "stdint.h"
#include <vector>
#include "cJSON.h"
#include <FS.h>       // Filesystem support header
#include <SPIFFS.h>   // Filesystem support header
#include <TFT_eSPI.h> // The TFT_eSPI library
#include <list>
#include <cstdlib>
#include <array>
#include <iostream>
#include <new>
#include "DrawHelper.h"

#ifdef ESP_IDF_VERSION_MAJOR // IDF 4+
#if ESP_IDF_VERSION_MAJOR > 3
#if CONFIG_IDF_TARGET_ESP32 // ESP32/PICO-D4
#include "esp32/rom/rtc.h"
#elif CONFIG_IDF_TARGET_ESP32S2
#include "esp32s2/rom/rtc.h"
#elif CONFIG_IDF_TARGET_ESP32C3
#include "esp32c3/rom/rtc.h"
#else
#error Target CONFIG_IDF_TARGET is not supported
#endif
#else // ESP32 Before IDF 4.0
#include "rom/rtc.h"
#endif
#else // ESP32 Before IDF 4.0
#include "rom/rtc.h"
#endif


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
};
extern const char *versionnumber;
// the following can be uncommented in case the coordinates of the
// X or Y axis touch panel are reversed
// #define INVERSE_X_TOUCH
// #define INVERSE_Y_TOUCH

//Struct to hold the general config like colours.
struct Config
{
  uint16_t menuButtonColour;
  uint16_t functionButtonColour;
  uint16_t backgroundColour;
  uint16_t latchedColour;
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
  std::list<char *> modifiers;
  char * deviceName;
  char * manufacturer;
  uint16_t helperdelay;
  uint16_t DefaultOutline;
  uint8_t DefaultTextSize;
  uint16_t DefaultTextColor;
  int ledBrightness;
  FreeTouchDeck::LogLevels LogLevel;
};
extern Config generalconfig;
enum class SystemMode
{
  STANDARD,
  CONSOLE,
  CONFIG
};

#ifndef QUOTE
#define Q(x) #x
#define QUOTE(x) Q(x)
#endif
#define ENUM_TO_STRING_HELPER(x,y) \
  case x::y:                        \
    return QUOTE(y)
#define ISNULLSTRING(x) (!x || strlen(x) == 0)
#define STRING_OR_DEFAULT(x, y) ISNULLSTRING(x) ? y : x
#define CJSON_STRING_OR_DEFAULT(x, y) (x && cJSON_IsString(x) && !ISNULLSTRING(cJSON_GetStringValue(x))) ? cJSON_GetStringValue(x) : y
#define ASSING_IF_PASSED(x, y) \
    if (x)                     \
    *x = y
#define FREE_AND_ASSIGNED_IF_PASSED(x, y) \
    if (x){ if(*x) FREE_AND_NULL(*x);  *x = y;    }                     \
    
extern void *malloc_fn(size_t sz);
IRAM_ATTR char *ps_strdup(const char *fmt);
extern void PrintMemInfo();
extern TFT_eSPI tft;
extern void displayInit();
extern SystemMode restartReason;
extern bool saveConfig(bool serial);
extern bool GetValueOrDefault(cJSON *value, char **valuePointer, const char *defaultValue);
extern bool GetValueOrDefault(cJSON *value, uint16_t *valuePointer, uint16_t defaultValue);
extern bool GetValueOrDefault(cJSON *value, uint8_t *valuePointer, uint8_t defaultValue);
extern bool GetValueOrDefault(cJSON *value, bool *valuePointer, bool defaultValue);
extern bool GetValueOrDefault(cJSON *doc, const char *name, char **valuePointer, const char *defaultValue);
extern bool GetValueOrDefault(cJSON *doc, const char *name, uint16_t *valuePointer, uint16_t defaultValue);
extern bool GetValueOrDefault(cJSON *doc, const char *name, uint8_t *valuePointer, uint8_t defaultValue);
extern void GetValueOrDefault(cJSON *doc, const char *name, bool *valuePointer, bool defaultValue);
extern void ChangeMode(SystemMode newMode);
extern void DumpCJson(cJSON *doc);
extern void HandleSleepConfig();
extern unsigned long Interval;

#define FREE_AND_NULL(x) \
  if (x != NULL)         \
  {                      \
    free(x);             \
    x = NULL;            \
  }
#define MEMSET_SIZEOF(x) memset(x, 0x00, sizeof(x))
#define EXECUTE_IF_EXISTS(x, y)                                \
  if (x)                                                       \
  {                                                            \
    x(y);                                                      \
  }                                                            \
  else                                                         \
  {                                                            \
    LOC_LOGW(module, "Function %s not implemented", QUOTE(x)); \
  }
#define LOC_LOGE(tag, ...)  if(generalconfig.LogLevel >= FreeTouchDeck::LogLevels::ERROR) log_e(__VA_ARGS__)
#define LOC_LOGW(tag, ...)  if(generalconfig.LogLevel >= FreeTouchDeck::LogLevels::WARN) log_w(__VA_ARGS__)
#define LOC_LOGI(tag, ...)  if(generalconfig.LogLevel >= FreeTouchDeck::LogLevels::INFO) log_i(__VA_ARGS__)
#define LOC_LOGD(tag, ...)  if(generalconfig.LogLevel >= FreeTouchDeck::LogLevels::DEBUG) log_d(__VA_ARGS__)
#define LOC_LOGV(tag, ...)  if(generalconfig.LogLevel >= FreeTouchDeck::LogLevels::VERBOSE) log_v(__VA_ARGS__)