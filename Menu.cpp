#include "Menu.h"
#include <cstdlib>
static const char *module = "Menu";
static const char *nameTemplate = "/config/%s.json";

// Overloading global new operator
void *operator new(size_t sz)
{
    //LOC_LOGD("Menu","operator new : %d",sz);
    void *m = malloc_fn(sz);
    return m;
}
namespace FreeTouchDeck
{

    FTAction *Menu::homeMenu = new FTAction(ParametersList_t({"MENU", "home"}));
    bool Menu::Button(FTAction *action)
    {
        bool success = false;
        FTButton *button = GetButton(action->SecondParameter());
        if (button)
        {
            LOC_LOGD(module, "Found button %s to run action %s", button->Label, action->toString());
            success = button->Latch(action);
            if (!success)
            {
                LOC_LOGE(module, "Running %s failed", action->toString());
            }
        }
        else
        {
            LOC_LOGE(module, "Could not find button %s to run action %s", action->SecondParameter(), action->toString());
        }
        return success;
    }
    FTButton *Menu::GetButton(const char *buttonName)
    {
        for (auto button : buttons)
        {
            if (strcmp(button->Label, buttonName) == 0)
            {
                return button;
            }
            LOC_LOGD(module, "Ignoring button %s", STRING_OR_DEFAULT(button->Label,""));
        }
        return NULL;
    }

    void Menu::Draw(bool force)
    {
        for (auto button : buttons)
        {
            button->Draw(force);
        }
        if (HasBackButton())
        {
            FTButton::BackButton->Draw(force);
        }
    }
    Menu::~Menu()
    {
        LOC_LOGD(module, "Freeing memory for menu %s", Name ? Name : "UNKNOWN");
        for (auto button : buttons)
        {
            delete (button);
        }
        FREE_AND_NULL(Name);
        FREE_AND_NULL(Label);
    }

    void Menu::ReleaseAll()
    {
        if (!Pressed)
            return;

        Pressed = false;
        LOC_LOGV(module, "Releasing all %d button ", buttons.size());
        for (auto button : buttons)
        {
            button->Release();
        }
        if (HasBackButton())
        {

            FTButton::BackButton->Release();
        }
    }
    void Menu::Activate()
    {
        if (!Active)
        {
            tft.fillScreen(BackgroundColor);
            Active = true;
            LOC_LOGD(module, "Activating menu %s", Name);
            if (HasBackButton())
            {
                FTButton::BackButton->SetCoordinates(ButtonWidth, ButtonHeight, (uint16_t)(buttons.size() / ColsCount), (uint16_t)(buttons.size() % ColsCount), Spacing);
            }
            Draw(true);
        }
    }
    void Menu::Deactivate()
    {
        if (Active)
        {
            LOC_LOGD(module, "Deactivating screen %s", Name);
            Active = false;
            ReleaseAll();
        }
    }

    void Menu::SetButtonWidth()
    {
        if (ColsCount == 0)
        {
            ColsCount = generalconfig.colscount;
        }
        if (RowsCount == 0)
        {
            RowsCount = generalconfig.rowscount;
        }
        LOC_LOGD(module, "Menu is organized in a %dx%d matrix", ColsCount, RowsCount);
        ButtonWidth = (tft.width() - max(ColsCount - 1, 0) * Spacing) / ColsCount;
        ButtonHeight = (tft.height() - max(RowsCount - 1, 0) * Spacing) / RowsCount;
    }

