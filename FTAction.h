#pragma once
#include "BleKeyboard.h"
#include "globals.hpp"
#include <vector>
#include <map>
namespace FreeTouchDeck
{
    enum class ActionTypes
    {
        NONE = 0,
        DELAY,
        ARROWS_AND_TAB,
        MEDIAKEY,
        LETTERS,
        OPTIONKEYS,
        FUNCTIONKEYS,
        NUMBERS,
        SPECIAL_CHARS,
        COMBOS,
        HELPERS,
        LOCAL,
        MENU,
        FREETEXT,
        SETLATCH,
        CLEARLATCH,
        TOGGLELATCH,
        RELEASEALL,
        ENDLIST
    };
    inline ActionTypes &operator++(ActionTypes &state, int)
    {
        int i = static_cast<int>(state) + 1;
        i = i >= (int)ActionTypes::ENDLIST ? (int)ActionTypes::NONE : i;
        state = static_cast<ActionTypes>(i);
        return state;
    }
    const char *enum_to_string(ActionTypes type);

    enum class LocalActionTypes
    {
        NONE = 0,
        ENTER_CONFIG,
        BRIGHTNESS_DOWN,
        BRIGHTNESS_UP,
        SLEEP,
        INFO,
        REBOOT,
        BEEP,
        ENDLIST

    };
    typedef std::vector<uint8_t> KeyValue_t;
    typedef std::map<const char *, KeyValue_t> KeyMap_t;
    typedef std::map<ActionTypes, KeyMap_t> KeyMaps_t;
    typedef struct
    {
        ActionTypes Type;
        KeyValue_t Values;
    } KeySequence_t;
    typedef std::list<KeySequence_t> KeySequences_t;

    inline LocalActionTypes &
    operator++(LocalActionTypes &state, int)
    {
        int i = static_cast<int>(state) + 1;
        i = i >= (int)LocalActionTypes::ENDLIST ? (int)LocalActionTypes::NONE : i;
        state = static_cast<LocalActionTypes>(i);
        return state;
    }
    const char *enum_to_string(LocalActionTypes type);

    class FTAction
    {
    public:
        ActionTypes Type;
        LocalActionTypes LocalActionType;
        uint8_t value;
        char *symbol = NULL;
        static const char *JsonLabelType;
        static const char *JsonLabelValue;
        static const char *JsonLabelSymbol;
        static const char *JsonLabelLocalActionType;
        FTAction(ActionTypes actionParm, const char *jsonString);
        FTAction(char *jsonActionType, char *jsonValueStr);
        FTAction(cJSON *jsonActionType, cJSON *jsonValue);
        FTAction(cJSON *jsonActionType);
        void ParseModifierKey(char * modifier);
        ~FTAction();
        static const char *GetNthElementKey(ActionTypes actionType, uint8_t index);
        static bool get_by_index(ActionTypes actionType, const char *index, const char **value);
        static const KeyMap_t GetMap(ActionTypes actionType);
        static bool ParseToken(const char *token, KeyValue_t *values, ActionTypes *type);
        bool ParseFreeText(const char *text, KeySequences_t *keySequences);
        cJSON *ToJson();
        void Execute();
        static ActionTypes GetType(cJSON *jsonActionType);
        void *operator new(size_t sz)
        {
            LOC_LOGV("FTAction", "operator new : %d", sz);
            return malloc_fn(sz);
        };

        bool IsLatch();
        bool GetLatchButton(char *screen, size_t screenSize, char *button, size_t buttonSize);
        const char *toString();
        inline bool IsString()
        {
            return Type == ActionTypes::LETTERS || Type == ActionTypes::MEDIAKEY || Type == ActionTypes::SPECIAL_CHARS || Type == ActionTypes::MENU || Type == ActionTypes::SETLATCH || Type == ActionTypes::CLEARLATCH || Type == ActionTypes::TOGGLELATCH;
        }
        inline bool IsScreen()
        {
            return Type == ActionTypes::MENU ||
                   Type == ActionTypes::LOCAL ||
                   Type == ActionTypes::SETLATCH ||
                   Type == ActionTypes::CLEARLATCH ||
                   Type == ActionTypes::TOGGLELATCH;
        }
        inline bool IsBTSequence()
        {
            return Type == ActionTypes::ARROWS_AND_TAB ||
                   Type == ActionTypes::COMBOS ||
                   Type == ActionTypes::FUNCTIONKEYS ||
                   Type == ActionTypes::HELPERS ||
                   Type == ActionTypes::LETTERS ||
                   Type == ActionTypes::MEDIAKEY ||
                   Type == ActionTypes::NUMBERS ||
                   Type == ActionTypes::OPTIONKEYS ||
                   Type == ActionTypes::FREETEXT;
        }
        static bool parse(const char *value, ActionTypes *result);
        bool NeedsRelease;
        static bool parse(const char *value, LocalActionTypes *result);
        static bool parse(const char *keyname, ActionTypes actionType, KeyValue_t *keyvalue, const char **foundKey = NULL);
        static bool parse(const char *keyname, ActionTypes actionType, KeyValue_t *keyvalue, char **foundKey = NULL);
        bool ParseBTSequence();
        //        bool parse(char *keyname, ActionTypes actionType, KeyValue_t *keyvalue, const char **foundKey = NULL);
        //        bool parse(const char *keyname, ActionTypes actionType,  KeyValue_t * keyvalue,char **foundKey=NULL);
        static bool parse(char *keyname, ActionTypes actionType, KeyValue_t *keyvalue, char **foundKey = NULL);
        static FTAction releaseAllAction;
        static FTAction rebootSystem;

    private:
        void SetType(const char *jsonTypeStr);
        void SetValue(const char *jsonValue);
        void SetType(int jsonType);
        void SetValue(int jsonValue);
        KeySequences_t KeySequences;
    };

    typedef bool (*ActionCallbackFn_t)(FTAction *);
    typedef struct ActionCallbacks
    {
        ActionCallbackFn_t RunLatchAction;
        ActionCallbackFn_t ChangeBrightness;
        ActionCallbackFn_t SetSleep;
        ActionCallbackFn_t PrintInfo;
        ActionCallbackFn_t ConfigMode;
        ActionCallbackFn_t SetActiveScreen;
        ActionCallbackFn_t SetBeep;

    } ActionCallbacks_t;

    ActionCallbacks_t *ActionsCallbacks();
    extern FTAction *sleepSetLatchAction;
    extern FTAction *sleepClearLatchAction;
    extern FTAction *sleepToggleLatchAction;
    extern  FTAction *beepSetLatchAction;
    extern FTAction *beepClearLatchAction;
    extern bool QueueAction(FTAction *action);
    extern FTAction *PopQueue();
    extern bool QueueLock(TickType_t xTicksToWait);
    extern void QueueUnlock();
    extern FTAction *PopScreenQueue();

}