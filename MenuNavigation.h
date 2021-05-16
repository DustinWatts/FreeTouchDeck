#pragma once
#include "UserConfig.h"
#include "Menu.h"
#include "globals.hpp"
extern void LoadAllMenus();
extern void LoadSystemMenus();
extern void SetActiveScreen(const char * name);
extern bool ScreenLock(TickType_t xTicksToWait) ;
extern void ScreenUnlock() ;
extern FreeTouchDeck::Menu *GetActiveScreen();
