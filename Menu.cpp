#include "Menu.h"
#include <cstdlib>
static const char * module="Menu";
static const char *nameTemplate = "/config/%s.json";
static const char *homeButtonTemplate=R"({"logo0": "home.bmp","button0":{"latch": false,"latchlogo": "","actionarray": [ "12"],"valuearray": [ "homescreen"]   }})";
  // Overloading global new operator
  void* operator new(size_t sz)
  {
    //ESP_LOGD("Menu","operator new : %d",sz);    
    void* m = malloc_fn(sz);
    return m;
  }
namespace FreeTouchDeck
{
    static char configBuffer[3001]={0}; // used to load config files in memory for parsing 
    Menu::Menu(const char *name, const char * config)
    {
        ESP_LOGD(module,"Instantiating menu name %s",name);
        strncpy(Name, name,sizeof(Name));
        LoadConfig(config);
        ESP_LOGD(module,"Done loading config for menu name %s",name);
    }
    Menu::Menu(const char *name)
    {
        char FileName[31]={0};
        ESP_LOGD(module,"Instantiating menu from file name %s",name);
        strncpy(Name, name,sizeof(Name));
        snprintf(FileName,sizeof(FileName), nameTemplate, name);
        ESP_LOGD(module,"menu File name is %s", FileName);
        File configfile = FILESYSTEM.open(FileName, "r");
        if(!configfile)
        {
            drawErrorMessage(true,module,"Could not open file %s ",name);
        }
        PrintMemInfo();
        LoadConfig(&configfile);
        ESP_LOGD(module,"Done loading configuration file %s", FileName);
        configfile.close();
    }
    Menu::Menu(File *config)
    {
        ESP_LOGD(module,"Instantiating menu from file  %s",config->name());
        String fullName = config->name();
        int start = fullName.lastIndexOf("/") + 1;
        int end = fullName.lastIndexOf(".");
        if (end > start)
        {
            fullName = fullName.substring(start, end);
        }
        strncpy(Name,fullName.c_str(),sizeof(Name));
        ESP_LOGD(module,"Menu name is %s, file name is %s",Name, config->name());
        PrintMemInfo();
        LoadConfig(config);
        ESP_LOGD(module,"DONE Instantiating menu from file  %s",config->name());
        PrintMemInfo();

    }

    void Menu::Init()
    {
        uint32_t TotalHeight = 0;
        MenuRow * newRow = new MenuRow();
        if(!newRow)
        {
            drawErrorMessage(true,module,"Unable to allocate new row!");
        }
        ESP_LOGD(module,"Start of buttons init loop for %d buttons.",ButtonsCount);
        if(ButtonsCount==0)
        {
            ESP_LOGW(module,"No buttons in this screen");
            _rows.push_back( newRow);
            return;
        }
        for(uint8_t b=0;b<ButtonsCount ;b++)        
        {
            if(buttons[b]==NULL)
            {
                ESP_LOGD(module,"Button #%d is null",b);
                continue;
            }
            ESP_LOGD(module,"button %d is %s .",b,buttons[b]->Logo()->LogoName);
            ESP_LOGD(module,"Processing button width= %d. Total row width is %d, LCD width is %d ", buttons[b]->Width(),  newRow->TotalWidth, tft.width());
            if (newRow->TotalWidth + buttons[b]->Width() > tft.width())
            {
                ESP_LOGD(module,"Overflowing LCD Width %d", tft.width());
                TotalHeight += newRow->Height;
                ESP_LOGD(module,"Row has %d buttons, spacing = %d",newRow->Count, newRow->Spacing );
                ESP_LOGD(module,"Row has %d buttons. Pushing to rows list",newRow->Count );
                _rows.push_back( newRow);
                ESP_LOGD(module,"row was added successfully. Creating new row");
                newRow = new MenuRow();
                if(!newRow)
                {
                    drawErrorMessage(true,module,"Unable to allocate new row!");
                }                
            }
            newRow->Count++;
            newRow->Height = max(newRow->Height, buttons[b]->Height());
            newRow->ButtonCenterHeight = newRow->Height/2;
            newRow->ButtonCenterWidth = tft.width() / (newRow->Count *2);
            newRow->TotalWidth += buttons[b]->Width();
        }
        ESP_LOGD(module,"Last Row has %d buttons. Pushing to rows list",newRow->Count );
        _rows.push_back( newRow);
        ESP_LOGD(module,"last row was added successfully");
        for(auto row :_rows )
        {
            ESP_LOGD(module,"evaluating row");
            uint16_t wSpacing = (tft.width() - newRow->TotalWidth) / (newRow->Count + 1); // Spread remaining space
            uint16_t hSpacing = (tft.height() -TotalHeight ) / (_rows.size()+1); // Spread remaining space            
            row->Spacing = min(hSpacing,wSpacing)/2;
            
        }
        ESP_LOGD(module,"Menu has %d buttons spread over %d rows",ButtonsCount,_rows.size() );            

    }

