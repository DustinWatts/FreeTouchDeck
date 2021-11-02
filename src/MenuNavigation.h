#pragma once
#include "UserConfig.h"
#include "Menu.h"
#include "globals.hpp"
namespace FreeTouchDeck {
    void LoadAllMenus();
    void LoadSystemMenus();
    bool SetActiveScreen(const char * name);
    bool ScreenLock(TickType_t xTicksToWait) ;
    void ScreenUnlock() ;
    char *MenusToJson(bool withSystem = false);
    cJSON *MenusToJsonObject(bool withSystem = false);
    Menu *GetActiveScreen();
    Menu *GetScreen(const char *name, bool lock=true);
    bool SaveFullFormat();
    Menu *GetLatchScreen(FTAction *action);
    bool LoadFullFormat(const char * fileName);
    bool LoadFullFormat();
    void handleDisplay(bool pressed, uint16_t t_x, uint16_t t_y);
    extern FTAction *sleepSetLatchAction;
    extern FTAction *sleepClearLatchAction;
    extern FTAction *sleepToggleLatchAction;
    extern FTAction *beepSetLatchAction;
    extern FTAction *beepClearLatchAction;
}

