#include "MenuNavigation.h"
#include "Menu.h"
#include "FTAction.h"
#include <vector>
#include <TFT_eSPI.h>
#include "FTAction.h"
#include "Storage.h"
namespace FreeTouchDeck
{
    FTAction *sleepSetLatchAction = new FTAction(ParametersList_t({"LATCH", "Preferences", "Sleep", "ON"}));
    FTAction *sleepClearLatchAction = new FTAction(ParametersList_t({"LATCH", "Preferences", "Sleep", "OFF"}));
    FTAction *sleepToggleLatchAction = new FTAction(ParametersList_t({"LATCH", "Preferences", "Sleep", "TOGGLE"}));
    FTAction *beepSetLatchAction = new FTAction(ParametersList_t({"LATCH", "Preferences", "Beep", "ON"}));
    FTAction *beepClearLatchAction = new FTAction(ParametersList_t({"LATCH", "Preferences", "Beep", "OFF"}));
    FTAction *criticalMessage = new FTAction(ParametersList_t({"MENU", "criticalmessage"}));
    const char *MenuActionTemplate = "{MENU:%s}";
    std::vector<Menu *> PrevScreen;
    static const char *configMenu =
        R"({
	"name": "Preferences",
	"colscount": 3,
	"rowscount": 3,
    "logo": "spanner.jpg",
	"type": "HOMESYSTEM",
	"buttons": [
		{
			"label": "Configuration",
			"logo": "wifi.jpg",
			"type": "STANDARD",
			"actions": ["{MENU:empty}{ENTER_CONFIG}"]
		},
		{
			"label": "Brightness-Down",
			"logo": "brightnessdown.jpg",
			"type": "STANDARD",
			"actions": ["{BRIGHTNESS_DOWN}"]
		},
		{
			"label": "Brightness-Up",
			"logo": "brightnessup.jpg",
			"type": "STANDARD",
			"actions": ["{BRIGHTNESS_UP}"]
		},
		{
			"label": "Sleep",
			"logo": "sleep.jpg",
			"type": "LATCH",
			"actions": ["{SLEEP}"]
		},
		{
			"label": "Beep",
			"logo": "music.jpg",
			"type": "LATCH",
			"actions": ["{BEEP}"]
		},        
		{
			"label": "Info",
			"logo": "info.jpg",
			"type": "STANDARD",
			"actions": ["{MENU:empty}{INFO}"]

		},        
		{
			"label": "Sleep",
			"type": "STANDARD",
			"actions": ["{STARTSLEEP}"]
		}
	]
})";

    const char *blankMenu = R"({
		"name": "empty",
        "type": "EMPTY",
		"actions": ["{MENU:~BACK}"]
	})";
    const char *messageMenu = R"({
		"name": "criticalmessage",
        "type": "EMPTY",
        "actions":	["{REBOOT}"]        
	})";
    static const char *module = "MenuNavigation";
    using namespace std;
    std::vector<Menu *> Menus;
    SemaphoreHandle_t xScreenSemaphore = xSemaphoreCreateMutex();
    bool ScreenLock(TickType_t xTicksToWait)
    {
        //    LOC_LOGV(TAG, "Locking config json object");
        if (xSemaphoreTake(xScreenSemaphore, xTicksToWait) == pdTRUE)
        {
            //LOC_LOGD(TAG, "Screen Lock object locked!");
            return true;
        }
        else
        {
            LOC_LOGE(module, "Unable to lock the Screen object");
            return false;
        }
    }

    void ScreenUnlock()
    {
        //LOC_LOGD(TAG, "Screen object unlocked!");
        xSemaphoreGive(xScreenSemaphore);
    }
    Menu *GetActiveScreen()
    {
        Menu *ActiveScreen = NULL;
        LOC_LOGV(TAG, "Getting active screen");
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
            LOC_LOGV(module, "Found active screen %s", ActiveScreen->Name);
        }
        else
        {
            LOC_LOGV(module, "No active screen found");
        }
        return ActiveScreen;
    }
    FreeTouchDeck::Menu *GetScreen(const char *name, bool lock)
    {
        Menu *Match = NULL;
        LOC_LOGD(TAG, "Getting screen object for %s", name);
        if (!lock || ScreenLock(portMAX_DELAY / portTICK_PERIOD_MS))
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
                    if (!strcmp(m->Name.c_str(), name))
                    {
                        Match = m;
                    }
                }
            }

            if (!Match)
            {
                LOC_LOGD(module, "Screen %s not found", name);
            }
            else
            {
                LOC_LOGD(module, "Screen %s was found", name);
            }
            if (lock)
                ScreenUnlock();
        }
        return Match;
    }
    bool SetActiveScreen(const char *name)
    {
        bool result = false;
        PrintMemInfo(__FUNCTION__, __LINE__);
        if (Menus.size() == 0)
            return false;
        Menu *Active = GetActiveScreen();
        PrintMemInfo(__FUNCTION__, __LINE__);
        Menu *Match = GetScreen(name);
        PrintMemInfo(__FUNCTION__, __LINE__);
        if (Match)
        {
            if (Active && Match && strcmp(Active->Name.c_str(), Match->Name.c_str()) == 0)
            {
                LOC_LOGD(module, "Screen %s is already active", name);
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
                Match->Activate();
                if (strcmp("home", Match->Name.c_str()) == 0 && PrevScreen.size() > 0)
                {
                    LOC_LOGD(module, "Returning to home from lower level menu. Clearing navigation stack");
                    PrevScreen.clear();
                }

                ScreenUnlock();
                result = true;
            }
            PrintMemInfo(__FUNCTION__, __LINE__);
        }
        else
        {
            LOC_LOGE(module, "Menu %s was not found", name);
        }
        return result;
    }

    bool AddReplaceMenuEntry(Menu *menu)
    {
        if (!menu)
        {
            LOC_LOGE(module, "Invalid menu object!");
            return false;
        }
        LOC_LOGD(module, "Checking if menu %s already exists in list of %d", menu->Name.c_str(), Menus.size());
        for (int i = 0; i < Menus.size(); i++)
        {
            if (Menus[i]->Name == menu->Name)
            {
                LOC_LOGD(module, "Replacing existing menu %s in the structure", menu->Name.c_str());
                delete (Menus[i]);
                Menus[i] = menu;
                return true;
            }
        }
        LOC_LOGD(module, "Adding menu %s to the list", menu->Name.c_str());
        Menus.push_back(menu);
        return true;
    }
    bool PushJsonMenu(cJSON *menuJson)
    {
        // This function assumes that the menu collection
        // object will be locked
        Menu *menu = NULL;
        LOC_LOGD(module, "Instantiating new menu from a JSON object");
        PrintMemInfo(__FUNCTION__, __LINE__);
        try
        {
            menu = new Menu(menuJson);
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << '\n';
        }

        PrintMemInfo(__FUNCTION__, __LINE__);
        return AddReplaceMenuEntry(menu);
    }
    bool PushJsonMenu(const char *menuString)
    {
        // This function assumes that the menu collection
        // object will be locked
        Menu *menu = NULL;
        bool result = true;
        PrintMemInfo(__FUNCTION__, __LINE__);
        cJSON *doc = cJSON_Parse(menuString);
        if (!doc)
        {
            const char *error = cJSON_GetErrorPtr();
            LOC_LOGE(module, "Menu parsing failed: %s", error);
            drawErrorMessage(true, module, "Unable to parse json string : %s", error);
            return false;
        }
        else
        {
            LOC_LOGD(module, "Menu structure was parsed successfully.");
            if (cJSON_IsArray(doc))
            {
                LOC_LOGD(module, "Processing array with %d entries.", cJSON_GetArraySize(doc));
                cJSON *menuJson = NULL;
                cJSON_ArrayForEach(menuJson, doc)
                {
                    LOC_LOGD(module, "Pushing one menu");
                    result = !result ? result : PushJsonMenu(menuJson);
                }
            }
            else
            {
                LOC_LOGD(module, "Processing single menu.");
                result = PushJsonMenu(doc);
            }
        }
        cJSON_Delete(doc);
        PrintMemInfo(__FUNCTION__, __LINE__);
        return result;
    }

    void LoadSystemMenus()
    {
        LOC_LOGD(module, "Adding settings menu");
        PushJsonMenu(configMenu);
        LOC_LOGD(module, "Adding blank menu");
        PushJsonMenu(blankMenu);
        LOC_LOGD(module, "Adding System Message Screen");
        PushJsonMenu(messageMenu);
    }
    bool compare_nocase(const Menu *first, const Menu *second)
    {
        unsigned int i = 0;
        while ((i < first->Name.length()) && (i < second->Name.length()))
        {
            if (tolower(first->Name[i]) < tolower(second->Name[i]))
                return true;
            else if (tolower(first->Name[i]) > tolower(second->Name[i]))
                return false;
            ++i;
        }
        return (first->Name.length()) < second->Name.length();
    }
    bool GenerateHomeScreen()
    {
        FTButton *oldButton = NULL;
        LOC_LOGD(module, "Generating home screen");
        // todo:  for "OLDHOME" menu types,
        // try to get the corresponding icon to show up on new homescreen
        cJSON *home = cJSON_CreateObject();
        cJSON_AddStringToObject(home, Menu::JsonLabelType, enum_to_string(MenuTypes::ROOT));
        cJSON_AddStringToObject(home, Menu::JsonLabelName, "home");
        cJSON_AddNumberToObject(home, Menu::JsonLabelColsCount, generalconfig.colscount);
        cJSON_AddNumberToObject(home, Menu::JsonLabelRowsCount, generalconfig.rowscount);
        if (Menus.size() > 0)
        {

            cJSON *buttons = cJSON_CreateArray();
            for (auto menu : Menus)
            {
                if (menu->Type == MenuTypes::HOME || menu->Type == MenuTypes::HOMESYSTEM)
                {
                    cJSON *button = cJSON_CreateObject();
                    cJSON_AddStringToObject(button, FTButton::JsonLabelLabel, menu->Label.c_str());
                    if (!menu->Icon.empty())
                    {
                        cJSON_AddStringToObject(button, FTButton::JsonLabelLogo, menu->Icon.c_str());
                    }
                    cJSON *actions = cJSON_CreateArray();
                    char *menuActionString = (char *)malloc_fn(strlen(MenuActionTemplate) + menu->Name.length());
                    sprintf(menuActionString, MenuActionTemplate, menu->Name.c_str());
                    cJSON_AddItemToArray(actions, cJSON_CreateString(menuActionString));
                    FREE_AND_NULL(menuActionString);
                    // if the menu has an action, make sure it is also added
                    for (ActionsSequences &a : menu->Actions)
                    {
                        cJSON_AddItemToArray(actions, cJSON_CreateString(a.ConfigSequence));
                    }
                    cJSON_AddItemToObject(button, FTButton::JsonLabelActions, actions);
                    DumpCJson(button);
                    cJSON_AddItemToArray(buttons, button);
                }
            }
            cJSON_AddItemToObject(home, Menu::JsonLabelButtons, buttons);

            LOC_LOGD(module, "Instantiating home menu entry");
            Menu *m = new FreeTouchDeck::Menu(home);
            if (m)
            {
                LOC_LOGD(module, "Home screen has %d buttons", m->buttons.size());
                Menus.push_back(m);
            }
            else
            {
                LOC_LOGE(module, "Home screen creation failed");
            }

            cJSON_Delete(home);
        }
        else
        {
            LOC_LOGE(module, "No menu was found");
        }
        return true;
    }
    bool GenerateHomeScreenObject()
    {
        LOC_LOGD(module, "Generating home screen");
        // todo:  for "OLDHOME" menu types,
        // try to get the corresponding icon to show up on new homescreen
        Menu *home = new Menu(MenuTypes::ROOT, "home", "", "", generalconfig.rowscount, generalconfig.colscount, generalconfig.backgroundColour, generalconfig.DefaultOutline, generalconfig.DefaultTextColor, generalconfig.DefaultTextSize);
        if (Menus.size() > 0)
        {

            for (auto menu : Menus)
            {
                if (menu->Type == MenuTypes::HOME || menu->Type == MenuTypes::HOMESYSTEM)
                {
                    LOC_LOGD(module,"Creating home screen button for menu %s", menu->Label.c_str());
                    auto button = FTButton(ButtonTypes::STANDARD, menu->Label.c_str(), menu->Icon.c_str(), "", generalconfig.DefaultOutline, generalconfig.DefaultTextSize, generalconfig.DefaultTextColor);
                    ActionsSequences sequences;
                    char *menuActionString = (char *)malloc_fn(strlen(MenuActionTemplate) + menu->Name.length());
                    sprintf(menuActionString, MenuActionTemplate, menu->Name.c_str());
                    sequences.Parse(menuActionString);
                    FREE_AND_NULL(menuActionString);
                    button.Sequences.push_back(sequences);
                    home->AddButton(button);
                }
            }
            Menus.push_back(home);
        }
        else
        {
            LOC_LOGE(module, "No menu was found");
        }
        return true;
    }
    bool SaveFullFormat()
    {
        LOC_LOGI(module, "Saving full menu structure");
        File menus = ftdfs->open("/config/menus.json", FILE_WRITE);
        if (!menus)
        {
            LOC_LOGE(module, "Error opening menus.json");
            return false;
        }
        char *json = MenusToJson(false);
        if (json)
        {

            size_t written = menus.write((const uint8_t *)json, strlen(json));
            menus.close();
            if (strlen(json) != written)
            {
                ESP_LOGE(module, "Expected to write %d bytes but only %d bytes were written", strlen(json), written);
                ftdfs->remove("/config/menus.json");
            }
            FREE_AND_NULL(json);
        }
        else
        {
            LOC_LOGE(module, "Unable to print menu structure");
        }
    }
    bool LoadFullFormat()
    {
        return LoadFullFormat("/config/menus.json");
    }
    bool LoadFullFormat(const char *fileName)
    {
        bool result = false;
        LOC_LOGI(module, "Loading menu structure from %s", fileName);
        PrintMemInfo(__FUNCTION__, __LINE__);
        File menus = ftdfs->open(fileName, FILE_READ);
        if (!menus || menus.size() == 0)
        {
            LOC_LOGW(module, "File not found or file is empty");
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
                LOC_LOGD(module, "Menu structure was read from file. Parsing");
                result = PushJsonMenu(fullbuffer);
            }
        }
        menus.close();
        FREE_AND_NULL(fullbuffer);
        PrintMemInfo(__FUNCTION__, __LINE__);
        return result;
    }
    void LoadAllMenus()
    {
        if (ScreenLock(portMAX_DELAY / portTICK_PERIOD_MS))
        {
            if (LoadFullFormat())
            {
                LoadSystemMenus();
                GenerateHomeScreenObject();
            }
            else
            {
                drawErrorMessage(true, module, "Unable to load file /config/menus.json");
            }

            LOC_LOGD(module, "Done Adding home screen menu from file name homescreen");
            ScreenUnlock();
        }
        else
        {
            LOC_LOGE(module, "Unable to add menus");
        }
    }
    Menu *GetLatchScreen(FTAction *action)
    {
        return GetScreen(action->FirstParameter());
    }

    cJSON *MenusToJsonObject(bool withSystem)
    {
        cJSON *menusArray = cJSON_CreateArray();
        LOC_LOGD(module, "Locking menu object");
        if (ScreenLock(portMAX_DELAY / portTICK_PERIOD_MS))
        {
            for (auto m : Menus)
            {
                if ((m->Type == MenuTypes::SYSTEM || m->Type == MenuTypes::HOMESYSTEM) && !withSystem)
                    continue; // don't output system menus as they are built-in
                LOC_LOGD(module, "Converting menu %s", m->Name.c_str());
                PrintMemInfo(__FUNCTION__, __LINE__);
                cJSON *menuEntry = m->ToJSON();
                cJSON_AddItemToArray(menusArray, menuEntry);
            }
            LOC_LOGD(module, "Unlocking menu object");
            ScreenUnlock();
        }
        else
        {
            LOC_LOGE(module, "Unable to lock screens ");
        }
        PrintMemInfo(__FUNCTION__, __LINE__);
        LOC_LOGD(module, "Menus were converted to JSON structure. Converting to string");
        return menusArray;
    }
    char *MenusToJson(bool withSystem)
    {
        cJSON *menusArray = MenusToJsonObject(withSystem);
        char *json = NULL;
        json = cJSON_Print(menusArray);
        if (json)
        {
            LOC_LOGD(module, "JSON menu structure contains %d bytes", strlen(json));
        }
        else
        {
            LOC_LOGE(module, "Json structure could not be converted to string");
        }
        cJSON_Delete(menusArray);
        LOC_LOGD(module, "Done converting menu to json structure");
        PrintMemInfo(__FUNCTION__, __LINE__);
        return json;
    }
    void handleDisplay(bool pressed, uint16_t t_x, uint16_t t_y)
    {
        static unsigned nextlog = 0;
        auto Active = GetActiveScreen();
        if (Active)
        {
            if (pressed)
            {
                Active->Touch(t_x, t_y);
            }
            else
            {
                Active->ReleaseAll();
            }
            Active->DrawShape();
            Active->DrawImages();
        }
        else
        {
            if (nextlog <= millis())
            {
                LOC_LOGD(module, "No active display");
                // to prevent flooding of the serial log, reduce the rate of this message
                nextlog = millis() + 1000;
            }
        }
    }
}