    void Menu::Draw(bool force)
    {
        uint32_t width = 0;
        uint32_t height = 0;
        uint8_t buttonIndex = 0;
        uint8_t rowIndex=0;
        for (auto row : _rows)
        {
            for (int col = 0; col < row->Count; col++)
            {
                //ESP_LOGD(module,"Row has %d buttons, height is %d. Column #%d", row->Count, row->Height,col);
                if(col+buttonIndex >= ButtonsCount || !buttons[col + buttonIndex])
                {
                    ESP_LOGW(module,"Button count in rows %d exceeds total button count %d for screen ", col+buttonIndex, ButtonsCount);
                }
                buttons[col + buttonIndex]->Draw( row->ButtonCenterWidth * (col*2+1) ,
                                                row->ButtonCenterHeight * (rowIndex*2+1),
                                                1,force);
            }
            buttonIndex += row->Count;
            rowIndex++;
        }
    }
    Menu::~Menu()
    {
        ESP_LOGD(module,"Freeing memory for menu %s",Name?Name:"UNKNOWN");
        for (int button = 0; button < ButtonsCount; ButtonsCount++)
        {
            if(buttons[button])
            {
                delete(buttons[button]);
            }
        }

        if (Name)
            free(Name);
    }

    void Menu::ReleaseAll()
    {
        if(!Pressed) return;
        Pressed=false;
        ESP_LOGV(module, "Releasing all %d button ",ButtonsCount);
        for(uint8_t b=0;b<ButtonsCount && buttons[b];b++)
        {
            buttons[b]->Release();
        }
    }
    void Menu::Activate()
    {
        if(!Active)
        {
            Active=true;
            ESP_LOGD(module,"Activating menu %s",Name);
            Draw(true); 
        }
    }
    void Menu::Deactivate()
    {
        if(Active)
        {
            ESP_LOGD(module,"Deactivating screen %s",Name);
            tft.fillScreen(generalconfig.backgroundColour);
            Active=false;
            ReleaseAll();
        }
    }
    bool Menu::LoadConfig(File *config)
    {
        ESP_LOGD(module,"Loading config from file. Loading %s in memory.",config->name());
        memset(configBuffer, 0x00, sizeof(configBuffer));
        size_t read_size = config->readBytes(configBuffer, sizeof(configBuffer));
        if(read_size!=config->size())
        {
            drawErrorMessage(true,module,"Could not read config file %s. Read %d/%d bytes",config->name(), config->size(), read_size);
            // the line above will typicaly stop processing, but let's add a return here just in case
            return false;
        }
        LoadConfig(configBuffer);
        ESP_LOGD(module,"Done Loading config from file.");
    }
    void Menu::AddHomeButton()
    {
        cJSON * menubutton = cJSON_Parse(homeButtonTemplate);
        if(!menubutton)
        {
            const char * error=cJSON_GetErrorPtr();
            drawErrorMessage(true,module,"Unable to parse json string for home menu button: %s",error);
            return;
        }
        ESP_LOGD(module,"Adding home button to screen %s",Name);
        FTButton * newButton = new FTButton( 0, menubutton,cJSON_GetObjectItem(menubutton,"button0"),  _outline, _textSize, _textColor);
        if(!newButton)
        {
            drawErrorMessage(true,module,"Failed to allocate memory for new button");
        }
        buttons[ButtonsCount++]=newButton;
    }

