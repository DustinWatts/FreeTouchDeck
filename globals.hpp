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
#ifndef QUOTE
#define Q(x) #x
#define QUOTE(x) Q(x)
#endif
#define ENUM_TO_STRING_HELPER(x) case x: return QUOTE(x)

extern  void * malloc_fn(size_t sz);
IRAM_ATTR char* ps_strdup(const char * fmt);
extern void PrintMemInfo();
extern TFT_eSPI tft;
#define FREE_AND_NULL(x) if(x!=NULL) {free(x); x=NULL;}
#define MEMSET_SIZEOF(x)  memset(x,0x00,sizeof(x))
