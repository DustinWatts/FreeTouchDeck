#include "FTAction.h"
#include "BleKeyboard.h"
#include "UserConfig.h"
#include "MenuNavigation.h"
#include "Menu.h"
#include "globals.hpp"
#include <algorithm>
#include <cstdio>
static const char *module = "FTAction";
extern BleKeyboard bleKeyboard;
extern Config generalconfig;
extern unsigned long Interval;

using namespace std;

static char printBuffer[101] = {0};

namespace FreeTouchDeck
{
    const char *splitterFormat = "%[^.: ,-]%*[.: ,-]%s";
    SemaphoreHandle_t xQueueSemaphore = xSemaphoreCreateMutex();
    std::queue<FTAction *> Queue;
    std::queue<FTAction *> ScreenQueue;
    const char *unknown = "Unknown";
    const char *FTAction::JsonLabelType = "type";
    const char *FTAction::JsonLabelValue = "value";
    const char *FTAction::JsonLabelSymbol = "symbol";

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
    
    bool FTAction::IsValidKey(ActionTypes actionType, const char * name, char ** foundValue)
    {
        bool found=false;
        KeyValue_t kv;
        ASSING_IF_PASSED(foundValue,NULL);

        if(ISNULLSTRING(name))
        {
            LOC_LOGD(module,"Empty key type %s passed", enum_to_string(actionType));
            found=false;
        }
        else
        {
          char * foundKey=NULL;
          if(FTAction::parse(name, actionType, &kv, &foundKey))
          {
            found=true;
            LOC_LOGD(module,"Parsed modifier with key %s", foundKey);
            if(*foundValue)
            {
                FREE_AND_NULL(*foundValue);
                *foundValue=foundKey;
                found=true;
            }
          }
          else 
          {
            LOC_LOGE(module, "Invalid option key %s ", name);
          }
        }
        return false;
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

    bool FTAction::ParseToken(const char *token, KeySequences_t *keySequences)
    {
        FTAction *action = new FTAction(ActionTypes::LOCAL, token);

        // first check if the token matches a user action
        if (action->Type != ActionTypes::NONE)
        {
            KeySequence_t keySequence = {.Type = action->Type};
            keySequence.action = action;
            keySequences->push_back(keySequence);
            return true;
        }
        else
        {
            delete (action);
        }
        return false;
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
            if (*p == '{' && tokenStart && *tokenStart == '{')
            {
                //two curly brackets is interpreted as a single
                // curly bracket char to be sent
                //todo:  revise this logic
            }
            else if (*p == '{' || *p == '\0')
            {

                if (values.size() > 0)
                {
                    KeySequence_t sequence = {.Type = ActionTypes::LETTERS, .Values = values};
                    keySequences->push_back(sequence);
                    std::string valStr(values.begin(), values.end());
                    LOC_LOGD(module, "Sequence found with len %d: %s", values.size(), valStr); //, str.c_str());
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
                    LOC_LOGD(module, "Token isn't a key sequence");
                    KeySequence_t sequence = {.Type = type, .Values = values};
                    keySequences->push_back(sequence);
                    values.clear();
                }
                else if (ParseToken(token, keySequences))
                {
                    LOC_LOGD(module, "Successfully parsed action token %s", token);
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

    bool FTAction::SplitActionParameter(const char *value, char *name, size_t nameSize, char *parameter, size_t parameterSize)
    {
        char localName[strlen(value) + 1] = {0};
        char localParameter[strlen(value) + 1] = {0};

        if (!name || !parameter || nameSize == 0 || parameterSize == 0)
        {
            LOC_LOGE(module,"Missing parameters: %s%s%s%s", name?"":"Missing name ", parameter?"":"Missing parameter ", nameSize>0?"":"Name buffer is null ", parameterSize>0?"":"parameter buffer is null");
            return false;
        }
        LOC_LOGD(module,"Scanning for value");
        int ret = std::sscanf(value, splitterFormat, localName, localParameter);
        if (ret != 2)
        {
            LOC_LOGD(module, "Nothing tp split %s", value);
            return false;
        }
        LOC_LOGD(module, "Found Name %s and parameter %s", localName, localParameter);
        strncpy(name, localName, nameSize);
        strncpy(parameter, localParameter, parameterSize);
        return true;
    }
    bool FTAction::SplitActionParameter(char *name, size_t nameSize, char *parameter, size_t parameterSize)
    {
        return (IsLatch() && SplitActionParameter(symbol, name, nameSize, parameter, parameterSize));
    }
    void FTAction::ParseModifierKey(char *modifier)
    {
        KeyValue_t KeyValue;
        KeySequence_t Sequence;
        if (ISNULLSTRING(modifier))
            return;
        if (parse(modifier, ActionTypes::OPTIONKEYS, &KeyValue))
        {
            LOC_LOGD(module,"Adding modifier key %s",modifier);
            Sequence.Type = ActionTypes::FUNCTIONKEYS;
            Sequence.Values = KeyValue;
            KeySequences.push_back(Sequence);
            NeedsRelease = true;
        }
        else
        {
            LOC_LOGE(module,"Invalid modifier %s", STRING_OR_DEFAULT(modifier,""));
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
            for(auto modifier:generalconfig.modifiers)
            {
                ParseModifierKey(modifier);
            }
            if (parse(symbol, Type, &KeyValue, &foundKey))
            {
                Sequence.Type = Type;
                Sequence.Values = KeyValue;
                FREE_AND_NULL(symbol);
                symbol = foundKey;
                LOC_LOGD(module,"Adding key %s", STRING_OR_DEFAULT(symbol,""));
                KeySequences.push_back(Sequence);
            }
            break;
        case ActionTypes::FREETEXT:
        LOC_LOGD(module,"Parsing free text sequence %s",STRING_OR_DEFAULT(symbol,""));
            ParseFreeText(symbol, &KeySequences);
            break;
        default:
            break;
        }
    }

    void FTAction::SetValue(const char *jsonValue)
    {
        if (ISNULLSTRING(jsonValue))
        {
            LOC_LOGD(module, "No value to assign for action");
            return;
        }
        LOC_LOGD(module, "Evaluating value %s",jsonValue);
        char * name = (char *)malloc_fn(strlen(jsonValue) + 1);
        char * parameter = (char *)malloc_fn(strlen(jsonValue) + 1);
        if(IsValidKey)
        if (Type == ActionTypes::LOCAL)
        {
            LOC_LOGD(module, "Action Type local. Checking if we have a callback named %s", jsonValue);

            if (CallActionCallback(jsonValue, NULL, true))
            {
                LOC_LOGD(module, "Found user Action %s", jsonValue);
                ActionName = ps_strdup(jsonValue);
                symbol = ps_strdup("");
            }
            else
            {
                LOC_LOGD(module, "Trying to split action parameter");
                if (SplitActionParameter(jsonValue, name, sizeof(name) - 1, parameter, sizeof(parameter) - 1))
                {
                    LOC_LOGD(module, "Action type local. Attempting to find corresponding callback");
                    if (CallActionCallback(name, NULL, true))
                    {
                        LOC_LOGD(module, "Found user Action %s with parameter %s", name, parameter);
                        ActionName = ps_strdup(name);
                        symbol = ps_strdup(parameter);
                    }
                    else
                    {
                        LOC_LOGE(module, "Invalid local action type %s", jsonValue);
                        Type == ActionTypes::NONE;
                    }
                }
                else
                {
                    LOC_LOGE(module, "Invalid local action type %s", jsonValue);
                    Type == ActionTypes::NONE;
                }
            }
        }
        else if (IsLatch())
        {
            if (!SplitActionParameter(jsonValue, name, sizeof(name) - 1, parameter, sizeof(parameter) - 1))
            {
                LOC_LOGE(module, "Parsing of latch value %s failed.", jsonValue);
            }
            else
            {
                symbol = ps_strdup(jsonValue);
            }
        }
        else if (IsBTSequence())
        {
            symbol=ps_strdup(jsonValue);
            if (!ParseBTSequence())
            {
                LOC_LOGE(module, "Error parsing BT key sequences");
            }
        }
        else
        {
            symbol = ps_strdup(jsonValue);
            LOC_LOGD(module, "Found value %s", symbol);
        }
        FREE_AND_NULL(name);
        FREE_AND_NULL(parameter);
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
        LOC_LOGD(module, "Instantiating action type %s", enum_to_string(Type));
        SetValue(jsonString);
    }
    void FTAction::Execute()
    {
        std::map<const char *, ActionCallbackFn_t>::iterator callback;
        Menu *menu = NULL;
        MediaKeyReport MediaKey;
        KeyValue_t KeyValue;
        LOC_LOGD(module, "Executing Action: %s", toString());

        switch (Type)
        {
        case ActionTypes::NONE:
            break;

        case ActionTypes::DELAY:
            delay(atol(symbol));
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
                    delay(generalconfig.keyDelay);
                }
                else if (kv.Type == ActionTypes::LOCAL)
                {
                    CallActionCallback(kv.action);
                }
                else
                {
                    for (auto ks : kv.Values)
                    {
                        bleKeyboard.write(ks);
                        delay(generalconfig.keyDelay);
                    }
                }
            }
            break;
            break;
        case ActionTypes::MENU:
            CallActionCallback("MENU", this);
            break;
        case ActionTypes::SETLATCH:
        case ActionTypes::CLEARLATCH:
        case ActionTypes::TOGGLELATCH:
            CallActionCallback("TOGGLELATCH", this);
            break;
        case ActionTypes::RELEASEALL:
            bleKeyboard.releaseAll();
            break;
        case ActionTypes::LOCAL:
            CallActionCallback(this);
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
            snprintf(printBuffer, sizeof(printBuffer), "Type: %s, delay: %s", enum_to_string(Type), STRING_OR_DEFAULT(symbol,""));
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
        case ActionTypes::OPTIONKEYS:
        case ActionTypes::NUMBERS:
        case ActionTypes::LOCAL:
            snprintf(printBuffer, sizeof(printBuffer), "Type: %s, value: %s%s%s", enum_to_string(Type), STRING_OR_DEFAULT(ActionName,""),ISNULLSTRING(ActionName)?"":":",STRING_OR_DEFAULT(symbol,""));
            break;
        case ActionTypes::HELPERS:
            for(auto modifier:generalconfig.modifiers)
            {
                snprintf(printBuffer, sizeof(printBuffer), "%s %s", printBuffer, STRING_OR_DEFAULT(modifier,""));                
            }
            snprintf(printBuffer, sizeof(printBuffer), "%s %s", printBuffer, STRING_OR_DEFAULT(symbol,""));
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
                LOC_LOGV(module, "Screen Action Queue Length : %d", ScreenQueue.size());
                Action = ScreenQueue.front();
                ScreenQueue.pop();
                LOC_LOGV(module, "Screen Action Queue Length : %d", ScreenQueue.size());
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
                LOC_LOGV(module, "Action Queue Length : %d", Queue.size());
                Action = Queue.front();
                Queue.pop();
                LOC_LOGV(module, "Action Queue Length : %d", Queue.size());
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

        if (IsString())
        {
            LOC_LOGD(module, "Parsing value from symbol %s", STRING_OR_DEFAULT(symbol,""));
            GetValueOrDefault(jsonActionType, FTAction::JsonLabelSymbol, &symbol, "");
            SetValue(symbol);
        }
        else
        {
            LOC_LOGD(module, "Parsing numeric value from value");
            GetValueOrDefault(jsonActionType, FTAction::JsonLabelValue, &symbol, 0);
            LOC_LOGD(module, "Value %s ", symbol);
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
        if (Type == ActionTypes::LOCAL)
        {
            const char *tokenFormat = "%s:%s";
            if (!ISNULLSTRING(symbol))
            {
                size_t len = snprintf(NULL, 0, tokenFormat, ActionName, symbol);
                char *localSymbol = (char *)malloc_fn(len + 1);
                snprintf(localSymbol, len, tokenFormat, ActionName, symbol);
                cJSON_AddStringToObject(action, FTAction::JsonLabelSymbol, localSymbol);
                FREE_AND_NULL(localSymbol);
            }
            else
            {
                cJSON_AddStringToObject(action, FTAction::JsonLabelSymbol, symbol);
            }
        }
        else if (IsString())
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
    FTAction FTAction::rebootSystem = FTAction(ActionTypes::LOCAL, "REBOOT");
    bool CallActionCallback(const char *name, FTAction *action, bool checkOnly)
    {
        ActionCallbackFn_t callbackFn = NULL;
        auto callback = FreeTouchDeck::UserActions.find(name);
        if (callback == FreeTouchDeck::UserActions.end())
        {
            LOC_LOGW(module, "Find failed. Trying to iterate entries");
            for (auto c : FreeTouchDeck::UserActions)
            {
                if (c.first == name)
                {
                    callbackFn = c.second;
                    LOC_LOGD(module, "Found the callback with a loop in the map values");
                    break;
                }
            }
        }
        else
        {
            LOC_LOGD(module, "Found the callback in the map");
            callbackFn = callback->second;
        }

        if (callbackFn)
        {
            if (checkOnly)
            {
                return true;
            }
            else
            {
                return callbackFn(action);
            }
        }
        else
        {
            LOC_LOGE(module, "Invalid callback name %s. Valid callbacks are: ", name);
            for (auto c : FreeTouchDeck::UserActions)
            {
                LOC_LOGE(module, "    %s", c.first.c_str());
            }
        }
        return false;
    }
    bool CallActionCallback(FTAction *action, bool checkOnly)
    {
        return CallActionCallback(action->ActionName, action, checkOnly);
    }
}