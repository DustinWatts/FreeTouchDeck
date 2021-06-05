#pragma once
#include "UserConfig.h"
#include "Menu.h"
#include "globals.hpp"
namespace FreeTouchDeck {
    extern void LoadAllMenus();
    extern void LoadSystemMenus();
    extern bool SetActiveScreen(const char * name);
    extern bool ScreenLock(TickType_t xTicksToWait) ;
    extern void ScreenUnlock() ;
    extern char *MenusToJson(bool withSystem = false);
    extern Menu *GetActiveScreen();
    extern Menu *GetScreen(const char *name, bool lock=true);
    extern bool SaveFullFormat();
    extern Menu *GetLatchScreen(FTAction *action);
    extern bool LoadFullFormat(const char * fileName);
    extern bool LoadFullFormat();
}

