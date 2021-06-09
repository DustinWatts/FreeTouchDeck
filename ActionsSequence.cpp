#include "ActionsSequence.h"
namespace FreeTouchDeck
{

    bool ActionsSequences::Execute()
    {
        bool needsRelease = false;
        for (auto action : Actions)
        {
            needsRelease = action->NeedsRelease ? true : needsRelease;
            LOC_LOGD(module, "Queuing action %s",  action->toString());
            if (!QueueAction(action))
            {
                LOC_LOGW(module, "Button action %s could not be queued for execution.", action->toString());
            }
            // if(NeedsReleaseAll)
            // {
            //     QueueAction(&FTAction::releaseAllAction);
            // }
        }
    }
    bool ActionsSequences::Parse(const char * actionString)
    {
        if(ISNULLSTRING(actionString))
        {
            LOC_LOGD(module,"Null or empty action sequence received");
            return false;
        }
        ConfigSequence = ps_strdup(actionString);
        const char *p = actionString;
        const char *tokenStart = actionString;
        char token[101] = {0};
        KeyValue_t values;
        KeyValue_t releaseKeyList;
        FTAction * releaseAction=NULL;
        ParametersList_t releaseParameters;
        bool success = true;
        

        //NeedsReleaseAll = false;
        LOC_LOGD(module, "Parsing free form text %s", ConfigSequence);
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
                    char *buf = (char *)malloc_fn(values.size() + 1);
                    memcpy(buf, values.data(), values.size());
                    Actions.push_back(new FTAction(buf, values));
                    LOC_LOGD(module, "Character Sequence found with len %d: %s", values.size(), STRING_OR_DEFAULT(buf, ""));
                    FREE_AND_NULL(buf);
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
                if (!FTAction::ParseToken(token, this))
                {
                    LOC_LOGE(module, "Invalid token %s found", token);
                    success = false;
                }
                else 
                {
                    auto lastAction=Actions.back();
                    if(lastAction->NeedsRelease)
                    {
                        releaseKeyList.insert(releaseKeyList.end(), lastAction->Values.begin(), lastAction->Values.end()); 
                    }
                }
                values.clear();
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
        if(releaseKeyList.size()>0)
        {
           releaseParameters.push_back("RELEASEKEY");
           auto releaseAction = new FTAction(releaseParameters);
           releaseAction->Values.insert(releaseAction->Values.end(), releaseKeyList.begin(),releaseKeyList.end());
           Actions.push_back(releaseAction);
        }

        return success;
    }
    bool ActionsSequences::Parse(cJSON *actionJson)
    {
        if (!actionJson)
        {
            LOC_LOGE(module, "empty action json object");
            return false;
        }
        if (!cJSON_IsString(actionJson))
        {
            LOC_LOGE(module, "action json object is not a string");
            return false;
        }
        if (ISNULLSTRING(cJSON_GetStringValue(actionJson)))
        {
            LOC_LOGE(module, "empty action json object string passed");
            return false;
        }
        return Parse(cJSON_GetStringValue(actionJson));
    }

};