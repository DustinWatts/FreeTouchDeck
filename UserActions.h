
#include "globals.hpp"
#include "MenuNavigation.h"
#include <BleKeyboard.h>

extern BleKeyboard bleKeyboard;
bool SetSleep(FTAction *action)
{
    if (generalconfig.sleepenable)
    {
        generalconfig.sleepenable = false;
        LOC_LOGI(module, "Sleep disabled.");
    }
    else
    {
        generalconfig.sleepenable = true;
        Interval = generalconfig.sleeptimer * 60000;
        LOC_LOGI(module, "Sleep Enabled. Timer set to %d", generalconfig.sleeptimer);
    }
    // save the configuration and 
    saveConfig(false);
    HandleSleepConfig();
}
bool ChangeBrightness(FTAction *action)
{
    if (strcmp(action->ActionName, "BRIGHTNESS_UP") == 0)
    {
        generalconfig.ledBrightness = min(generalconfig.ledBrightness + LED_BRIGHTNESS_INCREMENT, 255);
    }
    else if (strcmp(action->ActionName, "BRIGHTNESS_DOWN") == 0)
    {
        generalconfig.ledBrightness = max(generalconfig.ledBrightness - LED_BRIGHTNESS_INCREMENT, 50);
    }
    else
    {
        LOC_LOGE(module, "Unknown Brightness action %s", action->ActionName);
    }
    ledcWrite(0, generalconfig.ledBrightness);
    return true;
}

/**
* @brief This function prints some information about the current version 
         and setup of FreetouchDeck to the TFT screen.
*
* @param none
*
* @return none
*
* @note none
*/

bool printinfo(FTAction *dummy)
{
    tft.setCursor(1, 3);
    tft.setTextFont(2);
    if (tft.width() < 480)
    {
        tft.setTextSize(1);
    }
    else
    {
        tft.setTextSize(2);
    }
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.printf("Version: %s\n", versionnumber);

#ifdef touchInterruptPin
    if (generalconfig.sleepenable)
    {
        tft.println("Sleep: Enabled");
        tft.printf("Sleep timer: %u minutes\n", generalconfig.sleeptimer);
    }
    else
    {
        tft.println("Sleep: Disabled");
    }
#else
    tft.println("Sleep: Disabled");
#endif

#ifdef speakerPin
    if (generalconfig.beep)
    {
        tft.println("Speaker: Enabled");
    }
    else
    {
        tft.println("Speaker: Disabled");
    }
#else
    tft.println("Speaker: Disabled");
#endif

    tft.print("Free Storage: ");
    float freemem = SPIFFS.totalBytes() - SPIFFS.usedBytes();
    tft.print(freemem / 1000);
    tft.println(" kB");
    tft.print("BLE Keyboard version: ");
    tft.println(BLE_KEYBOARD_VERSION);
    tft.print("ArduinoJson version: ");
    tft.println(ARDUINOJSON_VERSION);
    tft.print("TFT_eSPI version: ");
    tft.println(TFT_ESPI_VERSION);
    tft.println("ESP-IDF: ");
    tft.println(esp_get_idf_version());
    return true;
}
bool RunLatchAction(FTAction *action)
{
    bool success = false;
    Menu *menu = GetLatchScreen(action);
    if (menu)
    {
        LOC_LOGD(module, "Running Latch Action on Menu %s", menu->Name);
        success = menu->Button(action);
        if (!success)
        {
            LOC_LOGE(module, "Running Latch action failed");
        }
    }
    else
    {
        LOC_LOGE(module, "Screen not found for latch action.");
    }
    return success;
}
bool EnterSleep()
{
    // esp_deep_sleep does not shut down WiFi, BT, and higher level protocol connections gracefully. Make sure relevant WiFi and BT stack functions are called to close any connections and deinitialize the peripherals. These include:
    tft.fillScreen(TFT_BLACK);
    LOC_LOGD(module, "Going to sleep.");
    HandleAudio(Sounds::GOING_TO_SLEEP);
    //todo better power management for TWATCH
    //       power->setPowerOutPut(AXP202_LDO2, AXP202_OFF);
    esp_err_t err = esp_sleep_enable_ext0_wakeup(touchInterruptPin, 0);
    if (err != ESP_OK)
    {
        LOC_LOGE(module, "Unable to set external wakeup pin: %s", esp_err_to_name(err));
        generalconfig.sleepenable = false;
        // 
        HandleSleepConfig();
        // disable sleep for next time
        saveConfig(false);
        return false;
    }
    else
    {

        esp_deep_sleep_start();
    }
    return true;
}
ActionCallbackMap_t FreeTouchDeck::UserActions = {
    {"BRIGHTNESS_UP", ChangeBrightness},
    {"BRIGHTNESS_DOWN", ChangeBrightness},
    {"STARTSLEEP", [](FTAction *action)
     {
         LOC_LOGD(module, "Local action was called to enter sleep!");
         return EnterSleep();
     }},
    {"ENTER_CONFIG", [](FTAction *action)
     {
         LOC_LOGW(module, "Restarting in configuration mode");
         restartReason = SystemMode::CONFIG;
         ESP.restart();
         return true;
     }},
    {"MENU", [](FTAction *action)
     {
         return SetActiveScreen(action->symbol);
     }},
    {"SLEEP", SetSleep},
    {"BEEP", [](FTAction *action)
     {
         generalconfig.beep = !generalconfig.beep;
         return saveConfig(false);
     }},
    {"INFO", printinfo},
    {"REBOOT", [](FTAction *action)
     {
         ESP.restart();
         return false;
     }},
    {"DELAY", [](FTAction *action)
     {
         delay(atol(action->symbol));
         return true;
     }},     
    {"RELEASEALL", [](FTAction *action)
     {
         bleKeyboard.releaseAll();
         return true;
     }},
    {"TOGGLELATCH", RunLatchAction}};