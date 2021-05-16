#include "Menu.h"
#include <cstdlib>
static const char * module="Menu";
static const char *nameTemplate = "/config/%s.json";
  // Overloading CLass specific new operator
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
        Name = ps_strdup(name);
        FileName = ps_strdup("");
        PrintMemInfo();
        LoadConfig(config);
        ESP_LOGD(module,"Done loading config for menu name %s",name);
        PrintMemInfo();
        Init();
        ESP_LOGD(module,"DONE Instantiating menu name %s",name);
        PrintMemInfo();
    }
    Menu::Menu(const char *name)
    {
        ESP_LOGD(module,"Instantiating menu from file name %s",name);
        Name = ps_strdup(name);
        SetFileName();
        if(!FileName)
        {
            drawErrorMessage(true,module,"Unable to assign file name for menu %s",name);
            return; 
        }
        ESP_LOGD(module,"menu File name is %s", FileName);
        File configfile = FILESYSTEM.open(FileName, "r");
        PrintMemInfo();
        LoadConfig(&configfile);
        ESP_LOGD(module,"Done loading configuration file %s", FileName);
        configfile.close();
        PrintMemInfo();
        Init();
        ESP_LOGD(module,"Done loading instantiating menu file %s", FileName);
        PrintMemInfo();
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
        Name = ps_strdup(fullName.c_str());
        FileName = ps_strdup(config->name());
        ESP_LOGD(module,"Menu name is %s, file name is %s",Name, FileName);
        PrintMemInfo();
        LoadConfig(config);
        ESP_LOGD(module,"Done loading config from file %s",FileName);
        PrintMemInfo();
        Init();
        ESP_LOGD(module,"DONE Instantiating menu from file  %s",FileName);
        PrintMemInfo();

    }

    void Menu::Init()
    {
        uint32_t TotalHeight = 0;
        auto newRow = new row_t();
        FTButton * button=NULL;
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
        for(int i=0;i<ButtonsCount && buttons[i];i++)
        {
            ESP_LOGD(module,"Button index: %s",i);
            button=buttons[i];
            ESP_LOGD(module,"Processing button width= %d. Total row width is %d, LCD width is %d ", button->Width(),  newRow->TotalWidth, tft.width());
            yield();            
            if (newRow->TotalWidth + button->Width() > tft.width())
            {
                ESP_LOGD(module,"Overflowing LCD Width %d", tft.width());
                TotalHeight += newRow->Height;
                ESP_LOGD(module,"Row has %d buttons, spacing = %d",newRow->Count, newRow->Spacing );
                ESP_LOGD(module,"Row has %d buttons. Pushing to rows list",newRow->Count );
                _rows.push_back( newRow);
                ESP_LOGD(module,"row was added successfully. Creating new row");
                newRow = new row_t();
                if(!newRow)
                {
                    drawErrorMessage(true,module,"Unable to allocate new row!");
                }                
            }
            newRow->Count++;
            newRow->Height = max(newRow->Height, button->Height());
            newRow->ButtonCenterWidth = tft.width() / (newRow->Count *2);
            newRow->TotalWidth += button->Width();
        }
        ESP_LOGD(module,"Last Row has %d buttons. Pushing to rows list",newRow->Count );
        _rows.push_back( newRow);
        ESP_LOGD(module,"last row was added successfully");
        for(int z=0;z<_rows.size();z++)
        { 
            auto row=_rows[z];
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
        for (int row = 0; row < _rows.size(); row++)
        {
            for (int col = 0; col < _rows[row]->Count; col++)
            {
                if(col+buttonIndex >= ButtonsCount || !buttons[col + buttonIndex])
                {
                    ESP_LOGW(module,"Button count in rows %d exceeds total button count %d for screen ", col+buttonIndex, ButtonsCount);
                }
                buttons[col + buttonIndex]->Draw( _rows[row]->ButtonCenterWidth * (col*2+1) ,
                                                _rows[row]->ButtonCenterHeight * (row*2+1),
                                                1,force);
                                                //_rows[row]->Spacing);
            }
            buttonIndex += _rows[row]->Count;
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
        if (FileName)
            free(FileName);
        if (Name)
            free(Name);
    }

    void Menu::SetFileName()
    {
        size_t nameSize = printf(NULL, nameTemplate, Name);
        FileName = (char *)malloc(nameSize + 1);
        memset(FileName, 0x00, nameSize + 1);
        printf(FileName, nameTemplate, Name);
    }
    void Menu::ReleaseAll()
    {
        for(uint8_t b=0;b<ButtonsCount && buttons[b];b++)
        {
            buttons[b]->Release();
        }
    }
    #define CJSON_STRING_OR_DEFAULT(target,json,default) \
        if(json && cJSON_IsString(json)) \
        { \
            target=cJSON_GetStringValue(json); \
        } \
        else \
        { \
            target=default; \
        } 
    void Menu::Activate()
    {
        if(!Active)
        {
            Active=true;
            ESP_LOGD(module,"Activating menu %s",Name);
            for(int i=0;i<ButtonsCount && buttons[i];i++)
            {

                buttons[i]->Invalidate();
            }
            ESP_LOGD(module,"Drawing Menu");
            Draw(); 
        }
    }
    void Menu::Deactivate()
    {
        if(Active)
        {
            tft.fillScreen(generalconfig.backgroundColour);
            Active=false;
            ReleaseAll();
        }
    }
    bool Menu::LoadConfig(File *config)
    {
        ESP_LOGD(module,"Loading config from file. Loading %s in memory.",config->name());
//        char * configBuffer = (char *)malloc_fn(config->size()+1) ;
        // if(!configBuffer)
        // {
        //     drawErrorMessage(true,module,"Memory allocation failed ");
        // }
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
    void Menu::AddHomeButton(uint8_t * position)
    {
        FTButton * newButton = new FTButton( "", *position++, ButtonTypes::MENU, "home.bmp", "question.bmp", _outline, _textSize, _textColor);
        if(!newButton)
        {
            drawErrorMessage(true,module,"Failed to allocate memory for new button");
        }
        newButton->actions.push_back( homeMenu);
        buttons[ButtonsCount++]=newButton;
    }

    bool Menu::LoadConfig(const char * config)
    {
        ESP_LOGD(module,"Parsing json configuration: \n%s", config);
        homeMenu=new FTAction(ActionTypes::MENU,"homescreen");
        if(!homeMenu)
        {
            drawErrorMessage(true,module,"Failed to allocate memory for home menu action");
        }
        cJSON * doc = cJSON_Parse(config);
        if (!doc)
        {
            const char * error=cJSON_GetErrorPtr();
            drawErrorMessage(true,module,"Unable to parse json string : %s",error);
            return false;
        }
        else
        {
            ESP_LOGD(module,"Parsing success. Processing entries");
            // char * jsonstr=cJSON_Print(doc);
            // if(jsonstr)
            // {
            //     ESP_LOGD(module,"Parsed json: \n%s",jsonstr);
            //     free(jsonstr);
            // }
            char logoName[31] = {0};
            char buttonName[31] = {0};
            uint8_t position = 0;
            cJSON * jsonButton = NULL;
            cJSON * jsonLogo = NULL;
            cJSON * jsonLatch=NULL;
            cJSON * jsonLatchedLogo=NULL;
            cJSON * jsonAction=NULL;
            cJSON * jsonActions=NULL;
            cJSON * jsonActionValue=NULL;
            const char * logoValue=NULL;
            const char * latchLogoValue=NULL;
            do
            {
                snprintf(logoName, sizeof(logoName), "logo%d", position);
                snprintf(buttonName, sizeof(buttonName), "button%d", position);
                jsonButton=cJSON_GetObjectItem(doc,buttonName);
                if(jsonButton)
                {
                    ESP_LOGD(module,"Found button %s", buttonName);
                    CJSON_STRING_OR_DEFAULT(logoValue,cJSON_GetObjectItem(doc,logoName),"question.bmp" );
                    CJSON_STRING_OR_DEFAULT(latchLogoValue,cJSON_GetObjectItem(jsonButton,logoName),"" );
                    jsonLatchedLogo=cJSON_GetObjectItem(jsonButton,"latchlogo");
                    jsonLatch=cJSON_GetObjectItem(jsonButton, "latch");
                    ButtonTypes Latched=(jsonLatch && cJSON_IsBool(jsonLatch) && cJSON_IsTrue(jsonLatch))?ButtonTypes::LATCH : ButtonTypes::STANDARD;

                    FTButton * newButton = new FTButton( "", position,
                                              Latched ,
                                              logoValue,
                                              latchLogoValue, _outline, _textSize, _textColor);
                    if(!newButton)
                    {
                        drawErrorMessage(true,module,"Failed to allocate memory for new button");
                    }                                              
                    jsonActions =  cJSON_GetObjectItem(jsonButton, "actionarray");
                    if(!jsonActions)
                    {
                        Serial.println("[ERROR]: Button does not have any action");
                        return false;
                    }

                    uint8_t valuePos = 0;
                    cJSON_ArrayForEach(jsonActionValue,cJSON_GetObjectItem(jsonButton,"valuearray"))
                    {
                        jsonAction=cJSON_GetArrayItem(jsonActions,valuePos++);
                        if(!jsonAction)
                        {
                            ESP_LOGE(module,"Current value does not have a matching action!");
                            return false;
                        }
                        if(FTAction::GetType(jsonAction)!= ActionTypes::NONE)
                        {
                            // only push valid actions
                            newButton->actions.push_back(new FTAction(jsonAction,jsonActionValue));
                        }
                    }
                    buttons[ButtonsCount++]=newButton;
                    position++;
                }
                
                /* code */
            } while (jsonButton);
            
            ESP_LOGD(module,"Done processing json structure, with %d buttons",ButtonsCount);
            if (ButtonsCount==0)
            {
                // this only contains menus
                position = 0;
                char menuName[31] = {0};
                ESP_LOGD(module,"No buttons were found.  Trying to load as menu");
                do
                {
                    snprintf(logoName, sizeof(logoName), "logo%d", position);
                    snprintf(menuName, sizeof(menuName), "menu%d", position+1);
                    jsonLogo=cJSON_GetObjectItem(doc,logoName);
                    if(jsonLogo)
                    {
                        CJSON_STRING_OR_DEFAULT(logoValue,cJSON_GetObjectItem(doc,logoName),"question.bmp" );
                        FTButton *  newButton = new FTButton( "", position, ButtonTypes::MENU,logoValue, "question.bmp", _outline, _textSize, _textColor);
                        if(!newButton)
                        {
                            drawErrorMessage(true,module,"Failed to allocate memory for new button");
                        }
                        newButton->actions.push_back(new FTAction(ActionTypes::MENU,menuName));
                        buttons[ButtonsCount++]=newButton;
                    }   
                    position++;     

                } while (jsonLogo);
            }
            if(strcmp(Name, "homescreen")!=0 && ButtonsCount>0)
            {
                AddHomeButton(&position);
            }
            else if(ButtonsCount==0)
            {
                ESP_LOGD(module,"No buttons were found on screen %s. Adding default action as home screen", Name);
                actions.push_back(homeMenu);
            }
            ESP_LOGD(module,"Freeing up the json structure.");
            cJSON_Delete(doc);
        }
        return true;
    }
    void Menu::Touch(uint16_t x, uint16_t y)
    {
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