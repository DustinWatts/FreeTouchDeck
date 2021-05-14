#pragma once
#include "UserConfig.h"
#include "Menu.h"
extern void LoadAllMenus(TFT_eSPI &tft);
extern void SetActiveScreen(const char * name);
extern bool ScreenLock(TickType_t xTicksToWait) ;
extern void ScreenUnlock() ;
extern FreeTouchDeck::Menu *GetActiveScreen();
