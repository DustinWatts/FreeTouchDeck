#pragma once
#include "UserConfig.h"
#include "Menu.h"
#include "globals.hpp"
namespace FreeTouchDeck {
    extern void LoadAllMenus();
    extern void LoadSystemMenus();
    extern bool SetActiveScreen(const char * name);
    extern bool RunActiveScreenAction(FTAction * action);
    extern bool ScreenLock(TickType_t xTicksToWait) ;
    extern void ScreenUnlock() ;
    extern void InitAllMenus();
    extern Menu *GetActiveScreen();
    extern bool RunLatchAction(FTAction *action);
    extern Menu *GetScreen(const char *name);
}

