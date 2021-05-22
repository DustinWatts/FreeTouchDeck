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

enum class SystemMode
{
  STANDARD,
  CONFIG
};

#ifndef QUOTE
#define Q(x) #x
#define QUOTE(x) Q(x)
#endif
#define ENUM_TO_STRING_HELPER(x) case x: return QUOTE(x)
#define ISNULLSTRING(x) (!x || strlen(x)==0)
#define STRING_OR_DEFAULT(x,y) ISNULLSTRING(x)?y:x
#define CJSON_STRING_OR_DEFAULT(x,y) (x && cJSON_IsString(x) && !ISNULLSTRING(cJSON_GetStringValue(x)))?cJSON_GetStringValue(x):y

extern void * malloc_fn(size_t sz);
IRAM_ATTR char* ps_strdup(const char * fmt);
extern void PrintMemInfo();
extern TFT_eSPI tft;
extern SystemMode restartReason;
#define FREE_AND_NULL(x) if(x!=NULL) {free(x); x=NULL;}
#define MEMSET_SIZEOF(x)  memset(x,0x00,sizeof(x))
#define EXECUTE_IF_EXISTS(x,y) if(x) { x(y); } else {ESP_LOGW(module,"Function %s not implemented", QUOTE(x));}

