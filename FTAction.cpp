#include "FTAction.h"
#include "BleKeyboard.h"
#include "UserConfig.h"
#include "MenuNavigation.h"
#include "Menu.h"
static const char *module = "FTAction";
extern BleKeyboard bleKeyboard;
extern Config generalconfig;
extern int ledBrightness;
extern unsigned long Interval;


using namespace std;
#define MEDIA_2_VECTOR(m) \
    {                     \
        m[0], m[1]        \
    }

static char printBuffer[101] = {0};

namespace FreeTouchDeck
{
    extern bool RunLatchAction(FTAction *action);
    SemaphoreHandle_t xQueueSemaphore = xSemaphoreCreateMutex();
    std::queue<FTAction *> Queue;
    std::queue<FTAction *> ScreenQueue;
    const char *enum_to_string(ActionTypes type)
    {
        switch (type)
        {
            ENUM_TO_STRING_HELPER(ActionTypes::NONE);
            ENUM_TO_STRING_HELPER(ActionTypes::DELAY);
            ENUM_TO_STRING_HELPER(ActionTypes::ARROWS_AND_TAB);
            ENUM_TO_STRING_HELPER(ActionTypes::MEDIAKEY);
            ENUM_TO_STRING_HELPER(ActionTypes::LETTERS);
            ENUM_TO_STRING_HELPER(ActionTypes::OPTIONKEYS);
            ENUM_TO_STRING_HELPER(ActionTypes::FUNCTIONKEYS);
            ENUM_TO_STRING_HELPER(ActionTypes::NUMBERS);
            ENUM_TO_STRING_HELPER(ActionTypes::SPECIAL_CHARS);
            ENUM_TO_STRING_HELPER(ActionTypes::COMBOS);
            ENUM_TO_STRING_HELPER(ActionTypes::HELPERS);
            ENUM_TO_STRING_HELPER(ActionTypes::LOCAL);
            ENUM_TO_STRING_HELPER(ActionTypes::MENU);
            ENUM_TO_STRING_HELPER(ActionTypes::SETLATCH);
            ENUM_TO_STRING_HELPER(ActionTypes::CLEARLATCH);
            ENUM_TO_STRING_HELPER(ActionTypes::TOGGLELATCH);
        default:
            return "Unknown Action Type";
        }
    }
    const char *enum_to_string(LocalActionTypes type)
    {
        switch (type)
        {
            ENUM_TO_STRING_HELPER(LocalActionTypes::NONE);
            ENUM_TO_STRING_HELPER(LocalActionTypes::ENTER_CONFIG);
            ENUM_TO_STRING_HELPER(LocalActionTypes::BRIGHTNESS_DOWN);
            ENUM_TO_STRING_HELPER(LocalActionTypes::BRIGHTNESS_UP);
            ENUM_TO_STRING_HELPER(LocalActionTypes::SLEEP);
            ENUM_TO_STRING_HELPER(LocalActionTypes::INFO);
        default:
            return "Unknown Local Action Type";
        }
    }
    static const uint8_t ArrowsAndTab[] = {KEY_UP_ARROW, KEY_DOWN_ARROW, KEY_LEFT_ARROW, KEY_RIGHT_ARROW, KEY_BACKSPACE, KEY_TAB, KEY_RETURN, KEY_PAGE_UP, KEY_PAGE_DOWN, KEY_DELETE};
    static const vector<vector<uint8_t>> Keys = {{{0, 0},
                                                  MEDIA_2_VECTOR(KEY_MEDIA_NEXT_TRACK),
                                                  MEDIA_2_VECTOR(KEY_MEDIA_PREVIOUS_TRACK),
                                                  MEDIA_2_VECTOR(KEY_MEDIA_STOP),
                                                  MEDIA_2_VECTOR(KEY_MEDIA_PLAY_PAUSE),
                                                  MEDIA_2_VECTOR(KEY_MEDIA_MUTE),
                                                  MEDIA_2_VECTOR(KEY_MEDIA_VOLUME_UP),
                                                  MEDIA_2_VECTOR(KEY_MEDIA_VOLUME_DOWN),
                                                  MEDIA_2_VECTOR(KEY_MEDIA_WWW_HOME),
                                                  MEDIA_2_VECTOR(KEY_MEDIA_LOCAL_MACHINE_BROWSER),
                                                  MEDIA_2_VECTOR(KEY_MEDIA_CALCULATOR),
                                                  MEDIA_2_VECTOR(KEY_MEDIA_WWW_BOOKMARKS),
                                                  MEDIA_2_VECTOR(KEY_MEDIA_WWW_SEARCH),
                                                  MEDIA_2_VECTOR(KEY_MEDIA_WWW_STOP),
                                                  MEDIA_2_VECTOR(KEY_MEDIA_WWW_BACK),
                                                  MEDIA_2_VECTOR(KEY_MEDIA_CONSUMER_CONTROL_CONFIGURATION),
                                                  MEDIA_2_VECTOR(KEY_MEDIA_EMAIL_READER)}};

