#include "Menu.h"
#include <cstdlib>
static const char *module = "Menu";
static const char *nameTemplate = "/config/%s.json";
static const char *homeButtonTemplate = R"({"logo0": "home.bmp","button0":{"latch": false,"latchlogo": "","actionarray": [ "12"],"valuearray": [ "homescreen"]   }})";
static const char *backButtonTemplate = R"({"logo0": "arrow_back.bmp","button0":{"latch": false,"latchlogo": "","actionarray": [ "12"],"valuearray": [ "~BACK"]   }})";
// Overloading global new operator
void *operator new(size_t sz)
{
    //ESP_LOGD("Menu","operator new : %d",sz);
    void *m = malloc_fn(sz);
    return m;
}
namespace FreeTouchDeck
{
    FTAction * Menu::homeMenu = new FTAction(ActionTypes::MENU, "homescreen");
    FTAction * Menu::backButton = new FTAction(ActionTypes::MENU, "~BACK");
    Menu::Menu(const char *name, const char *config)
    {
        ESP_LOGD(module, "Instantiating menu name %s", name);
        strncpy(Name, name, sizeof(Name));
        LoadConfig(config);
        ESP_LOGD(module, "Done loading config for menu name %s", name);
    }
    Menu::Menu(const char *name)
    {
        char FileName[31] = {0};
        ESP_LOGD(module, "Instantiating menu from file name %s", name);
        strncpy(Name, name, sizeof(Name));
        snprintf(FileName, sizeof(FileName), nameTemplate, name);
        ESP_LOGD(module, "menu File name is %s", FileName);
        File configfile = FILESYSTEM.open(FileName, "r");
        if (!configfile)
        {
            drawErrorMessage(true, module, "Could not open file %s ", name);
        }
        PrintMemInfo();
        LoadConfig(&configfile);
        ESP_LOGD(module, "Done loading configuration file %s", FileName);
        configfile.close();
    }
    Menu::Menu(File *config)
    {
        ESP_LOGD(module, "Instantiating menu from file  %s", config->name());
        String fullName = config->name();
        int start = fullName.lastIndexOf("/") + 1;
        int end = fullName.lastIndexOf(".");
        if (end > start)
        {
            fullName = fullName.substring(start, end);
        }
        strncpy(Name, fullName.c_str(), sizeof(Name));
        ESP_LOGD(module, "Menu name is %s, file name is %s", Name, config->name());
        PrintMemInfo();
        LoadConfig(config);
        ESP_LOGD(module, "DONE Instantiating menu from file  %s", config->name());
        PrintMemInfo();
    }

