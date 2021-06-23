#pragma once
#include "globals.hpp"
#include "System.h"
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
    typedef std::vector<FTAction *> ActionsList;
    typedef std::vector<std::string> ParametersList_t;
    typedef std::vector<std::string> ActionQueueType_t;
    extern ActionQueueType_t UserActionsKeyboardQueue;
    class FTAction
    {
    public:
        ActionTypes Type;
        bool NeedsRelease;
        bool NeedsDoubleBytes;
        uint16_t HoldTime=0;
        KeyValue_t Values;
        ParametersList_t Parameters;
        static const char *JsonLabelType;
        static const char *JsonLabelValue;
        static const char *JsonLabelSymbol;
        FTAction();
        FTAction(char *jsonActionType, char *jsonValueStr);
        FTAction(const ParametersList_t &parameters);
        FTAction(const char *keyName, const KeyValue_t &values);
        FTAction(const KeyValue_t &values);
        static bool SplitParameters(const char *parmString, ParametersList_t &parameters);
        static void InitConstants();
        void ParseModifierKey(char *modifier);
        ~FTAction();
        static bool IsValidKey(const char *name, char **foundValue);
        static bool ParseToken(const char *token, std::vector<FTAction *> &sequences);

        static bool KeyNeedsRelease(const char *keyName);
        static bool KeyIsDoubleBytes(const char *keyName);
        void Execute();
        static void Stop();
        const char *toString();
        std::string& GetParameter(int index);
        const char *ActionName();
        const char *FirstParameter();
        const char *SecondParameter();
        const char *ThirdParameter();
        std::string& ActionNameStr();
        std::string& FirstParameterStr();
        std::string& SecondParameterStr();
        std::string& ThirdParameterStr();
        static std::string& GetParameter(int index, ParametersList_t &parameters);
        static ActionTypes GetType(cJSON *jsonActionType, ParametersList_t &parameters);
        bool SplitActionParameter(char *name, size_t nameSize, char *parameter, size_t parameterSize);
        static bool SplitActionParameter(const char *value, char *name, size_t nameSize, char *parameter, size_t parameterSize);
        inline bool IsScreen()
        {
            bool KeyboardLocalAction=false;
            for(const std::string &e : UserActionsKeyboardQueue)
            {
                if(e==ActionNameStr())
                {
                    KeyboardLocalAction=true;
                    break;
                }
            }

            // Screen queue if not Keyboard event and acction name not in the Keyboard queue list
            return  Type != ActionTypes::KEYBOARD && !KeyboardLocalAction;
        }
        static FTAction rebootSystem;
        bool CallActionCallback(bool checkOnly = false);
        static bool CallActionCallback(ParametersList_t &parameters, FTAction *action, bool checkOnly);
        static bool IsActionCallback(ParametersList_t &parameters);
        static bool Stopped;
    };

    typedef std::function<bool(FTAction *)> ActionCallbackFn_t;
    typedef std::map<std::string, ActionCallbackFn_t> ActionCallbackMap_t;
    extern const ActionCallbackMap_t UserActions;
    extern bool QueueAction(FTAction *action);
    extern FTAction *PopQueue();
    void EmptyQueue();
    cJSON * UserActionsJson();
    cJSON *KeyNamesJson();
    size_t QueueSize();
    extern bool QueueLock(TickType_t xTicksToWait);
    extern void QueueUnlock();
    extern FTAction *PopScreenQueue();

}