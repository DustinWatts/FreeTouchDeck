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

    FTAction *Menu::homeMenu = new FTAction(ActionTypes::MENU, "home");
    Menu::Menu(const char *name, const char *config)
    {
        LOC_LOGD(module, "Instantiating menu name %s", name);
        Name = ps_strdup(STRING_OR_DEFAULT(name, ""));
        LoadConfig(config);
        LOC_LOGD(module, "Done loading config for menu name %s", name);
    }
    Menu::Menu(const char *name)
    {
        char FileName[31] = {0};
        LOC_LOGD(module, "Instantiating menu from file name %s", name);
        Name = ps_strdup(STRING_OR_DEFAULT(name, ""));
        snprintf(FileName, sizeof(FileName), nameTemplate, name);
        LOC_LOGD(module, "menu File name is %s", FileName);
        File configfile = FILESYSTEM.open(FileName, "r");
        if (!configfile)
        {
            drawErrorMessage(true, module, "Could not open file %s ", name);
        }
        PrintMemInfo();
        LoadConfig(&configfile);
        LOC_LOGD(module, "Done loading configuration file %s", FileName);
        configfile.close();
    }
    Menu::Menu(File *config)
    {
        LOC_LOGD(module, "Instantiating menu from file  %s", config->name());
        String fullName = config->name();
        int start = fullName.lastIndexOf("/") + 1;
        int end = fullName.lastIndexOf(".");
        if (end > start)
        {
            fullName = fullName.substring(start, end);
        }
        Name = ps_strdup(STRING_OR_DEFAULT(fullName.c_str(), ""));
        LOC_LOGD(module, "Menu name is %s, file name is %s", Name, config->name());
        PrintMemInfo();
        LoadConfig(config);
        LOC_LOGD(module, "DONE Instantiating menu from file  %s", config->name());
        PrintMemInfo();
    }

    bool Menu::Button(FTAction *action)
    {
        char screenName[51] = {0};
        char buttonName[51] = {0};
        bool success = false;
        FTButton *button = NULL;
        if (action->SplitActionParameter(screenName, sizeof(screenName), buttonName, sizeof(buttonName)) && strcmp(screenName, Name) == 0)
        {
            if ((button = GetButton(buttonName)) != NULL)
            {
                LOC_LOGD(module, "Found button %s to run action %s", buttonName, action->toString());
                success = button->Latch(action);
                if (!success)
                {
                    LOC_LOGE(module, "Running %s failed", action->toString());
                }
            }
            else
            {
                LOC_LOGE(module, "Could not find button %s to run action %s", buttonName, action->toString());
            }
        }
        else
        {
            LOC_LOGE(module, "Button name was not found.  Action string is %s", action->symbol);
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
            LOC_LOGD(module, "Ignoring button %s", button->Label);
        }
        return NULL;
    }
    FTButton *Menu::GetButtonForMenuName(const char *menuName)
    {
        for (auto button : buttons)
        {
            LOC_LOGV(module, "Checking button %s", button->Label);
            for (auto a : button->actions)
            {
                if (strcmp(a->symbol, menuName) == 0)
                {
                    return button;
                }
                else
                {
                    LOC_LOGV(module, "Ignoring action %s", a->toString());
                }
            }
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
            FTButton::BackButton.Draw(force);
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
            FTButton::BackButton.Release();
        }
    }
    void Menu::Activate()
    {
        if (!Active)
        {
            Active = true;
            LOC_LOGD(module, "Activating menu %s", Name);
            if (HasBackButton())
            {
                FTButton::BackButton.SetCoordinates(ButtonWidth, ButtonHeight, (uint16_t)(buttons.size() / ColsCount), (uint16_t)(buttons.size() % ColsCount), Spacing);
            }
            Draw(true);
        }
    }
    void Menu::Deactivate()
    {
        if (Active)
        {
            LOC_LOGD(module, "Deactivating screen %s", Name);
            tft.fillScreen(generalconfig.backgroundColour);
            Active = false;
            ReleaseAll();
        }
    }
    bool Menu::LoadConfig(File *config)
    {
        LOC_LOGD(module, "Loading config from file. Loading %s in memory.", config->name());
        char configBuffer[3001] = {0}; // used to load config files in memory for parsing
        memset(configBuffer, 0x00, sizeof(configBuffer));
        size_t read_size = config->readBytes(configBuffer, sizeof(configBuffer));
        if (read_size != config->size())
        {
            drawErrorMessage(true, module, "Could not read config file %s. Read %d/%d bytes", config->name(), config->size(), read_size);
            // the line above will typicaly stop processing, but let's add a return here just in case
            return false;
        }
        LoadConfig(configBuffer);
        LOC_LOGD(module, "Done Loading config from file.");
    }
    // void Menu::AddHomeButton()
    // {
    //     cJSON *menubutton = cJSON_Parse(homeButtonTemplate);
    //     if (!menubutton)
    //     {
    //         const char *error = cJSON_GetErrorPtr();
    //         drawErrorMessage(true, module, "Unable to parse json string for home menu button: %s", error);
    //         return;
    //     }
    //     LOC_LOGD(module, "Adding home button to screen %s", Name);
    //     FTButton *newButton = new FTButton("home", (uint8_t)0, menubutton, cJSON_GetObjectItem(menubutton, "button0"), _outline, _textSize, _textColor);
    //     if (!newButton)
    //     {
    //         drawErrorMessage(true, module, "Failed to allocate memory for new button");
    //     }
    //     newButton->SetCoordinates(ButtonWidth, ButtonHeight, (uint16_t)(buttons.size() / ColsCount), (uint16_t)(buttons.size() % ColsCount), Spacing);
    //     cJSON_Delete(menubutton);
    //     buttons.push_back(newButton);
    // }
    // void Menu::AddBackButton()
    // {
    //     cJSON *menubutton = cJSON_Parse(backButtonTemplate);
    //     if (!menubutton)
    //     {
    //         const char *error = cJSON_GetErrorPtr();
    //         drawErrorMessage(true, module, "Unable to parse json string for back menu button: %s", error);
    //         return;
    //     }
    //     LOC_LOGD(module, "Adding back button to screen %s", Name);
    //     FTButton *newButton = new FTButton(menubutton);
    //     if (!newButton)
    //     {
    //         drawErrorMessage(true, module, "Failed to allocate memory for new button");
    //     }
    //     newButton->SetCoordinates(ButtonWidth, ButtonHeight, (uint16_t)(buttons.size() / ColsCount), (uint16_t)(buttons.size() % ColsCount), Spacing);
    //     cJSON_Delete(menubutton);
    //     buttons.push_back(newButton);
    // }
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
        ButtonWidth = (tft.width() - max(ColsCount - 1,0) * Spacing) / ColsCount;
        ButtonHeight = (tft.height() - max(RowsCount - 1,0) * Spacing) / RowsCount;
    }
    bool Menu::LoadConfig(const char *config)
    {
        char logoName[31] = {0};
        char buttonName[31] = {0};
        uint16_t row;
        uint16_t col;
        SetButtonWidth();
        LOC_LOGD(module, "Parsing json configuration");
        LOC_LOGV(module, "%s", config);
        cJSON *doc = cJSON_Parse(config);
        if (!doc)
        {
            const char *error = cJSON_GetErrorPtr();
            drawErrorMessage(true, module, "Unable to parse json string : %s", error);
            return false;
        }
        else
        {
            LOC_LOGV(module, "Parsing success. Processing entries");

            cJSON *jsonButton = NULL;
            cJSON *jsonLogo = NULL;
            uint8_t ButtonsCount = 0;
            do
            {
                snprintf(buttonName, sizeof(buttonName), "button%d", ButtonsCount);
                snprintf(logoName, sizeof(logoName), "logo%d", ButtonsCount);
                jsonLogo = cJSON_GetObjectItem(doc, logoName);
                jsonButton = cJSON_GetObjectItem(doc, buttonName);
                if (jsonButton)
                {
                    // when importing older menu structures, assume all
                    // menus need to show on the home screen
                    Type = MenuTypes::HOME;
                    LOC_LOGD(module, "Found button index %d (%s), size %dx%d, row %d, col %d ", ButtonsCount, buttonName, ButtonWidth, ButtonHeight, (uint16_t)(buttons.size() / ColsCount), (uint16_t)(buttons.size() % ColsCount));
                    LOC_LOGD(module, "Creating new instance");
                    FTButton *newButton = new FTButton("", ButtonsCount, doc, jsonButton, _outline, _textSize, _textColor);
                    if (!newButton)
                    {
                        drawErrorMessage(true, module, "Failed to allocate memory for new button");
                    }
                    newButton->SetCoordinates(ButtonWidth, ButtonHeight, (uint16_t)(buttons.size() / ColsCount), (uint16_t)(buttons.size() % ColsCount), Spacing);
                    buttons.push_back(newButton);
                    ButtonsCount++;
                    LOC_LOGD(module, "Created new logo button");
                }
                else if (jsonLogo)
                {

                    Type = MenuTypes::OLDHOME;
                    // Most likely processing a stand alone menu logo.
                    // Add button with corresponding action
                    LOC_LOGD(module, "Creating new logo button, size %dx%d, row %d, col %d  ", ButtonWidth, ButtonHeight, (uint16_t)(buttons.size() / ColsCount), (uint16_t)(buttons.size() % ColsCount));

                    FTButton *newButton = new FTButton("", ButtonsCount, jsonLogo, _outline, _textSize, _textColor);
                    if (!newButton)
                    {
                        drawErrorMessage(true, module, "Failed to allocate memory for new button");
                    }
                    newButton->SetCoordinates(ButtonWidth, ButtonHeight, (uint16_t)(buttons.size() / ColsCount), (uint16_t)(buttons.size() % ColsCount), Spacing);
                    buttons.push_back(newButton);
                    ButtonsCount++;
                    LOC_LOGD(module, "Created new logo button");
                }
                PrintMemInfo();
                /* code */
            } while (jsonButton || jsonLogo);

            LOC_LOGD(module, "Done processing json structure, with %d buttons", ButtonsCount);

            // if (Type!=MenuTypes::OLDHOME && buttons.size() > 0)
            // {
            //     AddBackButton();
            // }
            // else if (buttons.size() == 0)
            // {
            //     LOC_LOGD(module, "No buttons were found on screen %s. Adding default action as back to prev screen", Name);
            //     actions.push_back(Menu::backButton);
            // }
            cJSON_Delete(doc);
            PrintMemInfo();
        }
        return true;
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
            if (FTButton::BackButton.contains(x, y))
            {
                foundPressedButton = true;
                FTButton::BackButton.Press();
            }
        }
        if (!foundPressedButton && buttons.size() == 0)
        {
            if (actions.size() > 0)
            {
                // Some empty screens might be defined with a default
                // action. e.g. go back to previous menu, home screen, etc.
                for (auto action : actions)
                {
                    QueueAction(action);
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
    const char *Menu::JsonLabelRowsCount = "rowscount";
    const char *Menu::JsonLabelColsCount = "colscount";
    const char *Menu::JsonLabelButtons = "buttons";
    const char *Menu::JsonLabelActions = "actions";
    const char *Menu::JsonLabelType = "type";
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

        if (actions.size() > 0)
        {
            LOC_LOGD(module, "Adding %d actions to Json", actions.size());
            cJSON *actionsJson = cJSON_CreateArray();
            if (!actionsJson)
            {
                drawErrorMessage(true, module, "Unable to allocate memory for actions");
            }
            for (auto action : actions)
            {
                if (action->Type != ActionTypes::NONE)
                {
                    cJSON *actionJson = action->ToJson();
                    if (actionJson)
                    {
                        cJSON_AddItemToArray(actionsJson, actionJson);
                    }
                    else
                    {
                        drawErrorMessage(true, module, "Could not retrieve json representation of action");
                    }
                }
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
    Menu::Menu(cJSON *menuJson)
    {
        char *value = NULL;
        GetValueOrDefault(menuJson, Menu::JsonLabelName, &Name, "");
        LOC_LOGD(module, "Label name: %s", Name);
        GetValueOrDefault(menuJson, Menu::JsonLabelColsCount, &ColsCount, generalconfig.colscount);
        GetValueOrDefault(menuJson, Menu::JsonLabelRowsCount, &RowsCount, generalconfig.rowscount);
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
            GetValueOrDefault(menuJson, Menu::JsonLabelIcon, &Icon, "question.bmp");
            LOC_LOGD(module, "Icon is %s", Icon);
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
                auto button = new FTButton(buttonJson);
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
                auto action = new FTAction(actionJson);
                if (action->Type != ActionTypes::NONE)
                {
                    actions.push_back(action);
                }
            }
        }
        // if(Type!=MenuTypes::SYSTEM  && buttons.size()>0)
        // {
        //     AddBackButton();
        // }
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