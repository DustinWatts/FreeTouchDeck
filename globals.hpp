#pragma once
#include "stdint.h"
#include <vector>
#include "cJSON.h"
#include <list>
#include <cstdlib>
#include <array>
#include <iostream>
#include <new>
#include <map>
#include <queue>
#include <functional>
#include "DrawHelper.h"
#include "System.h"
#include "ConfigLoad.h"

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


extern const char *versionnumber;

#ifndef QUOTE
#define Q(x) #x
#define QUOTE(x) Q(x)
#endif
#define ENUM_TO_STRING_HELPER(x,y) \
  case x::y:                        \
    return QUOTE(y)
#define ENUM_TO_STRING_HELPER_SIMPLE(x) \
  case x:                        \
    return QUOTE(x); break 
#define ISNULLSTRING(x) (!x || strlen(x) == 0)
#define STRING_OR_DEFAULT(x, y) ISNULLSTRING(x) ? y : x
#define CJSON_STRING_OR_DEFAULT(x, y) (x && cJSON_IsString(x) && !ISNULLSTRING(cJSON_GetStringValue(x))) ? cJSON_GetStringValue(x) : y
#define ASSING_IF_PASSED(x, y) \
    if (x)                     \
    *x = y
#define FREE_AND_ASSIGNED_IF_PASSED(x, y) \
    if (x){ if(*x) FREE_AND_NULL(*x);  *x = y;    }                     \
    
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
#define LOC_LOGE(tag, ...)  if(generalconfig.LogLevel >= LogLevels::ERROR) log_e(__VA_ARGS__)
#define LOC_LOGW(tag, ...)  if(generalconfig.LogLevel >= LogLevels::WARN) log_w(__VA_ARGS__)
#define LOC_LOGI(tag, ...)  if(generalconfig.LogLevel >= LogLevels::INFO) log_i(__VA_ARGS__)
#define LOC_LOGD(tag, ...)  if(generalconfig.LogLevel >= LogLevels::DEBUG) log_d(__VA_ARGS__)
#define LOC_LOGV(tag, ...)  if(generalconfig.LogLevel >= LogLevels::VERBOSE) log_v(__VA_ARGS__)