    void Menu::Init()
    {
        uint32_t TotalHeight = 0;
        MenuRow *newRow = new MenuRow();
        if (!newRow)
        {
            drawErrorMessage(true, module, "Unable to allocate new row!");
        }
        ESP_LOGD(module, "Start of buttons init loop for %d buttons.", buttons.size());
        if (buttons.size()==0)
        {
            ESP_LOGW(module, "No buttons in this screen");
            _rows.push_back(newRow);
            return;
        }
        for (FTButton * button : buttons)
        {
            ESP_LOGD(module, "Processing button %s width= %d. Total row width is %d, LCD width is %d ",button->Logo()->LogoName, button->Width(), newRow->TotalWidth, tft.width());
            if (newRow->TotalWidth + button->Width() > tft.width())
            {
                ESP_LOGD(module, "Overflowing LCD Width %d", tft.width());
                TotalHeight += newRow->Height;
                ESP_LOGD(module, "Row has %d buttons, spacing = %d", newRow->buttons.size(), newRow->Spacing);
                _rows.push_back(newRow);
                ESP_LOGD(module, "row was added successfully. Creating new row");
                newRow = new MenuRow();
                if (!newRow)
                {
                    drawErrorMessage(true, module, "Unable to allocate new row!");
                }
            }
            newRow->buttons.push_back(button);
            newRow->Height = max(newRow->Height, button->Height());
            newRow->Width = max(newRow->Width, button->Width());
            newRow->ButtonCenterHeight = newRow->Height / 2;
            newRow->ButtonCenterWidth = tft.width() / (newRow->buttons.size() * 2);
            newRow->TotalWidth += button->Width();
        }
        ESP_LOGD(module, "Last Row has %d buttons. Pushing to rows list", newRow->buttons.size());
        _rows.push_back(newRow);
        ESP_LOGD(module, "last row was added successfully");
        for (auto row : _rows)
        {
            ESP_LOGD(module, "evaluating row");
            uint16_t wSpacing = (tft.width() - newRow->TotalWidth) / (newRow->buttons.size() + 1); // Spread remaining space
            uint16_t hSpacing = (tft.height() - TotalHeight) / (_rows.size() + 1);                 // Spread remaining space
            row->Spacing = min(hSpacing, wSpacing) / 2;
        }
        ESP_LOGD(module, "Menu has %d buttons spread over %d rows", buttons.size(), _rows.size());
    }
    bool Menu::Button(FTAction *action)
    {
        char screenName[51] = {0};
        char buttonName[51] = {0};
        FTButton *button = NULL;
        if (action->GetLatchButton(screenName, sizeof(screenName), buttonName, sizeof(buttonName)) && strcmp(screenName, Name) == 0)
        {
            if ((button = GetButton(buttonName)) != NULL)
            {
                ESP_LOGD(module, "Found button %s to run action %s", buttonName, action->toString());
                return button->Latch(action);
            }
            else
            {
                ESP_LOGE(module, "Could not find button %s to run action %s", buttonName, action->toString());
            }
        }
        return false;
    }
    FTButton *Menu::GetButton(const char *buttonName)
    {
        for (auto button : buttons)
        {
            if(strcmp(button->Label,buttonName)==0)
            {
                return button;
            }
            ESP_LOGD(module,"Ignoring button %s", button->Label);
        }
        return NULL;
    }
    void Menu::Draw(bool force)
    {
        uint8_t rowIndex = 0;
        for (auto row : _rows)
        {
            uint8_t colIndex = 0;
            for (auto button : row->buttons)
            {
                button->Draw(row->ButtonCenterWidth * (colIndex * 2 + 1),
                             row->ButtonCenterHeight * (rowIndex * 2 + 1),
                             row->Width, row->Height,
                             row->Spacing, force);
                colIndex++;
            }
            rowIndex++;
        }
    }
    Menu::~Menu()
    {
        ESP_LOGD(module, "Freeing memory for menu %s", Name ? Name : "UNKNOWN");
        for (auto button : buttons)
        {
            delete (button);
        }

        if (Name)
            free(Name);
    }