    static const uint8_t OptionKeys[] = {0, KEY_LEFT_CTRL, KEY_LEFT_SHIFT, KEY_LEFT_ALT, KEY_LEFT_GUI, KEY_RIGHT_CTRL, KEY_RIGHT_SHIFT, KEY_RIGHT_ALT, KEY_RIGHT_GUI};
    static uint8_t FunctionKeys[] = {0, KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_F11, KEY_F12, KEY_F13, KEY_F14, KEY_F15, KEY_F16, KEY_F17, KEY_F18, KEY_F19, KEY_F20, KEY_F21, KEY_F22, KEY_F23, KEY_F24};
    static uint8_t Helpers[] = {0, KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_F11};
    static const vector<vector<uint8_t>> Combos = {{{0},
                                                    {KEY_LEFT_CTRL, KEY_LEFT_SHIFT},
                                                    {KEY_LEFT_ALT, KEY_LEFT_SHIFT},
                                                    {KEY_LEFT_GUI, KEY_LEFT_SHIFT},
                                                    {KEY_LEFT_CTRL, KEY_LEFT_GUI},
                                                    {KEY_LEFT_ALT, KEY_LEFT_GUI},
                                                    {KEY_LEFT_CTRL, KEY_LEFT_ALT},
                                                    {KEY_LEFT_CTRL, KEY_LEFT_ALT, KEY_LEFT_GUI},
                                                    {KEY_RIGHT_CTRL, KEY_RIGHT_SHIFT},
                                                    {KEY_RIGHT_ALT, KEY_RIGHT_SHIFT},
                                                    {KEY_RIGHT_GUI, KEY_RIGHT_SHIFT},
                                                    {KEY_RIGHT_CTRL, KEY_RIGHT_GUI},
                                                    {KEY_RIGHT_ALT, KEY_RIGHT_GUI},
                                                    {KEY_RIGHT_CTRL, KEY_RIGHT_ALT},
                                                    {KEY_RIGHT_CTRL, KEY_RIGHT_ALT, KEY_RIGHT_GUI}}};
    bool FTAction::IsLatch()
    {
        return Type == ActionTypes::SETLATCH ||
               Type == ActionTypes::CLEARLATCH ||
               Type == ActionTypes::TOGGLELATCH;
    }
    void FTAction::SetType(const char *jsonTypeStr)
    {
        Type = (ActionTypes)atol(jsonTypeStr);
    }

