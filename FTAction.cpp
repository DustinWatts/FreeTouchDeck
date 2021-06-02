#include "FTAction.h"
#include "BleKeyboard.h"
#include "UserConfig.h"
#include "MenuNavigation.h"
#include "Menu.h"
#include "globals.hpp"
#include <algorithm>
static const char *module = "FTAction";
extern BleKeyboard bleKeyboard;
extern Config generalconfig;
extern int ledBrightness;
extern unsigned long Interval;

using namespace std;

static char printBuffer[101] = {0};

namespace FreeTouchDeck
{
    SemaphoreHandle_t xQueueSemaphore = xSemaphoreCreateMutex();
    std::queue<FTAction *> Queue;
    std::queue<FTAction *> ScreenQueue;
    const char *unknown = "Unknown";
    const char *FTAction::JsonLabelType = "type";
    const char *FTAction::JsonLabelLocalActionType = "localactiontype";
    const char *FTAction::JsonLabelValue = "value";
    const char *FTAction::JsonLabelSymbol = "symbol";
    ActionCallbacks_t callbacks = {0};
    const char *enum_to_string(ActionTypes type)
    {
        switch (type)
        {
            ENUM_TO_STRING_HELPER(ActionTypes, NONE);
            ENUM_TO_STRING_HELPER(ActionTypes, DELAY);
            ENUM_TO_STRING_HELPER(ActionTypes, ARROWS_AND_TAB);
            ENUM_TO_STRING_HELPER(ActionTypes, MEDIAKEY);
            ENUM_TO_STRING_HELPER(ActionTypes, LETTERS);
            ENUM_TO_STRING_HELPER(ActionTypes, OPTIONKEYS);
            ENUM_TO_STRING_HELPER(ActionTypes, FUNCTIONKEYS);
            ENUM_TO_STRING_HELPER(ActionTypes, NUMBERS);
            ENUM_TO_STRING_HELPER(ActionTypes, SPECIAL_CHARS);
            ENUM_TO_STRING_HELPER(ActionTypes, COMBOS);
            ENUM_TO_STRING_HELPER(ActionTypes, HELPERS);
            ENUM_TO_STRING_HELPER(ActionTypes, LOCAL);
            ENUM_TO_STRING_HELPER(ActionTypes, MENU);
            ENUM_TO_STRING_HELPER(ActionTypes, FREETEXT);
            ENUM_TO_STRING_HELPER(ActionTypes, SETLATCH);
            ENUM_TO_STRING_HELPER(ActionTypes, CLEARLATCH);
            ENUM_TO_STRING_HELPER(ActionTypes, TOGGLELATCH);
            ENUM_TO_STRING_HELPER(ActionTypes, RELEASEALL);
        default:
            return unknown;
        }
    }
    const char *enum_to_string(LocalActionTypes type)
    {
        switch (type)
        {
            ENUM_TO_STRING_HELPER(LocalActionTypes, NONE);
            ENUM_TO_STRING_HELPER(LocalActionTypes, ENTER_CONFIG);
            ENUM_TO_STRING_HELPER(LocalActionTypes, BRIGHTNESS_DOWN);
            ENUM_TO_STRING_HELPER(LocalActionTypes, BRIGHTNESS_UP);
            ENUM_TO_STRING_HELPER(LocalActionTypes, BEEP);
            ENUM_TO_STRING_HELPER(LocalActionTypes, SLEEP);
            ENUM_TO_STRING_HELPER(LocalActionTypes, REBOOT);            
            ENUM_TO_STRING_HELPER(LocalActionTypes, INFO);
        default:
            return unknown;
        }
    }

#define KEY_TO_MAP(k)         \
    {                         \
        QUOTE(k), { KEY_##k } \
    }
#define MEDIAKEY_TO_MAP(k)                               \
    {                                                    \
        QUOTE(k), { KEY_MEDIA_##k[0], KEY_MEDIA_##k[1] } \
    }
#define START_END_KEY \
    {                 \
        "", { 0 }     \
    }
    const KeyMaps_t KeyMaps = {
        {ActionTypes::MEDIAKEY, {MEDIAKEY_TO_MAP(MUTE), MEDIAKEY_TO_MAP(VOLUME_DOWN), MEDIAKEY_TO_MAP(VOLUME_UP), MEDIAKEY_TO_MAP(PLAY_PAUSE), MEDIAKEY_TO_MAP(STOP), MEDIAKEY_TO_MAP(NEXT_TRACK), MEDIAKEY_TO_MAP(PREVIOUS_TRACK), MEDIAKEY_TO_MAP(WWW_HOME), MEDIAKEY_TO_MAP(LOCAL_MACHINE_BROWSER), MEDIAKEY_TO_MAP(CALCULATOR), MEDIAKEY_TO_MAP(WWW_BOOKMARKS), MEDIAKEY_TO_MAP(WWW_SEARCH), MEDIAKEY_TO_MAP(WWW_STOP), MEDIAKEY_TO_MAP(WWW_BACK), MEDIAKEY_TO_MAP(CONSUMER_CONTROL_CONFIGURATION), MEDIAKEY_TO_MAP(EMAIL_READER)}},
        {ActionTypes::FUNCTIONKEYS, {KEY_TO_MAP(F1), KEY_TO_MAP(F2), KEY_TO_MAP(F3), KEY_TO_MAP(F4), KEY_TO_MAP(F5), KEY_TO_MAP(F6), KEY_TO_MAP(F7), KEY_TO_MAP(F8), KEY_TO_MAP(F9), KEY_TO_MAP(F10), KEY_TO_MAP(F11), KEY_TO_MAP(F12), KEY_TO_MAP(F13), KEY_TO_MAP(F14), KEY_TO_MAP(F15), KEY_TO_MAP(F16), KEY_TO_MAP(F17), KEY_TO_MAP(F18), KEY_TO_MAP(F19), KEY_TO_MAP(F20), KEY_TO_MAP(F21), KEY_TO_MAP(F22), KEY_TO_MAP(F23), KEY_TO_MAP(F24)}},
        {ActionTypes::ARROWS_AND_TAB, {KEY_TO_MAP(UP_ARROW), KEY_TO_MAP(DOWN_ARROW), KEY_TO_MAP(LEFT_ARROW), KEY_TO_MAP(RIGHT_ARROW), KEY_TO_MAP(BACKSPACE), KEY_TO_MAP(TAB), KEY_TO_MAP(RETURN), KEY_TO_MAP(PAGE_UP), KEY_TO_MAP(PAGE_DOWN), KEY_TO_MAP(DELETE)}},
        {ActionTypes::OPTIONKEYS, {KEY_TO_MAP(LEFT_CTRL), KEY_TO_MAP(LEFT_SHIFT), KEY_TO_MAP(LEFT_ALT), KEY_TO_MAP(LEFT_GUI), KEY_TO_MAP(RIGHT_CTRL), KEY_TO_MAP(RIGHT_SHIFT), KEY_TO_MAP(RIGHT_ALT), KEY_TO_MAP(RIGHT_GUI)}},
        {ActionTypes::COMBOS, {{"LEFT_CTRL+LEFT_SHIFT", {KEY_LEFT_CTRL, KEY_LEFT_SHIFT}}, {"LALT+LSHIFT", {KEY_LEFT_ALT, KEY_LEFT_SHIFT}}, {"LGUI+LSHIFT", {KEY_LEFT_GUI, KEY_LEFT_SHIFT}}, {"LCTRL+LGUI", {KEY_LEFT_CTRL, KEY_LEFT_GUI}}, {"LALT+LGUI", {KEY_LEFT_ALT, KEY_LEFT_GUI}}, {"LCTRL+LALT", {KEY_LEFT_CTRL, KEY_LEFT_ALT}}, {"LCTRL+LALT+LGUI", {KEY_LEFT_CTRL, KEY_LEFT_ALT, KEY_LEFT_GUI}}, {"RCTL+RSHIFT", {KEY_RIGHT_CTRL, KEY_RIGHT_SHIFT}}, {"RALT+RSHIFT", {KEY_RIGHT_ALT, KEY_RIGHT_SHIFT}}, {"RGUI+RSHIFT", {KEY_RIGHT_GUI, KEY_RIGHT_SHIFT}}, {"RCTL+RGUI", {KEY_RIGHT_CTRL, KEY_RIGHT_GUI}}, {"RALT+RGUI", {KEY_RIGHT_ALT, KEY_RIGHT_GUI}}, {"RCTL+RALT", {KEY_RIGHT_CTRL, KEY_RIGHT_ALT}}, {"RCTL+RALT+RGUI", {KEY_RIGHT_CTRL, KEY_RIGHT_ALT, KEY_RIGHT_GUI}}}}};

    FTAction::~FTAction()
    {
        FREE_AND_NULL(symbol);
        for (auto ks : KeySequences)
        {
            ks.Values.clear();
        }
        KeySequences.clear();
    }
    const KeyMap_t FTAction::GetMap(ActionTypes actionType)
    {
        return KeyMaps.find(actionType)->second;
    }
    const char *FTAction::GetNthElementKey(ActionTypes actionType, uint8_t index)
    {
        KeyMap_t m = GetMap(actionType);
        uint8_t curPos = 0;
        for (KeyMap_t::iterator it = m.begin(); it != m.end(); ++it)
        {
            LOC_LOGV(module, "Looking for element #%d.  Current element #%d: %s", index, curPos, it->first);
            // numbered elements start at index 1
            if (++curPos == index)
            {
                return it->first;
            }
        }
        return "";
    }
    bool FTAction::get_by_index(ActionTypes actionType, const char *index, const char **value)
    {
        const KeyMap_t map = KeyMaps.find(actionType)->second;
        if (!value)
            return false;
        *value = index;
        bool success = false;
        if (index && index[0] >= '0' && index[0] <= '9')
        {
            // Parsing numeric local action type, which starts
            // at offset zero.  Push by one
            LOC_LOGD(module, "Getting entry from number %s", index);
            uint8_t val = atol(index);
            if (map.size() > val)
            {
                const char *testVal = NULL;
                testVal = GetNthElementKey(actionType, val);
                if (ISNULLSTRING(testVal))
                {
                    success = false;
                }
                else
                {
                    ASSING_IF_PASSED(value, testVal);
                    success = true;
                }

                LOC_LOGD(module, "Option key name is %s", *value);
            }
            else
            {
                LOC_LOGE(module, "Number %d, from string %s does not match an option key", val, *value);
            }
        }
        return success;
    }

    bool FTAction::parse(const char *value, ActionTypes *result)
    //bool parse(const char *value, ActionTypes *result)
    {
        *result = ActionTypes::NONE;
        ActionTypes LocalResult = ActionTypes::NONE;
        const char *resultStr = NULL;
        bool success = false;
        do
        {
            LocalResult++; // Start after NONE
            resultStr = enum_to_string(LocalResult);
            LOC_LOGV(module, "%s?=%s", resultStr, value);

        } while (strcmp(value, resultStr) != 0 && LocalResult != ActionTypes::NONE);
        if (strcmp(value, resultStr) == 0)
        {
            *result = LocalResult;
            success = true;
        }
        return success;
    }
    bool FTAction::parse(const char *value, LocalActionTypes *result)
    //bool parse(const char *value, LocalActionTypes *result)
    {
        LocalActionTypes LocalResult = LocalActionTypes::NONE;
        *result = LocalActionTypes::NONE;
        bool success = false;
        const char *resultStr = NULL;
        do
        {
            LocalResult++; // Start after NONE
            resultStr = enum_to_string(LocalResult);
            LOC_LOGV(module, "%s?=%s", resultStr, value);

        } while (strcmp(value, resultStr) != 0 && LocalResult != LocalActionTypes::NONE);
        if (strcmp(value, resultStr) == 0)
        {
            *result = LocalResult;
            success = true;
        }
        return success;
    }
    bool FTAction::parse(const char *keyname, ActionTypes actionType, KeyValue_t *keyvalue, const char **foundKey)
    //bool parse(const char *keyname, ActionTypes actionType,  KeyValue_t * keyvalue,const char **foundKey)
    {
        const char *searchName = NULL;
        ASSING_IF_PASSED(foundKey, keyname);
        KeyMap_t map = GetMap(actionType);
        bool success = false;

        if (keyname && strlen(keyname) > 0)
        {
            if (get_by_index(actionType, keyname, &searchName))
            {
                ASSING_IF_PASSED(foundKey, searchName);
            }
            auto found = map.find(searchName);
            if (found != map.end())
            {
                ASSING_IF_PASSED(keyvalue, found->second);
                success = true;
            }
            else
            {
                LOC_LOGE(module, "Key %s was not found for type %s", keyname, enum_to_string(actionType));
            }
        }
        return success;
    }
    bool FTAction::parse(const char *keyname, ActionTypes actionType, KeyValue_t *keyvalue, char **foundKey)
    //bool parse(const char *keyname, ActionTypes actionType,  KeyValue_t * keyvalue,char **foundKey)
    {
        const char *foundKeyConst = NULL;
        bool success = parse(keyname, actionType, keyvalue, &foundKeyConst);
        ASSING_IF_PASSED(foundKey, ps_strdup(foundKeyConst));
        return success;
    }
    bool FTAction::parse(char *keyname, ActionTypes actionType, KeyValue_t *keyvalue, char **foundKey)
    //bool parse(const char *keyname, ActionTypes actionType,  KeyValue_t * keyvalue,char **foundKey)
    {
        const char *foundKeyConst = NULL;
        bool success = parse(keyname, actionType, keyvalue, &foundKeyConst);
        ASSING_IF_PASSED(foundKey, ps_strdup(foundKeyConst));
        return success;
    }

    bool FTAction::ParseToken(const char *token, KeyValue_t *values, ActionTypes *type)
    {
        for (auto m : KeyMaps)
        {
            for (auto km : m.second)
            {
                if (strcmp(km.first, token) == 0)
                {
                    *values = km.second;
                    LOC_LOGD(module, "Found free form token: %s", token);
                    return true;
                }
            }
        }
        LOC_LOGE(module, "Free form token %s not found", token);
        return false;
    }
    bool FTAction::ParseFreeText(const char *text, KeySequences_t *keySequences)
    {
        const char *p = text;
        const char *tokenStart = text;
        char token[101] = {0};
        KeyValue_t values;
        ActionTypes type;
        if (p == '\0')
            return false;
        LOC_LOGD(module, "Parsing free form text %s", text);
        do
        {
            if (*p == '{' || *p == '\0')
            {
                if (values.size() > 0)
                {
                    KeySequence_t sequence = {.Type = ActionTypes::LETTERS, .Values = values};
                    //std::string str(values.begin(), values.end());
                    LOC_LOGD(module, "Sequence found with len %d: %s", values.size(), values); //, str.c_str());
                    values.clear();
                }

                tokenStart = p;
            }
            else if (*p == '}' && tokenStart != NULL)
            {
                memset(token, 0x00, sizeof(token));
                size_t len = min((size_t)(p - tokenStart - 1), (size_t)(sizeof(token) - 1));
                strncpy(token, tokenStart + 1, len);
                LOC_LOGD(module, "Found token %s", token);
                if (ParseToken(token, &values, &type))
                {
                    KeySequence_t sequence = {.Type = type, .Values = values};
                    keySequences->push_back(sequence);
                    values.clear();
                }
                tokenStart = p + 1;
            }
            else
            {
                values.push_back(*p);
            }
            if (*p == '\0')
            {
                break;
            }
            p++;
        } while (true);
    }
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
    void FTAction::ParseModifierKey(char *modifier)
    {
        KeyValue_t KeyValue;
        KeySequence_t Sequence;
        if (ISNULLSTRING(modifier))
            return;
        if (parse(modifier, ActionTypes::OPTIONKEYS, &KeyValue))
        {
            Sequence.Type = ActionTypes::FUNCTIONKEYS;
            Sequence.Values = KeyValue;
            KeySequences.push_back(Sequence);
            NeedsRelease = true;
        }
    }
    bool FTAction::ParseBTSequence()
    {
        KeyValue_t KeyValue;
        KeySequence_t Sequence;
        char modVal[2] = {0};
        char *foundKey = NULL;
        NeedsRelease = false;
        if (ISNULLSTRING(symbol))
        {
            LOC_LOGE(module, "No symbol provided");
            return false;
        }
        if (!IsBTSequence())
            return false;
        LOC_LOGD(module, "Parsing value  %s", symbol);
        Sequence.Type = Type;
        switch (Type)
        {
        case ActionTypes::ARROWS_AND_TAB:
            if (parse(symbol, Type, &KeyValue, &foundKey))
            {
                Sequence.Values = KeyValue;
                KeySequences.push_back(Sequence);
                FREE_AND_NULL(symbol);
                symbol = foundKey;
            }
            else
            {
                LOC_LOGE(module, "Invalid arrows and tab key %s", symbol);
            }
            NeedsRelease = true;
            break;
        case ActionTypes::MEDIAKEY:
            if (parse(symbol, Type, &KeyValue, &foundKey))
            {
                Sequence.Values = KeyValue;
                KeySequences.push_back(Sequence);
                FREE_AND_NULL(symbol);
                symbol = foundKey;
            }
            else
            {
                LOC_LOGE(module, "Invalid media key %s", symbol);
            }
            break;
        case ActionTypes::LETTERS:
        case ActionTypes::SPECIAL_CHARS:
        case ActionTypes::NUMBERS:

            KeyValue.assign((uint8_t *)symbol, (uint8_t *)symbol + strlen(symbol));
            Sequence.Values = KeyValue;
            KeySequences.push_back(Sequence);
            break;
        case ActionTypes::COMBOS:
        case ActionTypes::OPTIONKEYS:
        case ActionTypes::FUNCTIONKEYS:
            if (parse(symbol, Type, &KeyValue, &foundKey))
            {
                Sequence.Values = KeyValue;
                KeySequences.push_back(Sequence);
            }
            NeedsRelease = true;
            break;
        case ActionTypes::HELPERS:
            // todo:  parse modifiers too!
            ParseModifierKey(generalconfig.modifier1);
            ParseModifierKey(generalconfig.modifier2);
            ParseModifierKey(generalconfig.modifier3);
            if (parse(symbol, Type, &KeyValue, &foundKey))
            {
                Sequence.Type = Type;
                Sequence.Values = KeyValue;
                FREE_AND_NULL(symbol);
                symbol = foundKey;
                KeySequences.push_back(Sequence);
            }
            break;
        case ActionTypes::FREETEXT:
            ParseFreeText(symbol, &KeySequences);
            break;
        default:
            break;
        }
    }

    void FTAction::SetValue(const char *jsonValue)
    {
        symbol = ps_strdup(jsonValue);

        if (Type == ActionTypes::LOCAL)
        {
            LocalActionTypes localType = LocalActionTypes::NONE;
            parse(STRING_OR_DEFAULT(jsonValue, "NONE"), &localType);

            if (localType == LocalActionTypes::NONE)
            {
                LOC_LOGE(module, "Unexpected action type none while parsing vaue for local action type");
            }
            else
            {
                LOC_LOGD(module, "Local action type is %s", enum_to_string(localType));
                LocalActionType = localType;
            }
        }
        else if (IsBTSequence())
        {
            if (!ParseBTSequence())
            {
                LOC_LOGE(module, "Error parsing BT key sequences");
            }
        }
        else if (IsString())
        {
            LOC_LOGD(module, "Found value %s", symbol);
        }
        else
        {
            FREE_AND_NULL(symbol);
            LOC_LOGE(module, "No value found for action type %s", enum_to_string(Type));
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
            char *charValue = cJSON_GetStringValue(jsonActionType);
            if (charValue[0] >= '0' && charValue[0] <= '9')
            {
                return (ActionTypes)atol(cJSON_GetStringValue(jsonActionType));
            }
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
            //   SetValue(jsonValue->valueint);
            LOC_LOGE(module(, "Setting numeric value not implemented"));
        }
    }
    FTAction::FTAction(ActionTypes actionParm, const char *jsonString)
    {
        Type = actionParm;
        SetValue(jsonString);
    }
    void FTAction::Execute()
    {
        Menu *menu = NULL;
        MediaKeyReport MediaKey;
        KeyValue_t KeyValue;
        LOC_LOGD(module, "Executing Action: %s", toString());

        switch (Type)
        {
        case ActionTypes::NONE:
            break;

        case ActionTypes::DELAY:
            delay(value);
            break;
        case ActionTypes::LETTERS:
        case ActionTypes::SPECIAL_CHARS:
        case ActionTypes::ARROWS_AND_TAB:
        case ActionTypes::OPTIONKEYS:
        case ActionTypes::MEDIAKEY:
        case ActionTypes::FUNCTIONKEYS:
        case ActionTypes::NUMBERS:
        case ActionTypes::FREETEXT:
        case ActionTypes::HELPERS:
        case ActionTypes::COMBOS:
            for (auto kv : KeySequences)
            {
                if (kv.Type == ActionTypes::MEDIAKEY)
                {
                    MediaKey[0] = kv.Values[0];
                    MediaKey[1] = kv.Values[1];
                    bleKeyboard.write(MediaKey);
                }
                else
                {
                    for (auto ks : kv.Values)
                    {
                        bleKeyboard.write(ks);
                    }
                }
            }

            break;
            break;
        case ActionTypes::MENU:
            EXECUTE_IF_EXISTS(callbacks.SetActiveScreen, this);
            break;
        case ActionTypes::SETLATCH:
        case ActionTypes::CLEARLATCH:
        case ActionTypes::TOGGLELATCH:
            EXECUTE_IF_EXISTS(callbacks.RunLatchAction, this);
            break;
        case ActionTypes::RELEASEALL:
            bleKeyboard.releaseAll();
            break;
        case ActionTypes::LOCAL:

            switch (LocalActionType)
            {
            case LocalActionTypes::ENTER_CONFIG:
                /* code */
                EXECUTE_IF_EXISTS(callbacks.ConfigMode, this);
                break;
            case LocalActionTypes::BRIGHTNESS_DOWN:
                EXECUTE_IF_EXISTS(callbacks.ChangeBrightness, this);
                break;
            case LocalActionTypes::BRIGHTNESS_UP:
                EXECUTE_IF_EXISTS(callbacks.ChangeBrightness, this);
                break;
            case LocalActionTypes::BEEP:
                EXECUTE_IF_EXISTS(callbacks.SetBeep, this);
                break;
            case LocalActionTypes::SLEEP:
                EXECUTE_IF_EXISTS(callbacks.SetSleep, this);
                break;
            case LocalActionTypes::REBOOT:
                ESP.restart();
                break;
            case LocalActionTypes::INFO:
                EXECUTE_IF_EXISTS(callbacks.PrintInfo, NULL);
                break;
            default:
                break;
            }
            break;

        default:
            break;
        }
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

        case ActionTypes::FUNCTIONKEYS:
        case ActionTypes::MEDIAKEY:
        case ActionTypes::ARROWS_AND_TAB:
        case ActionTypes::LETTERS:
        case ActionTypes::SPECIAL_CHARS:
        case ActionTypes::MENU:
        case ActionTypes::SETLATCH:
        case ActionTypes::CLEARLATCH:
        case ActionTypes::TOGGLELATCH:
        case ActionTypes::COMBOS:
        case ActionTypes::FREETEXT:
            snprintf(printBuffer, sizeof(printBuffer), "Type: %s, value: %s", enum_to_string(Type), symbol);
            break;
        case ActionTypes::OPTIONKEYS:
            snprintf(printBuffer, sizeof(printBuffer), "Type: %s, key: %s", enum_to_string(Type), STRING_OR_DEFAULT(symbol, "Release All"));
            break;
        case ActionTypes::NUMBERS:
            snprintf(printBuffer, sizeof(printBuffer), "Type: %s, Number: %d", enum_to_string(Type), value);
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
            snprintf(printBuffer, sizeof(printBuffer), "%s %s", printBuffer, symbol);
            break;
        case ActionTypes::LOCAL:
            snprintf(printBuffer, sizeof(printBuffer), "Type: %s, local action: %s", enum_to_string(Type), enum_to_string(LocalActionType));
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
                LOC_LOGD(module, "Screen Action Queue Length : %d", ScreenQueue.size());
                Action = ScreenQueue.front();
                ScreenQueue.pop();
                LOC_LOGD(module, "Screen Action Queue Length : %d", ScreenQueue.size());
            }
            QueueUnlock();
        }
        else
        {
            LOC_LOGE(module, "Unable to screen lock Action queue");
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
                LOC_LOGD(module, "Action Queue Length : %d", Queue.size());
                Action = Queue.front();
                Queue.pop();
                LOC_LOGD(module, "Action Queue Length : %d", Queue.size());
            }
            QueueUnlock();
        }
        else
        {
            LOC_LOGE(module, "Unable to lock Action queue");
        }
        return Action;
    }
    bool QueueLock(TickType_t xTicksToWait)
    {
        LOC_LOGV(module, "Locking Action Queue object");
        if (xSemaphoreTake(xQueueSemaphore, xTicksToWait) == pdTRUE)
        {
            LOC_LOGV(module, "Action Queue object  locked!");
            return true;
        }
        else
        {
            LOC_LOGE(module, "Unable to lock the Action queue object");
            return false;
        }
    }

    void QueueUnlock()
    {
        LOC_LOGV(module, "Unlocking the Action queue object");
        xSemaphoreGive(xQueueSemaphore);
    }
    bool QueueAction(FTAction *action)
    {
        if (!QueueLock(100 / portTICK_PERIOD_MS))
        {
            LOC_LOGE(module, "Unable to queue new action ");
            return false;
        }
        if (action->IsScreen())
        {
            LOC_LOGD(module, "Pushing action %s to screen queue", action->toString());
            ScreenQueue.push(action);
        }
        else
        {
            LOC_LOGD(module, "Pushing action %s to regular queue", action->toString());
            Queue.push(action);
        }

        QueueUnlock();
        return true;
    }
    ActionCallbacks_t *ActionsCallbacks()
    {
        return &callbacks;
    }
    FTAction::FTAction(cJSON *jsonActionType)
    {
        char *value = NULL;
        DumpCJson(jsonActionType);
        LOC_LOGD(module, "Instantiating new Action from JSON");
        GetValueOrDefault(jsonActionType, FTAction::JsonLabelType, &value, "NONE");
        parse(value, &Type);
        LOC_LOGD(module, "Action type is %s ", enum_to_string(Type));
        FREE_AND_NULL(value);
        if (Type == ActionTypes::NONE)
        {
            return;
        }
        if (Type == ActionTypes::LOCAL)
        {
            GetValueOrDefault(jsonActionType, FTAction::JsonLabelLocalActionType, &value, "NONE");
            parse(value, &LocalActionType);
            LOC_LOGD(module, "Local action type %s ", enum_to_string(LocalActionType));
            FREE_AND_NULL(value);
        }
        else
        {
            LocalActionType = LocalActionTypes::NONE;
            if (IsString())
            {
                LOC_LOGD(module, "Parsing value from symbol");
                GetValueOrDefault(jsonActionType, FTAction::JsonLabelSymbol, &symbol, "");
            }
            else
            {
                LOC_LOGD(module, "Parsing value from value");
                GetValueOrDefault(jsonActionType, FTAction::JsonLabelValue, &value, 0);
                LOC_LOGD(module, "Value %d ", value);
            }
        }
    }

    cJSON *FTAction::ToJson()
    {
        PrintMemInfo();
        cJSON *action = cJSON_CreateObject();
        if (!action)
        {
            drawErrorMessage(true, module, "Memory allocation failed when rendering JSON action");
            return NULL;
        }
        LOC_LOGD(module, "Adding Action type %s to Json", enum_to_string(Type));
        cJSON_AddStringToObject(action, FTAction::JsonLabelType, enum_to_string(Type));
        if (IsString())
        {
            if (symbol && strlen(symbol) > 0)
            {
                LOC_LOGD(module, "Adding string value [%s] to Json", symbol);
                cJSON_AddStringToObject(action, FTAction::JsonLabelSymbol, symbol);
            }
            else
            {
                LOC_LOGD(module, "ERROR:String type Action has no symbol ");
            }
        }
        else if (Type == ActionTypes::LOCAL)
        {
            LOC_LOGD(module, "Adding local type %s to Json", enum_to_string(LocalActionType));
            cJSON_AddStringToObject(action, FTAction::JsonLabelLocalActionType, enum_to_string(LocalActionType));
        }
        else
        {
            LOC_LOGD(module, "Adding numeric value %d to Json", value);
            cJSON_AddNumberToObject(action, FTAction::JsonLabelValue, value);
        }
        if (generalconfig.LogLevel >= LogLevels::VERBOSE)
        {
            char *actionString = cJSON_Print(action);
            if (actionString)
            {
                LOC_LOGD(module, "Action json structure : \n%s", actionString);
                FREE_AND_NULL(actionString);
            }
            else
            {
                LOC_LOGE(module, "Unable to format JSON for output!");
            }
        }

        return action;
    }
    FTAction FTAction::releaseAllAction = FTAction(ActionTypes::RELEASEALL, "");
    FTAction FTAction::rebootSystem = FTAction(ActionTypes::LOCAL, enum_to_string(LocalActionTypes::REBOOT));
}