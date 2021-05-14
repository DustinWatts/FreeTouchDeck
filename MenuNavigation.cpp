#include "stdint.h"
#include "MenuNavigation.h"
#include "Menu.h"
#include "FTAction.h"
#include <vector>
#include <TFT_eSPI.h>

using namespace std;
std::vector<FreeTouchDeck::Menu *> Menus;
SemaphoreHandle_t xScreenSemaphore = xSemaphoreCreateMutex();
bool ScreenLock(TickType_t xTicksToWait)
{
    //    ESP_LOGV(TAG, "Locking config json object");
    if (xSemaphoreTake(xScreenSemaphore, xTicksToWait) == pdTRUE)
    {
        //      ESP_LOGV(TAG, "config Json object locked!");
        return true;
    }
    else
    {
        Serial.println("[ERROR]: Unable to lock the Screen object");
        return false;
    }
}

void ScreenUnlock()
{
    xSemaphoreGive(xScreenSemaphore);
}
FreeTouchDeck::Menu *GetActiveScreen()
{
    FreeTouchDeck::Menu *ActiveScreen = NULL;
    if (ScreenLock(portMAX_DELAY / portTICK_PERIOD_MS))
    {
        for (auto m : Menus)
        {
            if (m->Active)
            {

                ActiveScreen = m;
            }
        }
        ScreenUnlock();
    }
    return ActiveScreen;
}
FreeTouchDeck::Menu *GetScreen(const char *name)
{
    FreeTouchDeck::Menu *Match = NULL;
    if (ScreenLock(portMAX_DELAY / portTICK_PERIOD_MS))
    {
        for (auto m : Menus)
        {
            if (!strcmp(m->Name, name))
            {
                Match = m;
            }
        }
        if (!Match)
        {
            Serial.printf("[ERROR]: Screen %s not found", name);
        }
        ScreenUnlock();
    }
    return Match;
}
void SetActiveScreen(const char *name)
{
    FreeTouchDeck::Menu *Active = GetActiveScreen();
    FreeTouchDeck::Menu *Match = GetScreen(name);
    if (Match)
    {
        if (ScreenLock(portMAX_DELAY / portTICK_PERIOD_MS))
        {
            if (Active)
            {
                Serial.println(Active->Name);
                Active->Deactivate();
            }
            Match->Activate();
            ScreenUnlock();
        }
        else
        {
            Serial.printf("[ERROR]: Menu %s was not found\n", name);
        }
    }
}
void LoadSystemMenus(TFT_eSPI &tft)
{
    const char *configMenu = R"(
    {
      "logo0": "wifi.bmp",
      "logo1": "brightnessdown.bmp",
      "logo2": "brightnessup.bmp",
      "logo3": "sleep.bmp",
      "logo4": "info.bmp",
      "button0":{
        "latch": false,
        "latchlogo": "",
        "actionarray": ["12", "11" ],
        "valuearray": ["config", "1"] 
      },
      "button1":{
        "latch": false,
        "latchlogo": "",
        "actionarray": [ "11"],
        "valuearray": [ "2"] 
      },
      "button2":{
        "latch": false,
        "latchlogo": "",
        "actionarray": [ "11"],
        "valuearray": [ "3"] 
      },
      "button3":{
        "latch": true,
        "latchlogo": "",
        "actionarray": [ "11"],
        "valuearray": [ "4"] 
      },
      "button4":{
        "latch": false,
        "latchlogo": "",
        "actionarray": [ "12","11"],
        "valuearray": [ "info","5"] 
      }   
    }    
  )";
    Serial.printf("Adding settings menu\n");
    Menus.push_back(new FreeTouchDeck::Menu("menu6", configMenu, tft));

    const char *configInfo = R"(
    {}
  )";
    Serial.printf("Adding info  menu\n");
    Menus.push_back(new FreeTouchDeck::Menu("info", configInfo, tft));
    const char *configScreen = R"(
    {}
  )";
    Serial.printf("Adding config menu\n");
    Menus.push_back(new FreeTouchDeck::Menu("config", configInfo, tft));
}
void LoadAllMenus(TFT_eSPI &tft)
{
    if (ScreenLock(portMAX_DELAY / portTICK_PERIOD_MS))
    {
        File root = SPIFFS.open("/");
        File file = root.openNextFile();
        while (file)
        {
            String FileName = file.name();
            if (FileName.startsWith("/config/menu") || FileName.startsWith("/config/homescreen"))
            {
                Menus.push_back(new FreeTouchDeck::Menu(file, tft));
                Serial.printf("Added menu from file %s\n",file.name());
            }
            file = root.openNextFile();
        }
        ScreenUnlock();
    }
    else
    {
        Serial.printf("[ERROR]: Unable to add menus   \n");
    }
}