    void FTAction::SetType(int jsonType)
    {
        Type = (ActionTypes)jsonType;
    }
    bool FTAction::GetLatchButton(char *screen, size_t screenSize, char *button, size_t buttonSize)
    {
        char buffer[101] = {0};
        if (!IsLatch())
            return false;
        if (!screen || !button || screenSize == 0 || buttonSize == 0)
        {
            return false;
        }
        strncpy(buffer, symbol, sizeof(buffer));
        char *pch;
        pch = strtok(buffer, " ,-");
        if (!pch)
            return false;
        strncpy(screen, pch, screenSize);
        pch = strtok(NULL, " ,-");
        if (!pch)
            return false;
        strncpy(button, pch, buttonSize);
        return true;
    }
    void FTAction::SetValue(char *jsonValue)
    {
        if (Type == ActionTypes::LETTERS || Type == ActionTypes::SPECIAL_CHARS || Type == ActionTypes::MENU)
        {
            if (!jsonValue || strlen(jsonValue) == 0)
            {
                ESP_LOGW(module, "Empty string value for action type %s", enum_to_string(Type));
            }
            else
            {
                ESP_LOGD(module, "Assigning string value %s for action type %s", jsonValue, enum_to_string(Type));
            }
            symbol = ps_strdup(jsonValue);
        }
        else
        {
            value = atol(jsonValue);
            ESP_LOGD(module, "Converting value %s to number %d for action type %s", jsonValue, value, enum_to_string(Type));
        }
    }
    void FTAction::SetValue(int jsonValue)
    {
        char buffer[33];
        if (Type == ActionTypes::LETTERS || Type == ActionTypes::SPECIAL_CHARS)
        {
            symbol = ps_strdup(itoa(jsonValue, buffer, sizeof(buffer) - 1));
            ESP_LOGD(module, "Assigning string value %s from number %d for action type %s", symbol, jsonValue, enum_to_string(Type));
        }
        else
        {
            ESP_LOGD(module, "Assigning number %d for action type %s", value, enum_to_string(Type));
            value = jsonValue;
        }
    }
    FTAction::FTAction(char *jsonActionType, char *jsonValueStr)
    {
        SetType(jsonActionType);
        SetValue(jsonValueStr);
    }
    ActionTypes FTAction::GetType(cJSON *jsonActionType)
    {
        if (cJSON_IsString(jsonActionType))
        {
            return (ActionTypes)atol(cJSON_GetStringValue(jsonActionType));
        }
        else
        {
            return (ActionTypes)jsonActionType->valueint;
        }
    }
    FTAction::FTAction(cJSON *jsonActionType, cJSON *jsonValue)
    {
        Type = GetType(jsonActionType);
        if (cJSON_IsString(jsonValue))
        {
            SetValue(cJSON_GetStringValue(jsonValue));
        }
        else
        {
            SetValue(jsonValue->valueint);
        }
    }
    FTAction::FTAction(ActionTypes actionParm, char *jsonString)
    {
        Type = actionParm;
        SetValue(jsonString);
    }
    void FTAction::Execute()
    {
        Menu *menu = NULL;
        ESP_LOGD(module, "Executing Action type: %s, value: %d, symbol: %s", enum_to_string(Type), value, symbol);
        switch (Type)
        {
        case ActionTypes::NONE:
            break;

        case ActionTypes::DELAY:
            delay(value);
            break;
        case ActionTypes::ARROWS_AND_TAB:
            bleKeyboard.write(ArrowsAndTab[value]);
            break;
        case ActionTypes::MEDIAKEY:
            if (value >= 0)
            {
                for (auto kp : Keys[value])
                {
                    ESP_LOGD(module, "Sending Media Key 0X%04X", (kp));
                    bleKeyboard.write(kp);
                }
            }
            break;
        case ActionTypes::LETTERS:
        case ActionTypes::SPECIAL_CHARS:
            bleKeyboard.print(symbol);
            break;
        case ActionTypes::OPTIONKEYS:
            if (value < sizeof(OptionKeys) / sizeof(uint8_t) && value >= 0)
            {
                ESP_LOGD(module, "Pressing key 0X%04X", OptionKeys[value]);
                bleKeyboard.press(OptionKeys[value]);
            }
            else
            {
                ESP_LOGD(module, "Releasing all keys");
                bleKeyboard.releaseAll();
            }
            break;
        case ActionTypes::FUNCTIONKEYS:
            ESP_LOGD(module, "Pressing key 0X%04X", FunctionKeys[value]);
            bleKeyboard.press(FunctionKeys[value]);
            break;
        case ActionTypes::NUMBERS:
            ESP_LOGD(module, "printing ", value);
            bleKeyboard.print(value);
            break;

        case ActionTypes::COMBOS:
            for (auto k : Combos[value])
            {
                ESP_LOGD(module, "Pressing key 0X%04X", k);
                bleKeyboard.press(k);
            }
            break;
        case ActionTypes::HELPERS:
            if (generalconfig.modifier1 != 0)
            {
                ESP_LOGD(module, "Pressing modifier 0X%04X", generalconfig.modifier1);
                bleKeyboard.press(generalconfig.modifier1);
            }
            if (generalconfig.modifier2 != 0)
            {
                ESP_LOGD(module, "Pressing modifier 0X%04X", generalconfig.modifier2);
                bleKeyboard.press(generalconfig.modifier2);
            }
            if (generalconfig.modifier3 != 0)
            {
                ESP_LOGD(module, "Pressing modifier 0X%04X", generalconfig.modifier3);
                bleKeyboard.press(generalconfig.modifier3);
            }
            ESP_LOGD(module, "Pressing helpers 0X%04X", Helpers[value]);
            bleKeyboard.press(Helpers[value]);
            ESP_LOGD(module, "Releasing all", Helpers[value]);
            bleKeyboard.releaseAll();
            break;
        case ActionTypes::MENU:
            if (SetActiveScreen)
            {
                ESP_LOGD(module, "Selecting menu %s\n", symbol);
                SetActiveScreen(symbol);
            }
            break;
        case ActionTypes::SETLATCH:
        case ActionTypes::CLEARLATCH:
        case ActionTypes::TOGGLELATCH:
            RunLatchAction(this);
            break;
        case ActionTypes::LOCAL:
            ESP_LOGD(module, "Executing local action %s", enum_to_string((LocalActionTypes)value));
            switch ((LocalActionTypes)value)
            {
            case LocalActionTypes::ENTER_CONFIG:
                /* code */
                configmode();
                break;
            case LocalActionTypes::BRIGHTNESS_DOWN:
                ChangeBrightness(Direction::DOWN);
                break;
            case LocalActionTypes::BRIGHTNESS_UP:
                ChangeBrightness(Direction::UP);
                break;
            case LocalActionTypes::SLEEP:
                if (generalconfig.sleepenable)
                {
                    generalconfig.sleepenable = false;
                    ESP_LOGI(module, "Sleep disabled.");
                }
                else
                {
                    generalconfig.sleepenable = true;
                    Interval = generalconfig.sleeptimer * 60000;
                    ESP_LOGI(module, "Sleep Enabled. Timer set to %d", generalconfig.sleeptimer);
                }
                break;
            case LocalActionTypes::INFO:
                printinfo();
                break;
            default:
                break;
            }
            break;

        default:
            break;
        }
    }
    void FTAction::Init()
    {
    }
    const char *FTAction::toString()
    {

        switch (Type)
        {
        case ActionTypes::NONE:
            break;

        case ActionTypes::DELAY:
            snprintf(printBuffer, sizeof(printBuffer), "Type: %s, delay: %d", enum_to_string(Type), value);
            break;
        case ActionTypes::ARROWS_AND_TAB:
            snprintf(printBuffer, sizeof(printBuffer), "Type: %s, key: 0X%04X", enum_to_string(Type), ArrowsAndTab[value]);
            break;
        case ActionTypes::MEDIAKEY:
            if (value >= 0)
            {
                snprintf(printBuffer, sizeof(printBuffer), "Type: %s, key: ", enum_to_string(Type));
                for (auto kp : Keys[value])
                {
                    snprintf(printBuffer, sizeof(printBuffer), "%s 0X%04X", printBuffer, kp);
                }
            }
            break;
        case ActionTypes::LETTERS:
        case ActionTypes::SPECIAL_CHARS:
        case ActionTypes::MENU:
            snprintf(printBuffer, sizeof(printBuffer), "Type: %s, value: %s", enum_to_string(Type), symbol);
            break;
        case ActionTypes::OPTIONKEYS:
            if (value < sizeof(OptionKeys) / sizeof(uint8_t) && value >= 0)
            {
                snprintf(printBuffer, sizeof(printBuffer), "Type: %s, key 0X%04X", enum_to_string(Type), OptionKeys[value]);
            }
            else
            {
                snprintf(printBuffer, sizeof(printBuffer), "Type: %s, Release all", enum_to_string(Type));
            }
            break;
        case ActionTypes::FUNCTIONKEYS:
            snprintf(printBuffer, sizeof(printBuffer), "Type: %s, key 0X%04X", enum_to_string(Type), FunctionKeys[value]);
            break;
        case ActionTypes::NUMBERS:
            snprintf(printBuffer, sizeof(printBuffer), "Type: %s, Number: %d", enum_to_string(Type), value);
            break;

        case ActionTypes::COMBOS:
            snprintf(printBuffer, sizeof(printBuffer), "Type: %s, ", enum_to_string(Type));
            for (auto k : Combos[value])
            {
                snprintf(printBuffer, sizeof(printBuffer), "%s 0X%04X", printBuffer, k);
            }
            break;
        case ActionTypes::HELPERS:
            snprintf(printBuffer, sizeof(printBuffer), "Type: %s, key: ", enum_to_string(Type));
            if (generalconfig.modifier1 != 0)
            {
                snprintf(printBuffer, sizeof(printBuffer), "%s 0X%04X", printBuffer, generalconfig.modifier1);
            }
            if (generalconfig.modifier2 != 0)
            {
                snprintf(printBuffer, sizeof(printBuffer), "%s 0X%04X", printBuffer, generalconfig.modifier2);
            }
            if (generalconfig.modifier3 != 0)
            {
                snprintf(printBuffer, sizeof(printBuffer), "%s 0X%04X", printBuffer, generalconfig.modifier3);
            }
            snprintf(printBuffer, sizeof(printBuffer), "%s 0X%04X", printBuffer, Helpers[value]);
            break;
        case ActionTypes::LOCAL:
            snprintf(printBuffer, sizeof(printBuffer), "Type: %s, local action: %s", enum_to_string(Type), enum_to_string((LocalActionTypes)value));
            break;

        default:
            break;
        }

        return printBuffer;
    }

