#include "Menu.h"

namespace FreeTouchDeck
{
    Menu::Menu(const char *name, const char * config, TFT_eSPI &gfx): GFX(gfx)
    {
        Serial.printf("Instantiating menu name %s\n",name);
        Name = strdup(name);
        FileName = strdup("");
        LoadConfig(config);
        Init(gfx);
        Serial.printf("DONE Instantiating menu name %s\n",name);
    }
    Menu::Menu(const char *name, TFT_eSPI &gfx): GFX(gfx)
    {
        Serial.printf("Instantiating menu from file name %s\n",name);
        Name = strdup(name);
        SetFileName();
        File configfile = FILESYSTEM.open(FileName, "r");
        LoadConfig(configfile);
        Init(gfx);
        Serial.printf("DONE Instantiating menu from file name %s\n",name);

    }
    Menu::Menu(File &config, TFT_eSPI &gfx) : GFX(gfx)
    {
        Serial.printf("Instantiating menu from file  %s\n",config.name());
        String fullName = config.name();
        int start = fullName.lastIndexOf("/") + 1;
        int end = fullName.lastIndexOf(".");
        if (end > start)
        {
            fullName = fullName.substring(start, end);
        }
        Name = strdup(fullName.c_str());
        FileName = strdup(config.name());
        LoadConfig(config);
        Init(gfx);
        Serial.printf("DONE Instantiating menu from file  %s\n",config.name());
    }
    void Menu::Init(TFT_eSPI &gfx)
    {
        auto newRow = new row_t();
        uint32_t TotalHeight = 0;
        for (auto button : buttons)
        {
            // Serial.print("Processing button ");
            // Serial.printf("width= %d. ", button->Width());
            // Serial.printf("Total row width is %d, LCD width is %d\r",  newRow->TotalWidth, GFX.width());
            if (newRow->TotalWidth + button->Width() > gfx.width())
            {
                //Serial.printf("Overflowing LCD Width %d \n", gfx.width());
                TotalHeight += newRow->Height;
                //Serial.printf("Row has %d buttons, spacing = %d \n",newRow->Count, newRow->Spacing );
                _rows.push_back(newRow);
                newRow = new row_t();
            }
            newRow->Count++;
            newRow->Height = max(newRow->Height, button->Height());
            newRow->ButtonCenterWidth = gfx.width() / (newRow->Count *2);
            newRow->TotalWidth += button->Width();
            

        }
        //Serial.printf("Row has %d buttons \n",newRow->Count );
        _rows.push_back( newRow);
        for(auto row : _rows)
        {
            row->ButtonCenterHeight = gfx.height()/((_rows.size()+1)*2);
            uint16_t wSpacing = (gfx.width() - newRow->TotalWidth) / (newRow->Count + 1); // Spread remaining space
            uint16_t hSpacing = (gfx.height() -TotalHeight ) / (_rows.size()+1); // Spread remaining space            
            row->Spacing = min(hSpacing,wSpacing)/2;
        }
        //Serial.printf("Menu has %d buttons spread over %d rows \n",buttons.size(),_rows.size() );
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
        buttons.clear();
        if (FileName)
            free(FileName);
        if (Name)
            free(Name);
    }

