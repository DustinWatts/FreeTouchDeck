#include "stdint.h"
#include "MenuNavigation.h"
#include "Menu.h"
#include "FTAction.h"
#include <vector>
#include <TFT_eSPI.h>
#include <FS.h>

static const char *configMenu = R"(
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
const char *configInfo = R"(
    {}
  )";
const char *configScreen = R"(
    {}
  )";
static const char *module = "MenuNavigation";
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
        ESP_LOGE(module,"Unable to lock the Screen object");
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
            ESP_LOGE(module,"Screen %s not found", name);
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
            ESP_LOGE(module,"Menu %s was not found", name);
        }
    }
}
void LoadSystemMenus()
{
    if (ScreenLock(portMAX_DELAY / portTICK_PERIOD_MS))
    {
        ESP_LOGD(module,"Adding settings menu");
        Menus.push_back(new FreeTouchDeck::Menu("menu6", configMenu));
        ESP_LOGD(module,"Adding info  menu");
        Menus.push_back(new FreeTouchDeck::Menu("info", configInfo));
        ESP_LOGD(module,"Adding config menu");
        Menus.push_back(new FreeTouchDeck::Menu("config", configScreen));
        ScreenUnlock();
    }
    else
    {
        Serial.printf("[ERROR]: Unable to add menus   \n");
    }
}

void LoadAllMenus()
{
    if (ScreenLock(portMAX_DELAY / portTICK_PERIOD_MS))
    {
        File root = SPIFFS.open("/");
        File file = root.openNextFile();
        while (file)
        {
            String FileName = file.name();
            if (FileName.startsWith("/config/menu"))
            {
                ESP_LOGD(module, "Adding menu from file %s", file.name());
                Menus.push_back(new FreeTouchDeck::Menu(&file));
                ESP_LOGD(module, "Adding menu completed. Getting next file");
            }
            file = root.openNextFile();
        }
        ESP_LOGD(module, "Adding home screen menu from file name /config/homescreen.json");
        Menus.push_back(new FreeTouchDeck::Menu("/config/homescreen.json"));
        root.close();
        ScreenUnlock();
    }
    else
    {
        Serial.printf("[ERROR]: Unable to add menus   \n");
    }
}
