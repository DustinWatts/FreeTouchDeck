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
    std::list<Menu *> PrevScreen;
    static const char *configMenu =
        R"({
	"name": "Preferences",
	"colscount": 3,
	"rowscount": 3,
    "logo": "spanner.bmp",
	"type": "HOMESYSTEM",
	"buttons": [
		{
			"label": "Configuration",
			"logo": "wifi.bmp",
			"type": "STANDARD",
			"actions": ["{MENU:empty}{ENTER_CONFIG}"]
		},
		{
			"label": "Brightness-Down",
			"logo": "brightnessdown.bmp",
			"type": "STANDARD",
			"actions": ["{BRIGHTNESS_DOWN}"]
		},
		{
			"label": "Brightness-Up",
			"logo": "brightnessup.bmp",
			"type": "STANDARD",
			"actions": ["{BRIGHTNESS_UP}"]
		},
		{
			"label": "Sleep",
			"logo": "sleep.bmp",
			"type": "LATCH",
			"actions": ["{SLEEP}"]
		},
		{
			"label": "Beep",
			"logo": "music.bmp",
			"type": "LATCH",
			"actions": ["{BEEP}"]
		},        
		{
			"label": "Info",
			"logo": "info.bmp",
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
    std::list<Menu *> Menus;
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
                // stop executing any pending keyboard action
                EmptyQueue();
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
        PrintMemInfo();
        if (Menus.size() == 0)
            return false;
        Menu *Active = GetActiveScreen();
        PrintMemInfo();
        Menu *Match = GetScreen(name);
        PrintMemInfo();
        if (Match)
        {
            if (Active && Match && strcmp(Active->Name, Match->Name) == 0)
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
                if(strcmp("home",Match->Name)==0 && PrevScreen.size()>0)
                {
                    LOC_LOGD(module,"Returning to home from lower level menu. Clearing navigation stack");
                    PrevScreen.clear();
                }                

                ScreenUnlock();
                result = true;
            }
            PrintMemInfo();
        }
        else
        {
            LOC_LOGE(module, "Menu %s was not found", name);
        }
        return result;
    }

    void DeleteMenuEntry(const char *name)
    {
        Menu *existing = GetScreen(name, false);
        if (existing)
        {
            LOC_LOGD(module, "Removing existing menu %s", name);
            delete (existing);
            Menus.remove(existing);
        }
        else
        {
            LOC_LOGD(module, "Menu %s is new", STRING_OR_DEFAULT(name, "?"));
        }
    }
    bool AddReplaceMenuEntry(Menu *menu)
    {
        if (!menu)
        {
            LOC_LOGE(module, "Invalid menu object!");
            return false;
        }
        LOC_LOGD(module, "Checking if menu %s exists", STRING_OR_DEFAULT(menu->Name, "?"));
        DeleteMenuEntry(menu->Name);
        LOC_LOGD(module, "Adding menu %s to the list", STRING_OR_DEFAULT(menu->Name, "?"));
        Menus.push_back(menu);
        return true;
    }
    bool PushJsonMenu(cJSON *menuJson)
    {
        // This function assumes that the menu collection
        // object will be locked
        Menu *menu = new Menu(menuJson);
        return AddReplaceMenuEntry(menu);
    }
    bool PushJsonMenu(const char *menuString)
    {
        // This function assumes that the menu collection
        // object will be locked
        Menu *menu = NULL;
        bool result = true;

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
                    cJSON_AddStringToObject(button, FTButton::JsonLabelLabel, STRING_OR_DEFAULT(menu->Label, STRING_OR_DEFAULT(menu->Name, "")));
                    if (!ISNULLSTRING(menu->Icon))
                    {
                        cJSON_AddStringToObject(button, FTButton::JsonLabelLogo, menu->Icon);
                    }
                    cJSON *actions = cJSON_CreateArray();
                    char *menuActionString = (char *)malloc_fn(strlen(MenuActionTemplate) + strlen(menu->Name));
                    sprintf(menuActionString, MenuActionTemplate, menu->Name);
                    cJSON_AddItemToArray(actions, cJSON_CreateString(menuActionString));
                    FREE_AND_NULL(menuActionString);
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
    bool GenerateHomeScreenObject(bool sortFirst)
    {
        LOC_LOGD(module, "Generating home screen");
        // todo:  for "OLDHOME" menu types,
        // try to get the corresponding icon to show up on new homescreen
        Menu *home = new Menu(MenuTypes::ROOT, "home", "", "", generalconfig.rowscount, generalconfig.colscount, generalconfig.backgroundColour, generalconfig.DefaultOutline, generalconfig.DefaultTextColor, generalconfig.DefaultTextSize);
        if (Menus.size() > 0)
        {
            // sort by alphabetical order
            if (sortFirst)
            {
                Menus.sort(compare_nocase);
            }
            for (auto menu : Menus)
            {
                if (menu->Type == MenuTypes::HOME || menu->Type == MenuTypes::HOMESYSTEM)
                {
                    FTButton *button = new FTButton(ButtonTypes::STANDARD, STRING_OR_DEFAULT(menu->Label, STRING_OR_DEFAULT(menu->Name, "")), menu->Icon, "", generalconfig.DefaultOutline, generalconfig.DefaultTextSize, generalconfig.DefaultTextColor);
                    ActionsSequences sequences;
                    char *menuActionString = (char *)malloc_fn(strlen(MenuActionTemplate) + strlen(menu->Name));
                    sprintf(menuActionString, MenuActionTemplate, menu->Name);
                    sequences.Parse(menuActionString);
                    FREE_AND_NULL(menuActionString);
                    button->Sequences.push_back(sequences);
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
        if (result)
        {
            // generate home screen. Don't sort
            // as the configuration file should
            // list buttons in the order in which
            // they should be displayed
            result = GenerateHomeScreenObject(false);
        }
        FREE_AND_NULL(fullbuffer);
        return result;
    }
    void LoadAllMenus()
    {
        if (ScreenLock(portMAX_DELAY / portTICK_PERIOD_MS))
        {
            if (LoadFullFormat())
            {
                LoadSystemMenus();
                GenerateHomeScreenObject(false);
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
    char *MenusToJson(bool withSystem)
    {
        char *json = NULL;
        cJSON *menusArray = cJSON_CreateArray();
        LOC_LOGD(module, "Locking menu object");
        if (ScreenLock(portMAX_DELAY / portTICK_PERIOD_MS))
        {
            for (auto m : Menus)
            {
                if ((m->Type == MenuTypes::SYSTEM || m->Type == MenuTypes::HOMESYSTEM) && !withSystem)
                    continue; // don't output system menus as they are built-in
                LOC_LOGD(module, "Converting menu %s", STRING_OR_DEFAULT(m->Name, "unknown"));
                PrintMemInfo();
                cJSON *menuEntry = m->ToJSON();
                cJSON_AddItemToArray(menusArray, menuEntry);
                json = cJSON_Print(menuEntry);
                if (json)
                {
                    LOC_LOGD(module, "%s", json);
                    FREE_AND_NULL(json);
                }
            }
            LOC_LOGD(module, "Unlocking menu object");
            ScreenUnlock();
        }
        else
        {
            LOC_LOGE(module, "Unable to lock screens ");
        }
        PrintMemInfo();
        LOC_LOGD(module, "Menus were converted to JSON structure. Converting to string");
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
        PrintMemInfo();
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