    void Menu::Touch(uint16_t x, uint16_t y)
    {
        Pressed = true;
        bool foundPressedButton = false;
        for (auto button : buttons)
        {
            if (button->contains(x, y))
            {
                foundPressedButton = true;
                button->Press();
            }
        }
        if (!foundPressedButton && HasBackButton())
        {
            if (FTButton::BackButton->contains(x, y))
            {
                foundPressedButton = true;
                FTButton::BackButton->Press();
            }
        }
        if (!foundPressedButton && buttons.size() == 0)
        {
            if (Actions.size() > 0)
            {
                // Some empty screens might be defined with a default
                // action. e.g. go back to previous menu, home screen, etc.
                for (auto action : Actions)
                {
                    action.Execute();
                    foundPressedButton = true;
                }
            }
            else if (strcmp("config", Name))
            {
                // in config mode, default to going back to home screen
                // when pressed
                QueueAction(Menu::homeMenu);
                foundPressedButton = true;
            }
        }
        else if (!foundPressedButton)
        {
            LOC_LOGD(module, "No button was found under touch coordinates");
        }
    }
    const char *Menu::JsonLabelName = "name";
    const char *Menu::JsonLabelIcon = "logo";
    const char *Menu::JsonLabelBackgroundColor = "backgroundcolor";
    const char *Menu::JsonLabelOutline = "outline";
    const char *Menu::JsonLabelTextColor = "textcolor";
    const char *Menu::JsonLabelRowsCount = "rowscount";
    const char *Menu::JsonLabelColsCount = "colscount";
    const char *Menu::JsonLabelButtons = "buttons";
    const char *Menu::JsonLabelActions = "actions";
    const char *Menu::JsonLabelType = "type";
    const char *Menu::JsonLabelLabel = "label";
    cJSON *Menu::ToJSON()
    {
        cJSON *menu = cJSON_CreateObject();
        if (!menu)
        {
            drawErrorMessage(true, module, "Memory allocation failed when rendering JSON menu");
            return NULL;
        }

        LOC_LOGD(module, "Adding members to Json");
        cJSON_AddStringToObject(menu, Menu::JsonLabelType, enum_to_string(Type));
        cJSON_AddStringToObject(menu, Menu::JsonLabelName, STRING_OR_DEFAULT(Name, ""));
        if (!ISNULLSTRING(Label))
        {
            cJSON_AddStringToObject(menu, Menu::JsonLabelLabel, Label);
        }
        if (!ISNULLSTRING(Icon))
        {
            cJSON_AddStringToObject(menu, Menu::JsonLabelIcon, Icon);
        }

        if (BackgroundColor != generalconfig.backgroundColour)
        {
            cJSON_AddStringToObject(menu, Menu::JsonLabelBackgroundColor, convertRGB888oHTMLRGB888(BackgroundColor));
        }
        if (_outline != generalconfig.DefaultOutline)
        {
            cJSON_AddStringToObject(menu, Menu::JsonLabelOutline, convertRGB888oHTMLRGB888(_outline));
        }
        if (_textColor != generalconfig.DefaultTextColor)
        {
            cJSON_AddStringToObject(menu, Menu::JsonLabelTextColor, convertRGB888oHTMLRGB888(_textColor));
        }
        if (ColsCount != generalconfig.colscount)
        {
            cJSON_AddNumberToObject(menu, Menu::JsonLabelColsCount, ColsCount);
        }
        if (RowsCount != generalconfig.rowscount)
        {
            cJSON_AddNumberToObject(menu, Menu::JsonLabelRowsCount, RowsCount);
        }

        if (buttons.size() > 0)
        {
            LOC_LOGD(module, "Adding buttons to Json");
            cJSON *buttonsJson = cJSON_CreateArray();
            for (auto button : buttons)
            {
                if (button->ButtonType != ButtonTypes::NONE)
                {
                    cJSON *buttonJson = button->ToJSON();
                    if (buttonJson)
                    {
                        cJSON_AddItemToArray(buttonsJson, buttonJson);
                    }
                }
            }
            cJSON_AddItemToObject(menu, Menu::JsonLabelButtons, buttonsJson);
        }

        if (Actions.size() > 0)
        {
            LOC_LOGD(module, "Adding %d actions to Json", Actions.size());
            cJSON *actionsJson = cJSON_CreateArray();
            if (!actionsJson)
            {
                drawErrorMessage(true, module, "Unable to allocate memory for actions");
            }
            for (auto action : Actions)
            {
                cJSON_AddItemToArray(actionsJson, cJSON_CreateString(action.ConfigSequence));
            }
            LOC_LOGD(module, "Converting actions is complete. Adding to menu object");
            cJSON_AddItemToObject(menu, Menu::JsonLabelActions, actionsJson);
        }
        return menu;
    }
    Menu *Menu::FromJson(const char *jsonString)
    {
        cJSON *doc = cJSON_Parse(jsonString);
        if (!doc)
        {

            const char *error = cJSON_GetErrorPtr();
            LOC_LOGE(module, "Menu parsing failed: %s", error);
            drawErrorMessage(true, module, "Unable to parse json string : %s", error);
            return NULL;
        }
        else
        {
            LOC_LOGD(module, "Json string parsed successfully");
        }
        return new Menu(doc);
    }
    Menu::Menu(MenuTypes menutype, const char *name, const char *label, const char *icon, uint8_t rowsCount, uint8_t colsCount, uint32_t backgroundColor, uint32_t outline, uint32_t textColor, uint8_t textSize)
    {
        Name = ps_strdup(name);
        Label = ps_strdup(STRING_OR_DEFAULT(label, ""));
        LOC_LOGD(module, "Label %s, name: %s", Label, Name);
        RowsCount = rowsCount;
        ColsCount = colsCount;
        BackgroundColor = backgroundColor;
        _outline = outline;
        _textColor = textColor;
        _textSize = textSize;
        LOC_LOGD(module, "Menu is a %dx%d matrix", RowsCount, ColsCount);
        // now that the rows and cols count are known, we can calculate the width of buttons
        SetButtonWidth();
        Type = menutype;
    }
    void Menu::AddButton(FTButton *button)
    {
        button->SetCoordinates(ButtonWidth, ButtonHeight, (uint16_t)(buttons.size() / ColsCount), (uint16_t)(buttons.size() % ColsCount), Spacing);
        buttons.push_back(button);
    }
    Menu::Menu(cJSON *menuJson)
    {
        char *value = NULL;
        GetValueOrDefault(menuJson, Menu::JsonLabelName, &Name, "");
        GetValueOrDefault(menuJson, Menu::JsonLabelLabel, &Label, "");
        LOC_LOGD(module, "Label %s, name: %s", STRING_OR_DEFAULT(Label,""),STRING_OR_DEFAULT(Name,""));
        GetValueOrDefault(menuJson, Menu::JsonLabelColsCount, &ColsCount, generalconfig.colscount);
        GetValueOrDefault(menuJson, Menu::JsonLabelRowsCount, &RowsCount, generalconfig.rowscount);
        GetColorOrDefault(menuJson, Menu::JsonLabelBackgroundColor, &BackgroundColor, generalconfig.backgroundColour);
        GetColorOrDefault(menuJson, Menu::JsonLabelOutline, &_outline, generalconfig.DefaultOutline);
        GetColorOrDefault(menuJson, Menu::JsonLabelTextColor, &_textColor, generalconfig.DefaultTextColor);

        LOC_LOGD(module, "Menu is a %dx%d matrix", RowsCount, ColsCount);
        // now that the rows and cols count are known, we can calculate the width of buttons
        SetButtonWidth();
        GetValueOrDefault(menuJson, Menu::JsonLabelType, &value, enum_to_string(MenuTypes::STANDARD));
        if (value)
        {
            LOC_LOGD(module, "Parsing menu type %s", value);
            parse(value, &Type);
            if (Type == MenuTypes::NONE)
            {
                drawErrorMessage(true, module, "Invalid menu type %s for menu %s.", value, STRING_OR_DEFAULT(Name, "unknown"));
                Type = MenuTypes::STANDARD;
            }
            FREE_AND_NULL(value);
        }

        if (Type == MenuTypes::HOME || Type == MenuTypes::HOMESYSTEM)
        {
            GetValueOrDefault(menuJson, Menu::JsonLabelIcon, &Icon, "");
            LOC_LOGD(module, "Icon is %s", STRING_OR_DEFAULT(Icon,""));
        }
        cJSON *buttonsJson = cJSON_GetObjectItem(menuJson, Menu::JsonLabelButtons);
        if (buttonsJson)
        {
            if (!cJSON_IsArray(buttonsJson))
            {
                drawErrorMessage(true, module, "Buttons under menu %s should be an array.", STRING_OR_DEFAULT(Name, "unknown"));
                return;
            }
            else
            {
                LOC_LOGD(module, "Menu has %d buttons to create", cJSON_GetArraySize(buttonsJson));
            }
            cJSON *buttonJson = NULL;
            cJSON_ArrayForEach(buttonJson, buttonsJson)
            {
                auto button = new FTButton(buttonJson, BackgroundColor, _outline, _textColor);
                if (button->ButtonType != ButtonTypes::NONE)
                {
                    button->SetCoordinates(ButtonWidth, ButtonHeight, (uint16_t)(buttons.size() / ColsCount), (uint16_t)(buttons.size() % ColsCount), Spacing);
                    buttons.push_back(button);
                }
                else
                {
                    LOC_LOGE(module, "Invalid button type.");
                    delete (button);
                }
            }
        }
        else
        {
            LOC_LOGD(module, "No buttons were found ");
        }
        cJSON *actionsJson = cJSON_GetObjectItem(menuJson, Menu::JsonLabelActions);
        if (actionsJson)
        {
            if (!cJSON_IsArray(actionsJson))
            {
                drawErrorMessage(true, module, "Actions under menu %s should be an array.", STRING_OR_DEFAULT(Name, "unknown"));
                return;
            }
            cJSON *actionJson = NULL;
            cJSON_ArrayForEach(actionJson, actionsJson)
            {
                ActionsSequences sequence;
                if (!sequence.Parse(actionJson))
                {
                    LOC_LOGE(module, "Could not parse action %s", cJSON_IsString(actionJson) ? STRING_OR_DEFAULT(cJSON_GetStringValue(actionJson), "") : "");
                }
                else
                {
                    Actions.push_back(sequence);
                }
            }
        }
    }
    bool parse(const char *value, MenuTypes *result)
    {
        MenuTypes outType = MenuTypes::NONE;
        bool ret = false;
        do
        {
            outType++; // Start after NONE
            LOC_LOGV(module, "%s =? MenuTypes[%d] (%s)", value, (int)(outType), enum_to_string(outType));
        } while (strcmp(value, enum_to_string(outType)) != 0 && outType != MenuTypes::NONE);
        if (strcmp(value, enum_to_string(outType)) == 0)
        {
            LOC_LOGV(module, "Found: %s==%s ", value, enum_to_string(outType));
            ret = true;
        }
        *result = outType;
        return ret;
    }
    const char *enum_to_string(MenuTypes type)
    {
        switch (type)
        {
            ENUM_TO_STRING_HELPER(MenuTypes, NONE);
            ENUM_TO_STRING_HELPER(MenuTypes, STANDARD);
            ENUM_TO_STRING_HELPER(MenuTypes, SYSTEM);
            ENUM_TO_STRING_HELPER(MenuTypes, HOME);
            ENUM_TO_STRING_HELPER(MenuTypes, OLDHOME);
            ENUM_TO_STRING_HELPER(MenuTypes, HOMESYSTEM);
            ENUM_TO_STRING_HELPER(MenuTypes, ROOT);
            ENUM_TO_STRING_HELPER(MenuTypes, EMPTY);
            ENUM_TO_STRING_HELPER(MenuTypes, ENDLIST);
        default:
            return "UNKNOWN";
        }
    }
}