    bool Menu::LoadConfig(const char * config)
    {
        char logoName[31] = {0};
        char buttonName[31] = {0};
        memset(buttons,0x00,sizeof(buttons));
        ButtonsCount=0;
        homeMenu=new FTAction(ActionTypes::MENU,"homescreen");
        if(!homeMenu)
        {
            drawErrorMessage(true,module,"Failed to allocate memory for home menu action");
        }
        ESP_LOGD(module,"Parsing json configuration");
        ESP_LOGV(module,"%s", config);
        cJSON * doc = cJSON_Parse(config);
        if (!doc)
        {
            const char * error=cJSON_GetErrorPtr();
            drawErrorMessage(true,module,"Unable to parse json string : %s",error);
            return false;
        }
        else
        {
            ESP_LOGV(module,"Parsing success. Processing entries");

            cJSON * jsonButton = NULL;
            cJSON * jsonLogo = NULL;
            cJSON * jsonLatch=NULL;
            cJSON * jsonLatchedLogo=NULL;

            do
            {
                snprintf(buttonName, sizeof(buttonName), "button%d", ButtonsCount);
                snprintf(logoName, sizeof(logoName), "logo%d", ButtonsCount);
                jsonLogo=cJSON_GetObjectItem(doc,logoName);
                jsonButton=cJSON_GetObjectItem(doc,buttonName);
                if(jsonButton)
                {
                    ESP_LOGD(module,"Found button %s", buttonName);
                    FTButton * newButton = new FTButton(ButtonsCount,doc,jsonButton, _outline, _textSize, _textColor);
                    if(!newButton)
                    {
                        drawErrorMessage(true,module,"Failed to allocate memory for new button");
                    }                                              
                    buttons[ButtonsCount++]=newButton;
                }
                else if(jsonLogo) 
                {
                    // Most likely processing a stand alone menu logo. 
                    // Add button with corresponding action
                    
                        FTButton *  newButton = new FTButton( ButtonsCount, jsonLogo, _outline, _textSize, _textColor);
                        if(!newButton)
                        {
                            drawErrorMessage(true,module,"Failed to allocate memory for new button");
                        }
                        buttons[ButtonsCount++]=newButton;
                }
                
                /* code */
            } while (jsonButton || jsonLogo);
            
            ESP_LOGD(module,"Done processing json structure, with %d buttons",ButtonsCount);

            if(strcmp(Name, "homescreen")!=0 && ButtonsCount>0)
            {
                AddHomeButton();
            }
            else if(ButtonsCount==0)
            {
                ESP_LOGD(module,"No buttons were found on screen %s. Adding default action as home screen", Name);
                actions.push_back(homeMenu);
            }
        }
        return true;
    }
    void Menu::Touch(uint16_t x, uint16_t y)
    {
        Pressed=true;
        for(uint8_t b = 0;b<ButtonsCount && buttons[b];b++)
        {
            if(buttons[b]->contains(x,y))
            {
                buttons[b]->Press();
            }
        }
        
        if(ButtonsCount ==0)
        {
            if(actions.size()>0)
            {
                // Some empty screens might be defined with a default
                // action. e.g. go back to previous menu, home screen, etc.
                for(auto action : actions)
                {
                    QueueAction(action);
                }
            }
            else if(strcmp("config",Name))
            {
                // in config mode, default to going back to home screen
                // when pressed
                QueueAction(homeMenu);
            }
        }

    }
    void Menu::SetMargin(uint16_t value)
    {
        _margin = value;
    }
}