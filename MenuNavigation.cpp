#include "stdint.h"
#include "MenuNavigation.h"
#include "Menu.h"
#include "FTAction.h"
#include <vector>
#include <TFT_eSPI.h>
#include <FS.h>
#include "FTAction.h"
namespace FreeTouchDeck
{
    FTAction *sleepSetLatchAction = new FTAction(ActionTypes::SETLATCH, "Preferences,Sleep");
    FTAction *sleepClearLatchAction = new FTAction(ActionTypes::CLEARLATCH, "Preferences,Sleep");
    FTAction *sleepToggleLatchAction = new FTAction(ActionTypes::TOGGLELATCH, "Preferences,Sleep");
    std::list<Menu *> PrevScreen;
    static const char *configMenu =
        R"({
	"name": "Preferences",
	"colscount": 3,
	"rowscount": 3,
    "icon": "spanner.bmp",
	"type": "HOMESYSTEM",
	"buttons": [
		{
			"label": "Configuration",
			"logo": "wifi.bmp",
			"type": "STANDARD",
			"actions": [
				{
					"type": "MENU",
					"symbol": "empty"
				},
				{
					"type": "LOCAL",
					"localactiontype": "ENTER_CONFIG"
				}
			]
		},
		{
			"label": "Brightness-Down",
			"logo": "brightnessdown.bmp",
			"type": "STANDARD",
			"actions": [
				{
					"type": "LOCAL",
					"localactiontype": "BRIGHTNESS_DOWN"
				}
			]
		},
		{
			"label": "Brightness-Up",
			"logo": "brightnessup.bmp",
			"type": "STANDARD",
			"actions": [
				{
					"type": "LOCAL",
					"localactiontype": "BRIGHTNESS_UP"
				}
			]
		},
		{
			"label": "Sleep",
			"logo": "sleep.bmp",
			"type": "LATCH",
			"actions": [
				{
					"type": "LOCAL",
					"localactiontype": "SLEEP"
				}
			]
		},
		{
			"label": "Info",
			"logo": "info.bmp",
			"type": "STANDARD",
			"actions": [
				{
					"type": "MENU",
					"symbol": "empty"
				},
				{
					"type": "LOCAL",
					"localactiontype": "INFO"
				}
			]
		}
	]
})";
    const char *action = R"([  
{"buttons":[],
"actions" :
[ 
    {
        "type":"MENU",
        "parm":"empty"
    }
]
}])";
    const char *blankMenu = R"({
		"name": "empty",
        "type": "SYSTEM",
        "actions":	[{
						"type":	"MENU",
						"symbol":	"~BACK"
					}]        
	})";
    static const char *module = "MenuNavigation";
    using namespace std;
    std::list<Menu *> Menus;
    SemaphoreHandle_t xScreenSemaphore = xSemaphoreCreateMutex();
    bool ScreenLock(TickType_t xTicksToWait)
    {
        //    ESP_LOGV(TAG, "Locking config json object");
        if (xSemaphoreTake(xScreenSemaphore, xTicksToWait) == pdTRUE)
        {
            //ESP_LOGD(TAG, "Screen Lock object locked!");
            return true;
        }
        else
        {
            ESP_LOGE(module, "Unable to lock the Screen object");
            return false;
        }
    }

    void ScreenUnlock()
    {
        //ESP_LOGD(TAG, "Screen object unlocked!");
        xSemaphoreGive(xScreenSemaphore);
    }
    Menu *GetActiveScreen()
    {
        Menu *ActiveScreen = NULL;
        ESP_LOGV(TAG, "Getting active screen");
        if (ScreenLock(portMAX_DELAY / portTICK_PERIOD_MS))
        {
            for (auto m : Menus)
            {
                if (m->Active)
                {
                    ActiveScreen = m;
                }
            }
            ScreenUnlock();
        }
        if (ActiveScreen)
        {
            ESP_LOGV(module, "Found active screen %s", ActiveScreen->Name);
        }
        else
        {
            ESP_LOGV(module, "No active screen found");
        }
        return ActiveScreen;
    }
    FreeTouchDeck::Menu *GetScreen(const char *name)
    {
        Menu *Match = NULL;
        ESP_LOGD(TAG, "Getting screen object for %s", name);
        if (ScreenLock(portMAX_DELAY / portTICK_PERIOD_MS))
        {
            if (strcmp(name, "~BACK") == 0)
            {
                if (PrevScreen.size() > 0)
                {
                    Match = PrevScreen.back();
                    PrevScreen.pop_back();
                }
                else
                {
                    Match = GetScreen("home");
                }
            }
            else
            {
                for (auto m : Menus)
                {
                    if (!strcmp(m->Name, name))
                    {
                        Match = m;
                    }
                }
            }

            if (!Match)
            {
                ESP_LOGE(module, "Screen %s not found", name);
            }
            else
            {
                ESP_LOGD(module, "Screen %s was found", name);
            }
            ScreenUnlock();
        }
        return Match;
    }
    bool SetActiveScreen(const char *name)
    {
        bool result = false;
        PrintMemInfo();
        Menu *Active = GetActiveScreen();
        PrintMemInfo();
        Menu *Match = GetScreen(name);
        PrintMemInfo();
        if (Match)
        {
            if (Active && Match && strcmp(Active->Name, Match->Name) == 0)
            {
                ESP_LOGD(module, "Screen %s is already active", name);
            }
            if (ScreenLock(portMAX_DELAY / portTICK_PERIOD_MS))
            {
                if (Active)
                {
                    if (strcmp(name, "~BACK") != 0)
                    {
                        PrevScreen.push_back(Active);
                    }
                    Active->Deactivate();
                }
                tft.fillScreen(generalconfig.backgroundColour);
                Match->Activate();
                ScreenUnlock();
                result = true;
            }
            PrintMemInfo();
        }
        else
        {
            ESP_LOGE(module, "Menu %s was not found", name);
        }
        return result;
    }
    bool RunActiveScreenAction(FTAction *action)
    {
        return SetActiveScreen(action->symbol);
    }
    FTButton *GetOldHomeButton(const char *MenuName)
    {
        FTButton *Match = NULL;
        // this function is assumed to be called with the creen object
        // already locked
        ESP_LOGD(TAG, "Getting old home screen button %s", MenuName);
        for (auto m : Menus)
        {
            if (m->Type == MenuTypes::OLDHOME)
            {
                ESP_LOGD(module, "Found Old menu entry %s", m->Name);
                Match = m->GetButtonForMenuName(MenuName);
                break;
            }
            else
            {
                ESP_LOGV(module, "Ignoring menu %s, type %s", m->Name, enum_to_string(m->Type));
            }
        }
        if (!Match)
        {
            ESP_LOGE(module, "Button %s not found in old home screen", MenuName);
        }
        else
        {
            ESP_LOGD(module, "Button %s was found", MenuName);
        }

        return Match;
    }
    bool PushJsonMenu(const char *menuString)
    {
        // This function assumes that the menu collection
        // object will be locked
        Menu *menu = NULL;
        bool result = false;
        if (ScreenLock(portMAX_DELAY / portTICK_PERIOD_MS))
        {

            menu = Menu::FromJson(menuString);
            if (menu)
            {
                Menus.push_back(menu);
                result = true;
            }
            else
            {
                ESP_LOGE(module, "Could not add menu");
            }
            ScreenUnlock();
        }
        else
        {
            ESP_LOGE(module, "Unable to add menus");
        }
        return result;
    }
    void LoadSystemMenus()
    {
        ESP_LOGD(module, "Adding settings menu");
        PushJsonMenu(configMenu);
        ESP_LOGD(module, "Adding blank menu");
        PushJsonMenu(blankMenu);
    }
    bool compare_nocase(const Menu *first, const Menu *second)
    {
        unsigned int i = 0;
        while ((i < strlen(first->Name)) && (i < strlen(second->Name)))
        {
            if (tolower(first->Name[i]) < tolower(second->Name[i]))
                return true;
            else if (tolower(first->Name[i]) > tolower(second->Name[i]))
                return false;
            ++i;
        }
        return (strlen(first->Name) < strlen(second->Name));
    }
    bool GenerateHomeScreen(bool sortFirst)
    {
        FTButton *oldButton = NULL;
        ESP_LOGD(module, "Generating home screen");
        // todo:  for "OLDHOME" menu types,
        // try to get the corresponding icon to show up on new homescreen
        cJSON *home = cJSON_CreateObject();
        cJSON_AddStringToObject(home, Menu::JsonLabelType, enum_to_string(MenuTypes::SYSTEM));
        cJSON_AddStringToObject(home, Menu::JsonLabelName, "home");
        cJSON_AddNumberToObject(home, Menu::JsonLabelColsCount, generalconfig.colscount);
        cJSON_AddNumberToObject(home, Menu::JsonLabelRowsCount, generalconfig.rowscount);
        if (Menus.size() > 0)
        {
            // sort by alphabetical order
            if (sortFirst)
            {
                Menus.sort(compare_nocase);
            }

            cJSON *buttons = cJSON_CreateArray();
            for (auto menu : Menus)
            {
                if (menu->Type == MenuTypes::HOME || menu->Type == MenuTypes::HOMESYSTEM)
                {
                    cJSON *button = cJSON_CreateObject();
                    cJSON_AddStringToObject(button, FTButton::JsonLabelLabel, menu->Name);
                    if (ISNULLSTRING(menu->Icon))
                    {
                        ESP_LOGD(module, "Processing old style menu. Fetching corresponding button for menu %s", menu->Name);
                        FTButton *buttonOld = GetOldHomeButton(menu->Name);
                        if (!buttonOld)
                        {
                            ESP_LOGE(module, "Unable to find old menu button. ");
                        }
                        else
                        {
                            ImageWrapper *image = buttonOld->Logo();
                            if (!ISNULLSTRING(image->LogoName))
                            {
                                ESP_LOGD(menu, "Menu button has logo %s", image->LogoName);
                                cJSON_AddStringToObject(button, FTButton::JsonLabelLogo, image->LogoName);
                            }
                            else
                            {
                                ESP_LOGE(module, "Defaulting button to question.bmp");
                                cJSON_AddStringToObject(button, FTButton::JsonLabelLogo, "question.bmp");
                            }
                        }
                    }
                    else
                    {
                        cJSON_AddStringToObject(button, FTButton::JsonLabelLogo, menu->Icon);
                    }

                    cJSON_AddStringToObject(button, FTButton::JsonLabelType, enum_to_string(ButtonTypes::MENU));

                    FTAction *action = new FTAction(ActionTypes::MENU, menu->Name);
                    cJSON *actions = cJSON_CreateArray();
                    cJSON_AddItemToArray(actions, action->ToJson());
                    cJSON_AddItemToObject(button, FTButton::JsonLabelActions, actions);
                    delete (action);
                    DumpCJson(button);
                    cJSON_AddItemToArray(buttons, button);
                }
            }
            cJSON_AddItemToObject(home, Menu::JsonLabelButtons, buttons);

            ESP_LOGD(module, "Instantiating home menu entry");
            Menu *m = new FreeTouchDeck::Menu(home);
            if (m)
            {
                ESP_LOGD(module, "Home screen has %d buttons", m->buttons.size());
                Menus.push_back(m);
            }
            else
            {
                ESP_LOGE(module, "Home screen creation failed");
            }

            cJSON_Delete(home);
        }
        else
        {
            ESP_LOGE(module, "No menu was found");
        }
        return true;
    }
    bool LoadFullFormat()
    {
        bool result = false;
        File menus = SPIFFS.open("/config/menus.json", "r");
        if (!menus || menus.size() == 0)
        {
            return false;
        }
        char *fullbuffer = (char *)malloc_fn(menus.size() + 1);
        if (!fullbuffer)
        {
            drawErrorMessage(true, module, "Unable to load menus file. Insufficient memory to allocate %d bytes", menus.size() + 1);
        }
        else
        {
            if (menus.readBytes(fullbuffer, menus.size()) != menus.size())
            {
                drawErrorMessage(true, module, "Could not read file %s", menus.name());
            }
            else
            {
                result = PushJsonMenu(fullbuffer);
            }
        }
        menus.close();
        if (result)
        {
            // generate home screen. Don't sort
            // as the configuration file should
            // list buttons in the order in which
            // they should be displayed
            result = GenerateHomeScreen(false);
        }
        return result;
    }
    void LoadAllMenus()
    {
        if (ScreenLock(portMAX_DELAY / portTICK_PERIOD_MS))
        {
            if (!LoadFullFormat())
            {
                File root = SPIFFS.open("/");
                File file = root.openNextFile();
                while (file)
                {
                    String FileName = file.name();
                    if (FileName.startsWith("/config/menu") || FileName.startsWith("/config/homescreen"))
                    {
                        ESP_LOGD(module, "Adding menu from file %s", file.name());
                        Menus.push_back(new FreeTouchDeck::Menu(&file));
                        PrintMemInfo();
                        ESP_LOGD(module, "Adding menu completed. Getting next file");
                    }
                    file = root.openNextFile();
                }
                // generate home screen with a sorted list.
                // This will ensure a positionning
                // similar to the old naming scheme
                GenerateHomeScreen(true);
                ESP_LOGD(module, "Done Adding home screen menu from file name homescreen");
                root.close();
            }
            ScreenUnlock();
        }
        else
        {
            ESP_LOGE(module, "Unable to add menus");
        }
    }
    Menu *GetLatchScreen(FTAction *action)
    {
        char screenName[51] = {0};
        char buttonName[51] = {0};
        Menu *menu = NULL;
        if (action->GetLatchButton(screenName, sizeof(screenName), buttonName, sizeof(buttonName)))
        {
            menu = GetScreen(screenName);
        }
        return menu;
    }
    bool RunLatchAction(FTAction *action)
    {
        bool success = false;
        Menu *menu = GetLatchScreen(action);
        if (menu)
        {
            ESP_LOGD(module, "Running Latch Action on Menu %s", menu->Name);
            success = menu->Button(action);
            if (!success)
            {
                ESP_LOGE(module, "Running Latch action failed");
            }
        }
        else
        {
            ESP_LOGE(module, "Screen not found for latch action.");
        }
        return success;
    }
    char *MenusToJson(bool withSystem = false)
    {
        char *json = NULL;
        cJSON *menusArray = cJSON_CreateArray();
        ESP_LOGD(module, "Locking menu object");
        if (ScreenLock(portMAX_DELAY / portTICK_PERIOD_MS))
        {
            for (auto m : Menus)
            {
                if ((m->Type == MenuTypes::SYSTEM || m->Type == MenuTypes::HOMESYSTEM) && !withSystem)
                    continue; // don't output system menus as they are built-in
                ESP_LOGD(module, "Converting menu %s", STRING_OR_DEFAULT(m->Name, "unknown"));
                PrintMemInfo();
                cJSON * menuEntry=m->ToJSON();
                cJSON_AddItemToArray(menusArray, menuEntry);
                json = cJSON_Print(menuEntry);
                if (json)
                {
                    ESP_LOGD(module, "%s", json);
                    FREE_AND_NULL(json);
                }
            }
            ESP_LOGD(module, "Unlocking menu object");
            ScreenUnlock();
        }
        else
        {
            ESP_LOGE(module, "Unable to lock screens ");
        }
        PrintMemInfo();
        ESP_LOGD(module, "Menus were converted to JSON structure. Converting to string");
        json = cJSON_Print(menusArray);
        if (json)
        {
            ESP_LOGD(module, "JSON menu structure contains %d bytes", strlen(json));
        }
        else
        {
            ESP_LOGE(module, "Json structure could not be converted to string");
        }
        cJSON_Delete(menusArray);
        ESP_LOGD(module, "Done converting menu to json structure");
        PrintMemInfo();
        return json;
    }
}