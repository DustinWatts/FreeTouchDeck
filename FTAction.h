#pragma once
#include "BleKeyboard.h"
#include "globals.hpp"
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
        SETLATCH,
        CLEARLATCH,
        TOGGLELATCH,
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
        ENDLIST

    };
    inline LocalActionTypes &operator++(LocalActionTypes &state, int)
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
        uint8_t value;
        char *symbol;
        FTAction(ActionTypes actionParm, char *jsonString);
        FTAction(char *jsonActionType, char *jsonValueStr);
        FTAction(cJSON *jsonActionType, cJSON *jsonValue);
        void Execute();
        static ActionTypes GetType(cJSON *jsonActionType);
        void *operator new(size_t sz)
        {
            ESP_LOGV("FTAction", "operator new : %d", sz);
            return malloc_fn(sz);
        };
        bool IsLatch();
        bool GetLatchButton(char *screen, size_t screenSize, char *button, size_t buttonSize);
        const char *toString();
        inline bool IsString()
        {
            return Type==ActionTypes::LETTERS || Type==ActionTypes::SPECIAL_CHARS || Type==ActionTypes::MENU || Type==ActionTypes::SETLATCH || Type==ActionTypes::CLEARLATCH || Type==ActionTypes::TOGGLELATCH;
        }
        inline bool IsScreen()
        {
            return Type == ActionTypes::MENU ||
            Type == ActionTypes::LOCAL ||
            Type == ActionTypes::SETLATCH ||
            Type == ActionTypes::CLEARLATCH ||
            Type == ActionTypes::TOGGLELATCH;
        }
    private:
        void SetType(const char *jsonTypeStr);
        void SetValue(char *jsonValue);
        void SetType(int jsonType);
        void SetValue(int jsonValue);
    };

    typedef bool (*ActionCallbackFn_t)(FTAction *);
    typedef struct ActionCallbacks
    {
        ActionCallbackFn_t RunLatchAction;
        ActionCallbackFn_t ChangeBrightness;
        ActionCallbackFn_t PrintInfo;
        ActionCallbackFn_t ConfigMode;
        ActionCallbackFn_t SetActiveScreen;
        
    } ActionCallbacks_t;

    ActionCallbacks_t *ActionsCallbacks();
    extern FTAction *sleepSetLatchAction;
    extern FTAction *sleepClearLatchAction;
    extern FTAction *sleepToggleLatchAction;

    extern bool QueueAction(FTAction *action);
    extern FTAction *PopQueue();
    extern bool QueueLock(TickType_t xTicksToWait);
    extern void QueueUnlock();
    extern FTAction *PopScreenQueue();

}