#pragma once
#include "stdint.h"
#include <vector>
#include "BleKeyboard.h"
#include "cJSON.h"
namespace FreeTouchDeck {

enum class ActionTypes{
    NONE=0,
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
    MENU
} ;
enum class LocalActionTypes {
    NONE=0,
    ENTER_CONFIG,
    BRIGHTNESS_DOWN,
    BRIGHTNESS_UP,
    SLEEP,
    INFO,
    
};
    class FTAction 
    {
        public:
            ActionTypes Type;
            uint8_t value;
            FTAction(ActionTypes actionParm,char * jsonString);
            FTAction(char * jsonActionType, char * jsonValueStr);
            FTAction(cJSON * jsonActionType, cJSON * jsonValue);
            void Execute();
            void Init();
            static ActionTypes GetType(cJSON * jsonActionType);
        protected:
            char * symbol;
        private: 
            void SetType(const char * jsonTypeStr);
            void SetValue(char * jsonValue);
            void SetType(int jsonType);
            void SetValue(int  jsonValue);
    };
    
}