    void Menu::ReleaseAll()
    {
        if (!Pressed)
            return;
            
        Pressed = false;
        ESP_LOGV(module, "Releasing all %d button ", buttons.size());
        for (auto button : buttons)
        {
            button->Release();
        }
    }
    void Menu::Activate()
    {
        if (!Active)
        {
            Active = true;
            ESP_LOGD(module, "Activating menu %s", Name);
            Draw(true);
        }
    }
    void Menu::Deactivate()
    {
        if (Active)
        {
            ESP_LOGD(module, "Deactivating screen %s", Name);
            tft.fillScreen(generalconfig.backgroundColour);
            Active = false;
            ReleaseAll();
        }
    }
    bool Menu::LoadConfig(File *config)
    {
        ESP_LOGD(module, "Loading config from file. Loading %s in memory.", config->name());
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
        ESP_LOGD(module, "Done Loading config from file.");
    }
    void Menu::AddHomeButton()
    {
        cJSON *menubutton = cJSON_Parse(homeButtonTemplate);
        if (!menubutton)
        {
            const char *error = cJSON_GetErrorPtr();
            drawErrorMessage(true, module, "Unable to parse json string for home menu button: %s", error);
            return;
        }
        ESP_LOGD(module, "Adding home button to screen %s", Name);
        FTButton *newButton = new FTButton(0, menubutton, cJSON_GetObjectItem(menubutton, "button0"), _outline, _textSize, _textColor);
        if (!newButton)
        {
            drawErrorMessage(true, module, "Failed to allocate memory for new button");
        }
        cJSON_Delete(menubutton);
        buttons.push_back(newButton);
    }
    void Menu::AddBackButton()
    {
        cJSON *menubutton = cJSON_Parse(backButtonTemplate);
        if (!menubutton)
        {
            const char *error = cJSON_GetErrorPtr();
            drawErrorMessage(true, module, "Unable to parse json string for back menu button: %s", error);
            return;
        }
        ESP_LOGD(module, "Adding back button to screen %s", Name);
        FTButton *newButton = new FTButton(0, menubutton, cJSON_GetObjectItem(menubutton, "button0"), _outline, _textSize, _textColor);
        if (!newButton)
        {
            drawErrorMessage(true, module, "Failed to allocate memory for new button");
        }
        cJSON_Delete(menubutton);
        buttons.push_back(newButton);
    }
    bool Menu::LoadConfig(const char *config)
    {
        char logoName[31] = {0};
        char buttonName[31] = {0};

        PrintMemInfo();
        ESP_LOGD(module, "Parsing json configuration");
        ESP_LOGV(module, "%s", config);
        cJSON *doc = cJSON_Parse(config);
        if (!doc)
        {
            const char *error = cJSON_GetErrorPtr();
            drawErrorMessage(true, module, "Unable to parse json string : %s", error);
            return false;
        }
        else
        {
            PrintMemInfo();
            ESP_LOGV(module, "Parsing success. Processing entries");

            cJSON *jsonButton = NULL;
            cJSON *jsonLogo = NULL;
            uint8_t ButtonsCount=0;
            do
            {
                snprintf(buttonName, sizeof(buttonName), "button%d", ButtonsCount);
                snprintf(logoName, sizeof(logoName), "logo%d", ButtonsCount);
                jsonLogo = cJSON_GetObjectItem(doc, logoName);
                jsonButton = cJSON_GetObjectItem(doc, buttonName);
                if (jsonButton)
                {
                    ESP_LOGD(module,"Creating new button");
                    PrintMemInfo();
                    ESP_LOGD(module, "Found button %s", buttonName);
                    FTButton *newButton = new FTButton(ButtonsCount, doc, jsonButton, _outline, _textSize, _textColor);
                    if (!newButton)
                    {
                        drawErrorMessage(true, module, "Failed to allocate memory for new button");
                    }
                    buttons.push_back( newButton);
                    ButtonsCount++;
                    ESP_LOGD(module,"Created new logo button");
                    PrintMemInfo();

                }
                else if (jsonLogo)
                {
                    // Most likely processing a stand alone menu logo.
                    // Add button with corresponding action
                    ESP_LOGD(module,"Creating new logo button");
                    PrintMemInfo();
                    FTButton *newButton = new FTButton(ButtonsCount, jsonLogo, _outline, _textSize, _textColor);
                    if (!newButton)
                    {
                        drawErrorMessage(true, module, "Failed to allocate memory for new button");
                    }
                    buttons.push_back(newButton);
                    ButtonsCount++;
                    ESP_LOGD(module,"Created new logo button");
                    PrintMemInfo();
                }

                /* code */
            } while (jsonButton || jsonLogo);

            ESP_LOGD(module, "Done processing json structure, with %d buttons", ButtonsCount);

            if (strcmp(Name, "homescreen") != 0 && buttons.size() > 0)
            {
                AddBackButton();
            }
            else if (buttons.size() == 0)
            {
                ESP_LOGD(module, "No buttons were found on screen %s. Adding default action as back to prev screen", Name);
                actions.push_back(Menu::backButton);
            }
            cJSON_Delete(doc);
            ESP_LOGD(module,"Deleted json document ");
            PrintMemInfo();

        }
        return true;
    }
    void Menu::Touch(uint16_t x, uint16_t y)
    {
        Pressed = true;
        for (auto button : buttons)
        {
            if (button->contains(x, y))
            {
                button->Press();
            }
        }

        if (buttons.size() == 0)
        {
            if (actions.size() > 0)
            {
                // Some empty screens might be defined with a default
                // action. e.g. go back to previous menu, home screen, etc.
                for (auto action : actions)
                {
                    QueueAction(action);
                }
            }
            else if (strcmp("config", Name))
            {
                // in config mode, default to going back to home screen
                // when pressed
                QueueAction(Menu::homeMenu);
            }
        }
    }
    void Menu::SetMargin(uint16_t value)
    {
        _margin = value;
    }
}