#pragma once
#include <Arduino.h>
#include <map>
#include <list>
#include <vector>
#include <ArduinoJson.h> // Using ArduinoJson to read and write config files
#include "cJSON.h" // using cJSON for menu processing 
#include <FS.h>       // Filesystem support header
#include <SPIFFS.h>   // Filesystem support header
#include "FTButton.h"
#include "UserConfig.h"
#include "globals.hpp"
#define MAX_NUM_BUTTONS 20
namespace FreeTouchDeck {
typedef struct 
{
  uint16_t TotalWidth=0;
  uint16_t ButtonCenterWidth=0;
  uint16_t ButtonCenterHeight=0;
  uint16_t Height=0;
  uint16_t Count=0;
  uint16_t Spacing=0;
} row_t;
class Menu
{
  public:
    char * Name=NULL;
    char * FileName=NULL;
    bool Active = false;
    bool Loaded = true;
    FTButton * buttons[MAX_NUM_BUTTONS]={0};
    uint8_t ButtonsCount=0;
    Menu(const char * name);
    Menu(File *config);
    Menu(const char *name, const char * config);
    void Draw(bool force=false);
    ~Menu();
    void Touch(uint16_t x, uint16_t y);
    void SetMargin(uint16_t value);
    void ReleaseAll();
    void Activate();
    void Deactivate();
             void* operator new(size_t sz) {
               ESP_LOGD("Menu","class operator new : %d",sz);    
                 return malloc_fn(sz);
            }   

  private:
    uint16_t _margin = 8; // 8 pixels
    uint16_t _outline = TFT_WHITE;
    uint8_t _textSize = KEY_TEXTSIZE;
    uint16_t _textColor = TFT_WHITE;
    void SetFileName();
    bool LoadConfig(File *config);
    bool LoadConfig(const char * config);
    std::vector<row_t *> _rows;
    std::list<FTAction *> actions;
    void Init();
    void AddHomeButton(uint8_t * position);
    FTAction * homeMenu;


};
}