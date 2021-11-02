#pragma once

#include "FTAction.h"
namespace FreeTouchDeck {

class ActionsSequences
{
    public:
    char *ConfigSequence;
   // bool NeedsReleaseAll;
    std::vector<FTAction *> Actions;
    bool Execute();
    bool HasKeyboardAction();
    bool HasMenuAction();
    bool Parse(cJSON * actionJson);
    bool Parse(const char * actionString);
    bool HasAction(ActionTypes actionType, const char * name = NULL);
    ActionsSequences();
};

typedef std::vector<ActionsSequences > ActionSequencesList;
};