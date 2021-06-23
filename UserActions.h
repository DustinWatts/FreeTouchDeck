
#include "globals.hpp"
#include "MenuNavigation.h"
#include "System.h"
#include "Audio.h"
#include "DrawHelper.h"
#include "FTAction.h"
#include "ConfigHelper.h"
namespace FreeTouchDeck
{
    bool SetSleep(FTAction *action)
    {
        if (generalconfig.sleepenable)
        {
            generalconfig.sleepenable = false;
            LOC_LOGI(module, "Sleep disabled.");
        }
        else
        {
            if (touchInterruptPin >= 0)
            {
                generalconfig.sleepenable = true;
                {
                    pinMode(touchInterruptPin, INPUT_PULLUP);
                }
                SetSleepInterval(generalconfig.sleeptimer);
                LOC_LOGI(module, "Sleep Enabled. Timer set to %d", generalconfig.sleeptimer);
            }
            else
            {
                LOC_LOGW(module, "Wakeup pin not defined. Cannot enable sleep");
                QueueAction(sleepClearLatchAction);
            }
        }
        // save the configuration
        QueueSaving();
    }
    bool ChangeBrightness(FTAction *action)
    {
        auto operation = action->GetParameter(0);
        if (strcmp(operation.c_str(), "BRIGHTNESS_UP") == 0)
        {
            generalconfig.ledBrightness = min(generalconfig.ledBrightness + LED_BRIGHTNESS_INCREMENT, 255);
        }
        else if (strcmp(operation.c_str(), "BRIGHTNESS_DOWN") == 0)
        {
            generalconfig.ledBrightness = max(generalconfig.ledBrightness - LED_BRIGHTNESS_INCREMENT, 50);
        }
        else
        {
            LOC_LOGE(module, "Unknown Brightness action %s", operation.c_str());
        }
        ledcWrite(0, generalconfig.ledBrightness);
        QueueSaving();
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
        ClearScreen();
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

        //todo: support seamless storage class
        // tft.print("Free Storage: ");

        // float freemem = ftdfs->totalBytes() - ftdfs->usedBytes();
        // tft.print(freemem / 1000);
        // tft.println(" kB");
        tft.print("BLE Keyboard version: ");
        tft.println(BLE_KEYBOARD_VERSION);
        tft.print("TFT_eSPI version: ");
        tft.println(TFT_ESPI_VERSION);
        tft.println("ESP-IDF: ");
        tft.println(esp_get_idf_version());
        tft.println();
        TFTPrintMemInfo();
        return true;
    }
    bool FontTest(FTAction *action)
    {
        const char *testText = STRING_OR_DEFAULT(action->FirstParameter(), generalconfig.deviceName);
        SetSmallestFont(0);
        int16_t curYpos = 0;
        tft.setTextSize(1);

        tft.fillScreen(generalconfig.backgroundColour);
        tft.setTextColor(generalconfig.DefaultTextColor);
        do
        {
            curYpos += tft.fontHeight();
            tft.setCursor(0, curYpos);
            tft.println(testText);

        } while (SetLargerFont());
        return true;
    }
    bool RunLatchAction(FTAction *action)
    {
        bool success = false;
        Menu *menu = GetLatchScreen(action);
        if (menu)
        {
            LOC_LOGD(module, "Running Latch Action on Menu %s", menu->Name.c_str());
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

    const ActionCallbackMap_t UserActions = {
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
             const char *screen = action->FirstParameter();
             LOC_LOGD(module, "Activating screen %s", screen);
             return SetActiveScreen(screen);
         }},
        {"SLEEP", SetSleep},
        {"BEEP", [](FTAction *action)
         {
             generalconfig.beep = !generalconfig.beep;
             LOC_LOGI(module,"Beep is now %s",generalconfig.beep?"ACTIVE":"INACTIVE");
             QueueSaving();
             return true;
         }},
        {"SAVECONFIG", [](FTAction *action)
         {
             LOC_LOGI(module,"Saving configuration");
             return saveConfig(false);
         }},
        {"INFO", printinfo},
        {"FONTTEST", FontTest},
        {"REBOOT", [](FTAction *action)
         {
             ESP.restart();
             return false;
         }},
        {"DELAY", [](FTAction *action)
         {
             delay(atol(action->FirstParameter()));
             return true;
         }},

        {"RELEASEALL", [](FTAction *action)
         {
             bleKeyboard.releaseAll();
             return true;
         }},
        {"LATCH", RunLatchAction}};

    ActionQueueType_t UserActionsKeyboardQueue = {"DELAY", "RELEASEALL"};
}
