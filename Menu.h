#pragma once
#include <Arduino.h>
#include <map>
#include <list>
#include <vector>
#include <ArduinoJson.h> // Using ArduinoJson to read and write config files
#include "cJSON.h"       // using cJSON for menu processing
#include <FS.h>          // Filesystem support header
#include <SPIFFS.h>      // Filesystem support header
#include "FTButton.h"
#include "UserConfig.h"
#include "globals.hpp"

namespace FreeTouchDeck
{
    enum class MenuTypes
    {
        NONE,
        STANDARD,
        OLDHOME,
        SYSTEM,
        HOME,
        ROOT,
        HOMESYSTEM,
        EMPTY,
        ENDLIST
    };
  class Menu
  {
  public:
    static const char *JsonLabelName;
    static const char *JsonLabelRowsCount;
    static const char *JsonLabelColsCount;
    static const char *JsonLabelButtons;
    static const char *JsonLabelActions;
    static const char *JsonLabelType;
    static const char *JsonLabelIcon;
    char *Name = NULL;
    char *Icon=NULL;
    uint8_t ColsCount=0;
    uint8_t RowsCount = 0;
    uint8_t Spacing = 3;
    MenuTypes Type=MenuTypes::STANDARD;
    bool Active = false;
    bool Loaded = true;
    std::list<FTButton *> buttons;
    bool Pressed = false;
    Menu(const char *name);
    Menu(File *config);
    Menu(cJSON *menuJson);
    Menu(const char *name, const char *config);
    void Draw(bool force = false);
    ~Menu();
    void Touch(uint16_t x, uint16_t y);
    void ReleaseAll();
    void Activate();
    //    void Init(uint8_t rowsCount, uint8_t colsCount);
    bool Button(FTAction *action);
    FTButton *GetButton(const char *buttonName);
    FTButton *GetButtonForMenuName(const char *menuName);
    void Deactivate();
    cJSON *ToJSON();
    static Menu * FromJson(const char * jsonString);

  private:
    uint16_t _outline = TFT_WHITE;
    uint8_t _textSize = KEY_TEXTSIZE;
    uint16_t _textColor = TFT_WHITE;
    bool LoadConfig(File *config);
    bool LoadConfig(const char *config);
    std::list<FTAction *> actions;
    void SetButtonWidth();
    inline bool HasBackButton()
    {
      return Type!=MenuTypes::EMPTY && Type!=MenuTypes::ROOT &&  Type!=MenuTypes::SYSTEM;
    }
    static FTAction *homeMenu;
    uint16_t ButtonWidth = 0;
    uint16_t ButtonHeight = 0;
  };
      inline MenuTypes &operator++(MenuTypes &state, int)
    {
        int i = static_cast<int>(state) + 1;
        i = i >= (int)MenuTypes::ENDLIST ? (int)MenuTypes::NONE : i;
        state = static_cast<MenuTypes>(i);
        return state;
    }
  const char *enum_to_string(MenuTypes value);
  bool parse(const char *value, MenuTypes* result);    
}