#pragma once
#include "BleKeyboard.h"
#include "globals.hpp"
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
const char *enum_to_string(ActionTypes type);

enum class LocalActionTypes {
    NONE=0,
    ENTER_CONFIG,
    BRIGHTNESS_DOWN,
    BRIGHTNESS_UP,
    SLEEP,
    INFO,
    
};
const char *enum_to_string(LocalActionTypes type);

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
            void* operator new(size_t sz) {
                ESP_LOGD("FTAction","operator new : %d",sz);
                 return malloc_fn(sz);
            }   
        protected:
            char * symbol;
        private: 
            void SetType(const char * jsonTypeStr);
            void SetValue(char * jsonValue);
            void SetType(int jsonType);
            void SetValue(int  jsonValue);
    };
    
}