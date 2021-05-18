#include "FTAction.h"
#include "BleKeyboard.h"
#include "UserConfig.h"
#include "MenuNavigation.h"

extern BleKeyboard bleKeyboard;
extern Config generalconfig;
extern int ledBrightness;
extern unsigned long Interval;
using namespace std;
#define MEDIA_2_VECTOR(m) \
    {                     \
        m[0], m[1]        \
    }
namespace FreeTouchDeck
{
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

    void FTAction::SetType(const char *jsonTypeStr)
    {
        Type = (ActionTypes)atol(jsonTypeStr);
    }

    void FTAction::SetType(int jsonType)
    {
        Type = (ActionTypes)jsonType;
    }

    void FTAction::SetValue(char *jsonValue)
    {
        if (Type == ActionTypes::LETTERS || Type == ActionTypes::SPECIAL_CHARS || Type == ActionTypes::MENU)
        {
            if(!jsonValue || strlen(jsonValue)==0)
            {
                ESP_LOGW(module,"Empty string value for action type %s",enum_to_string(Type));
            }
            else 
            {
                ESP_LOGD(module,"Assigning string value %s for action type %s",jsonValue,enum_to_string(Type));
            }
            symbol = ps_strdup(jsonValue);
        }
        else
        {
            value = atol(jsonValue);
            ESP_LOGD(module,"Converting value %s to number %d for action type %s",jsonValue,value,enum_to_string(Type));
        }
    }
    void FTAction::SetValue(int jsonValue)
    {
        char buffer[33];
        if (Type == ActionTypes::LETTERS || Type == ActionTypes::SPECIAL_CHARS)
        {
            symbol = ps_strdup(itoa(jsonValue, buffer, sizeof(buffer) - 1));
            ESP_LOGD(module,"Assigning string value %s from number %d for action type %s",symbol,jsonValue,enum_to_string(Type));
        }
        else
        {
            ESP_LOGD(module,"Assigning number %d for action type %s",value,enum_to_string(Type));
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
        switch (Type)
        {
        case ActionTypes::NONE:
            Serial.println("Action Type is NONE");
            break;

        case ActionTypes::DELAY:
            Serial.println("Action Type is DELAY");
            delay(value);
            break;
        case ActionTypes::ARROWS_AND_TAB:
            Serial.println("Action Type is ARROWS_AND_TAB");
            bleKeyboard.write(ArrowsAndTab[value]);
            break;
        case ActionTypes::MEDIAKEY:
            Serial.printf("Action Type is MEDIAKEY, with value index %d of %d. Value(s): ", value - 1, Keys.size());
            if (value - 1 >= 0)
            {
                for (auto kp : Keys[value])
                {
                    Serial.print(kp);
                    Serial.print(" ");
                    bleKeyboard.write(kp);
                }
            }

            Serial.println();

            break;
        case ActionTypes::LETTERS:
        case ActionTypes::SPECIAL_CHARS:
            Serial.print("Action Type is LETTERS/SPECIAL_CHARS with symbol: ");
            Serial.println(symbol);
            bleKeyboard.print(symbol);
            break;
        case ActionTypes::OPTIONKEYS:
            Serial.print("Action Type is OPTIONKEYS. ");
            if (value < sizeof(OptionKeys) / sizeof(uint8_t) && value >= 0)
            {
                Serial.print(" Pressing: ");
                Serial.print(OptionKeys[value]);
                bleKeyboard.press(OptionKeys[value]);
            }
            else
            {
                Serial.print("Releasing all");
                bleKeyboard.releaseAll();
            }
            Serial.println();
            break;
        case ActionTypes::FUNCTIONKEYS:
            Serial.print("Action Type is FUNCTIONKEYS. ");
            Serial.print(" Pressing: ");
            Serial.print(FunctionKeys[value]);
            bleKeyboard.press(FunctionKeys[value]);
            Serial.println();
            break;
        case ActionTypes::NUMBERS:
            Serial.print("Action Type is NUMBERS. ");
            Serial.print(" Printing: ");
            Serial.print(value);
            bleKeyboard.print(value);
            Serial.println();
            break;

        case ActionTypes::COMBOS:
            Serial.print("Action Type is COMBOS. Pressing: ");
            for (auto k : Combos[value])
            {
                Serial.print(k);
                Serial.print(" ");
                bleKeyboard.press(k);
            }
            Serial.println();
            break;
        case ActionTypes::HELPERS:
            Serial.print("Action Type is HELPERS. Pressing: ");
            if (generalconfig.modifier1 != 0)
            {
                Serial.print(generalconfig.modifier1);
                Serial.print(" ");
                bleKeyboard.press(generalconfig.modifier1);
            }
            if (generalconfig.modifier2 != 0)
            {
                Serial.print(generalconfig.modifier2);
                Serial.print(" ");
                bleKeyboard.press(generalconfig.modifier2);
            }
            if (generalconfig.modifier3 != 0)
            {
                Serial.print(generalconfig.modifier3);
                Serial.print(" ");
                bleKeyboard.press(generalconfig.modifier3);
            }
            Serial.print(" and ");
            Serial.print(Helpers[value - 1]);
            bleKeyboard.press(Helpers[value - 1]);
            Serial.println(". Releasing all.");

            bleKeyboard.releaseAll();
            break;
        case ActionTypes::MENU:
            if (SetActiveScreen)
            {
                Serial.printf("Selecting menu %s\n", symbol);
                SetActiveScreen(symbol);
            }
            break;
        case ActionTypes::LOCAL:
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
                    Serial.println("[INFO]: Sleep disabled.");
                }
                else
                {
                    generalconfig.sleepenable = true;
                    Interval = generalconfig.sleeptimer * 60000;
                    Serial.println("[INFO]: Sleep enabled.");
                    Serial.print("[INFO]: Timer set to: ");
                    Serial.println(generalconfig.sleeptimer);
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
}