    void Menu::SetFileName()
    {
        char *fullName = NULL;
        const char *nameTemplate = "/config/%s.json";
        size_t nameSize = printf(NULL, nameTemplate, Name);
        fullName = (char *)malloc(nameSize + 1);
        memset(fullName, 0x00, nameSize + 1);
        printf(fullName, nameTemplate, Name);
    }
    void Menu::ReleaseAll()
    {
        for(uint8_t b=0;b<buttons.size();b++)
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
            //Serial.printf("Activating menu %s\n",Name);
            for(int i=0;i<buttons.size();i++)
            {
                buttons[i]->Invalidate();
            }
            Draw(); 
        }
    }
    void Menu::Deactivate()
    {
        if(Active)
        {
            GFX.fillScreen(generalconfig.backgroundColour);
            Active=false;
            ReleaseAll();
        }
    }
    bool Menu::LoadConfig(File config)
    {
        char * configBuffer = (char *)malloc(config.size()+1) ;
        memset(configBuffer, 0x00, config.size()+1);
        size_t read_size = config.readBytes(configBuffer, config.size()+1);
        if(read_size!=config.size())
        {
            Serial.printf("[ERROR]: File size is %d bytes, file read was %d bytes", config.size(), read_size);
            return false;
        }
        LoadConfig(configBuffer);
        free(configBuffer);        
    }
    void Menu::AddHomeButton(uint8_t * position)
    {
        FTButton * newButton = new FTButton(GFX, "", *position++, MENU, "home.bmp", "question.bmp", _outline, _textSize, _textColor);
        newButton->actions.push_back( &homeMenu);
        buttons.push_back( newButton);
    }

    bool Menu::LoadConfig(const char * config)
    {
        cJSON * doc = cJSON_Parse(config);
        if (!doc)
        {
            const char * error=cJSON_GetErrorPtr();
            Serial.println("[ERROR]: Unable to parse json file : ");
            Serial.println(error);
            return false;
        }
        else
        {
            
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
                    CJSON_STRING_OR_DEFAULT(logoValue,cJSON_GetObjectItem(doc,logoName),"question.bmp" );
                    CJSON_STRING_OR_DEFAULT(latchLogoValue,cJSON_GetObjectItem(jsonButton,logoName),"" );

                    jsonLatchedLogo=cJSON_GetObjectItem(jsonButton,"latchlogo");
                    jsonLatch=cJSON_GetObjectItem(jsonButton, "latch");
                    ButtonTypes Latched=(jsonLatch && cJSON_IsBool(jsonLatch) && cJSON_IsTrue(jsonLatch))?LATCH : STANDARD;

                    FTButton * newButton = new FTButton(GFX, "", position,
                                              Latched ,
                                              logoValue,
                                              latchLogoValue, _outline, _textSize, _textColor);
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
                            Serial.println("[ERROR]: Current value does not match an action");
                            return false;
                        }
                        if(FTAction::GetType(jsonAction)!= ActionTypes::NONE)
                        {
                            // only push valid actions
                            newButton->actions.push_back(new FTAction(jsonAction,jsonActionValue));
                        }
                    }
                    buttons.push_back( newButton);
                    position++;
                }
                
                /* code */
            } while (jsonButton);
            
            //Serial.println("Done processing json structure");
            if (buttons.empty())
            {
                // this only contains menus
                position = 0;
                char menuName[31] = {0};
                do
                {
                    snprintf(logoName, sizeof(logoName), "logo%d", position);
                    snprintf(menuName, sizeof(menuName), "menu%d", position+1);
                    jsonLogo=cJSON_GetObjectItem(doc,logoName);
                    if(jsonLogo)
                    {
                        CJSON_STRING_OR_DEFAULT(logoValue,cJSON_GetObjectItem(doc,logoName),"question.bmp" );
                        auto newButton = new FTButton(GFX, "", position, MENU,logoValue, "question.bmp", _outline, _textSize, _textColor);
                        newButton->actions.push_back(new FTAction(ActionTypes::MENU,menuName));
                        buttons.push_back(newButton);
                    }   
                    position++;     

                } while (jsonLogo);
            }
            if(strcmp(Name, "homescreen")!=0 && !buttons.empty())
            {
                AddHomeButton(&position);
            }

            cJSON_Delete(doc);
        }
        return true;
    }
    void Menu::Touch(uint16_t x, uint16_t y)
    {
        for(uint8_t b = 0;b<buttons.size();b++)
        {
            if(buttons[b]->contains(x,y))
            {
                buttons[b]->Press();
            }
        }
        if(buttons.size()==0 && strcmp("config",Name))
        {
            QueueAction(&homeMenu);
        }
    }
    void Menu::SetMargin(uint16_t value)
    {
        _margin = value;
    }
}