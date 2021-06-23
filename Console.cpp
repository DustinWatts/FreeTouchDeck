#include "globals.hpp"
#include "Console.h"
#include "Storage.h"
#include "MenuNavigation.h"
#include "ConfigHelper.h"
#include "ConfigLoad.h"
namespace FreeTouchDeck
{
    static const char *module = "Console";
    String readLine()
    {
        String ret;
        int c = Serial.read();
        while (c >= 0 && c != '\r' && c != '\n')
        {
            ret += (char)c;
            c = Serial.read();
        }
        while (c >= 0 && (c == '\r' || c == '\n'))
        {
            c = Serial.read();
        }
        LOC_LOGD(module, "Processing: %s", ret.c_str());
        return ret;
    }
    void processSerial()
    {
        // Check if there is data available on the serial input that needs to be handled.

        if (Serial.available())
        {
            String command = readLine();
            Serial.printf("Executing command %s\n", command.c_str());
            if (command == "cal")
            {
                ftdfs->remove(CALIBRATION_FILE);
                ESP.restart();
            }
            else if (command.startsWith("loglevel"))
            {
                String level = command.substring(command.lastIndexOf(" "));
                LOC_LOGI(module, "Attempting to set log level to %s", level.c_str());
                LogLevels lev = static_cast<LogLevels>(level.toInt());
                if (lev >= LogLevels::NONE && lev <= LogLevels::VERBOSE)
                {
                    generalconfig.LogLevel = lev;
                    LOC_LOGI(module, "Log level changed to %d", generalconfig.LogLevel);
                    QueueSaving();
                }
                else
                {
                    LOC_LOGE(module, "Invalid log level %s", level.c_str());
                }
            }
            else if (command == "console")
            {
                LOC_LOGW(module, "Changing mode to console");
                ChangeMode(SystemMode::CONSOLE);
            }
            else if (command == "configmode")
            {
                LOC_LOGW(module, "Changing mode to configuration");
                ChangeMode(SystemMode::CONFIG);
            }
            else if (command == "dir")
            {
                ShowDir();
            }

            else if (command == "menus")
            {
                LOC_LOGI(module, "Generating menus structure");
                char *json = FreeTouchDeck::MenusToJson(true);
                if (json)
                {
                    LOC_LOGI(module, "Menu structure: \n%s", json);
                    FREE_AND_NULL(json);
                }
                else
                {
                    LOC_LOGE(module, "Unable to print menu structure");
                }
            }
            else if (command == "memory")
            {
                LOC_LOGI(module, "free_iram: %d", heap_caps_get_free_size(MALLOC_CAP_INTERNAL));
                LOC_LOGI(module, "min_free_iram: %d", heap_caps_get_minimum_free_size(MALLOC_CAP_INTERNAL));
            }

            else if (command.startsWith("activate"))
            {
                String value = command.substring(command.lastIndexOf(" "));
                value.trim();
                LOC_LOGD(module, "Activating screen %s", value.c_str());
                if (!SetActiveScreen(value.c_str()))
                {
                    LOC_LOGE(module, "unable to activate screen %s", value.c_str());
                }
            }
            else if (command.startsWith("rot"))
            {
                String value = command.substring(command.lastIndexOf(" "));
                value.trim();
                uint8_t rot = value.toInt();
                if (rot <= 3 && rot >= 0)
                {
                    generalconfig.screenrotation = rot;
                    QueueSaving();
                    LOC_LOGI(module, "Screen rotation was updated to %d", generalconfig.screenrotation);
                }
            }
            else if (command == "revx")
            {
                generalconfig.reverse_x_touch = !generalconfig.reverse_x_touch;
                LOC_LOGI(module, "X axis touch reverse set to %s", generalconfig.reverse_x_touch ? "YES" : "NO");
                QueueSaving();
            }
            else if (command.startsWith("rows"))
            {
                String value = command.substring(command.lastIndexOf(" "));
                value.trim();
                generalconfig.rowscount = value.toInt();
                LOC_LOGI(module, "Rows count set to %d", generalconfig.rowscount);
                QueueSaving();
            }
            else if (command.startsWith("cols"))
            {
                String value = command.substring(command.lastIndexOf(" "));
                value.trim();
                generalconfig.colscount = value.toInt();
                LOC_LOGI(module, "Rows count set to %d", generalconfig.colscount);
                QueueSaving();
            }
            else if (command == "revy")
            {
                generalconfig.reverse_y_touch = !generalconfig.reverse_y_touch;
                LOC_LOGI(module, "Y axis touch reverse set to %s", generalconfig.reverse_y_touch ? "YES" : "NO");
                QueueSaving();
            }
            else if (command == "invaxis")
            {
                generalconfig.flip_touch_axis = !generalconfig.flip_touch_axis;
                LOC_LOGI(module, "Touch axis flip set to %s", generalconfig.flip_touch_axis ? "YES" : "NO");
                QueueSaving();
            }
            else if (command.startsWith("setssid"))
            {
                String value = command.substring(command.indexOf(" "));
                value.trim();
                if (saveWifiSSID(value))
                {
                    LOC_LOGI(module, "Saved new SSID: %s\n", value.c_str());
                    loadWifiConfig();
                    LOC_LOGI(module, "New configuration loaded");
                }
            }
            else if (command.startsWith("setpassword"))
            {
                String value = command.substring(command.indexOf(" "));
                value.trim();
                if (saveWifiPW(value))
                {
                    LOC_LOGI(module, "Saved new Password: %s\n", value.c_str());
                    loadWifiConfig();
                    LOC_LOGI(module, "New configuration loaded");
                }
            }
            else if (command.startsWith("setwifimode"))
            {
                String value = command.substring(command.indexOf(" "));
                value.trim();
                if (saveWifiMode(value))
                {
                    LOC_LOGI(module, "Saved new WiFi Mode: %s\n", value.c_str());
                    loadWifiConfig();
                    LOC_LOGI(module, "New configuration loaded");
                }
            }
            else if (command == "restart")
            {
                LOC_LOGD(module, "Restarting");
                ESP.restart();
            }
            else if (command == "convertmenus")
            {
                LOC_LOGI(module, "Converting menu structure from old to new format");
                SaveFullFormat();
            }
            else if (command == "reset")
            {
                String file = Serial.readString();
                file.trim();
                ESP_LOGI(module, "Resetting %s.json now\n", file.c_str());
                resetconfig(file);
            }
            else if (command == "setmenus")
            {
                LOC_LOGI(module, "Paste menu(s) here. ~~~ to terminate");
                const char *tempName = "/config/~tempmenu.";
                fs::File tempfile = ftdfs->open(tempName, FILE_WRITE);
                if (tempfile)
                {
                    int v;
                    bool end = false;
                    bool cancel = false;
                    uint8_t endCount = 0;
                    while (!end && !cancel)
                    {
                        if (Serial.available() > 0)
                        {
                            v = Serial.read();
                            if ((char)v == '~')
                            {
                                endCount++;
                            }
                            else if ((char)v == '\032')
                            {
                                LOC_LOGW(module, "Cancelling");
                                cancel = true;
                            }
                            else
                            {
                                while (endCount > 0)
                                {
                                    endCount--;
                                    tempfile.write((uint8_t)'~');
                                }
                                tempfile.write((uint8_t)v);
                            }
                            if (endCount >= 3)
                            {
                                end = true;
                            }
                        }
                    }
                    tempfile.close();
                    if (!cancel)
                    {
                        if (!LoadFullFormat(tempName))
                        {
                            LOC_LOGE(module, "Error loading file. ");
                        }
                        else
                        {
                            SaveFullFormat();
                        }
                    }
                }
            }
            else if (command == "setconfig")
            {
                LOC_LOGI(module, "Paste configuration here. ~~~ to terminate");
                const char *tempName = "/config/~temp.";
                fs::File tempfile = ftdfs->open(tempName, FILE_WRITE);
                if (tempfile)
                {
                    int v;
                    bool end = false;
                    bool cancel = false;
                    uint8_t endCount = 0;
                    while (!end && !cancel)
                    {
                        if (Serial.available() > 0)
                        {
                            v = Serial.read();
                            if ((char)v == '~')
                            {
                                endCount++;
                            }
                            else if ((char)v == '\032')
                            {
                                LOC_LOGW(module, "Cancelling");
                                cancel = true;
                            }
                            else
                            {
                                while (endCount > 0)
                                {
                                    endCount--;
                                    tempfile.write((uint8_t)'~');
                                }
                                tempfile.write((uint8_t)v);
                            }
                            if (endCount >= 3)
                            {
                                end = true;
                            }
                        }
                    }
                    tempfile.close();
                    if (!cancel)
                    {
                        if (loadConfig(tempName, true))
                        {
                            QueueSaving();
                        }
                    }
                }
            }
            else if (command == "showconfig")
            {
                saveConfig(true);
            }
            else if (command == "help")
            {
                LOC_LOGI(module,
                         R"(
====================
Help 
====================
cal : Restarts in screen calibration mode (resistive screens only)
revx : reverse X touch axis
revy : reverse Y touch axis
rot (0-3) : sets the rotation of the screen
invaxis: Flip the X and Y axis
setssid YOURSSID: Sets the WiFi SSID access point to connect to
setpassword YOURPASWORD: Sets the WiFi password 
setwifimode (WIFI_STA|WIFI_AP)
convertmenus : converts from older menu formats to new format
restart : Restarts the system
reset : reset configuration and reboot 
menus : dump the menu structure
setmenus : load menu structure from console.  End with ~~~
showconfig  : dump current configuration
setconfig (config json text) : Upload the configuration file - terminate with ~~~
console : change the system mode to console
configmode : change the system mode to configuration
loglevel (0-5) : increase log details for some activities - warning: more logs will slow down the system
dir : show the content of the file system
memory : show memory usage
)");
            }
            else
            {
                LOC_LOGE(module, "Invalid command [%s]. Use help to get a list", command.c_str());
            }
        }
    }

}