    FTAction *PopScreenQueue()
    {
        FTAction *Action = NULL;

        if (QueueLock(portMAX_DELAY / portTICK_PERIOD_MS))
        {
            if (!ScreenQueue.empty())
            {
                Action = ScreenQueue.front();
                ScreenQueue.pop();
            }
            QueueUnlock();
        }
        else
        {
            ESP_LOGE(module, "Unable to screen lock Action queue");
        }
        return Action;
    }
    FTAction *PopQueue()
    {
        FTAction *Action = NULL;

        if (QueueLock(portMAX_DELAY / portTICK_PERIOD_MS))
        {
            if (!Queue.empty())
            {
                Action = Queue.front();
                Queue.pop();
            }
            QueueUnlock();
        }
        else
        {
            ESP_LOGE(module, "Unable to lock Action queue");
        }
        return Action;
    }
    bool QueueLock(TickType_t xTicksToWait)
    {
        ESP_LOGV(module, "Locking Action Queue object");
        if (xSemaphoreTake(xQueueSemaphore, xTicksToWait) == pdTRUE)
        {
            ESP_LOGV(module, "Action Queue object  locked!");
            return true;
        }
        else
        {
            ESP_LOGE(module, "Unable to lock the Action queue object");
            return false;
        }
    }

    void QueueUnlock()
    {
        ESP_LOGV(module, "Unlocking the Action queue object");
        xSemaphoreGive(xQueueSemaphore);
    }
    bool QueueAction(FTAction *action)
    {
        if (!QueueLock(100 / portTICK_PERIOD_MS))
        {
            ESP_LOGE(module, "Unable to queue new action ");
            return false;
        }
        if (action->Type == ActionTypes::MENU ||
            action->Type == ActionTypes::LOCAL ||
            action->Type == ActionTypes::SETLATCH ||
            action->Type == ActionTypes::CLEARLATCH ||
            action->Type == ActionTypes::TOGGLELATCH)
        {
            ESP_LOGD(module, "Pushing action type %s to screen queue", enum_to_string(action->Type));
            ScreenQueue.push(action);
        }
        else
        {
            ESP_LOGD(module, "Pushing action type %s to regular queue", enum_to_string(action->Type));
            Queue.push(action);
        }

        QueueUnlock();
        return true;
    }
}