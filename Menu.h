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
    std::vector<FTButton *> buttons;
    Menu(const char * name,TFT_eSPI &gfx);
    Menu(File &config,TFT_eSPI &gfx);
    Menu(const char *name, const char * config, TFT_eSPI &gfx);
    void Draw(bool force=false);
    ~Menu();
    void Touch(uint16_t x, uint16_t y);
    void SetMargin(uint16_t value);
    void ReleaseAll();
    void Activate();
    void Deactivate();
  private:
    uint16_t _margin = 8; // 8 pixels
    uint16_t _outline = TFT_WHITE;
    uint8_t _textSize = KEY_TEXTSIZE;
    uint16_t _textColor = TFT_WHITE;
    void SetFileName();
    bool LoadConfig(File config);
    bool LoadConfig(const char * config);
    TFT_eSPI &GFX;
    std::vector<row_t *> _rows;
    void Init(TFT_eSPI &gfx);
    void AddHomeButton(uint8_t * position);
    FTAction homeMenu=FTAction(ActionTypes::MENU,"homescreen");


};
}