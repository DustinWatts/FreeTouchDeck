#pragma once
#include "BleKeyboard.h"
#include "globals.hpp"

namespace FreeTouchDeck
{
    class FTAction;
    class ActionsSequences;

    enum class ActionTypes
    {
        NONE = 0,
        KEYBOARD,
        LOCAL,
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
    typedef std::vector<uint8_t> KeyValue_t;
    typedef std::map<const char *, KeyValue_t> KeyMap_t;
    typedef std::list<FTAction *> ActionsList;
    typedef std::list<std::string> ParametersList_t;
    class FTAction
    {
    public:
        ActionTypes Type;
        bool NeedsRelease;
        bool NeedsDoubleBytes;
        KeyValue_t Values;
        ParametersList_t Parameters;
        static const char *JsonLabelType;
        static const char *JsonLabelValue;
        static const char *JsonLabelSymbol;
        FTAction();
        FTAction(char *jsonActionType, char *jsonValueStr);
        FTAction(ParametersList_t parameters);
        FTAction(const char *keyName, KeyValue_t values);
        FTAction(KeyValue_t values);
        static bool SplitParameters(const char *parmString, ParametersList_t &parameters);
        static void InitConstants();
        void ParseModifierKey(char *modifier);
        ~FTAction();
        static bool IsValidKey(const char *name, char **foundValue);
        static bool ParseToken(const char *token, ActionsSequences *sequences);

        static bool KeyNeedsRelease(const char *keyName);
        static bool KeyIsDoubleBytes(const char *keyName);
        void Execute();
        const char *toString();
        std::string& GetParameter(int index);
        const char *ActionName();
        const char *FirstParameter();
        const char *SecondParameter();
        const char *ThirdParameter();
        static std::string& GetParameter(int index, ParametersList_t &parameters);
        static ActionTypes GetType(cJSON *jsonActionType, ParametersList_t &parameters);
        bool SplitActionParameter(char *name, size_t nameSize, char *parameter, size_t parameterSize);
        static bool SplitActionParameter(const char *value, char *name, size_t nameSize, char *parameter, size_t parameterSize);
        inline bool IsScreen()
        {
            return Type != ActionTypes::KEYBOARD;
        }

        static FTAction releaseAllAction;
        static FTAction rebootSystem;
        bool CallActionCallback(bool checkOnly = false);
        static bool CallActionCallback(ParametersList_t &parameters, FTAction *action, bool checkOnly);
        static bool IsActionCallback(ParametersList_t &parameters);
    };

    typedef std::function<bool(FTAction *)> ActionCallbackFn_t;
    typedef std::map<std::string, ActionCallbackFn_t> ActionCallbackMap_t;

    extern ActionCallbackMap_t UserActions;
    extern FTAction *sleepSetLatchAction;
    extern FTAction *sleepClearLatchAction;
    extern FTAction *sleepToggleLatchAction;
    extern FTAction *beepSetLatchAction;
    extern FTAction *beepClearLatchAction;
    extern bool QueueAction(FTAction *action);
    extern FTAction *PopQueue();
    extern bool QueueLock(TickType_t xTicksToWait);
    extern void QueueUnlock();
    extern FTAction *PopScreenQueue();

}