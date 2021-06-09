#pragma once

#include "FTAction.h"
namespace FreeTouchDeck {

class ActionsSequences
{
    public:
    char *ConfigSequence;
   // bool NeedsReleaseAll;
    std::list<FTAction *> Actions;
    bool Execute();
    bool Parse(cJSON * actionJson);
    bool Parse(const char * actionString);
};

typedef std::list<ActionsSequences> ActionSequencesList;
};