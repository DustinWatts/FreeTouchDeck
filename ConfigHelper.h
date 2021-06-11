#pragma once
#include "WString.h"
#include <ESPAsyncWebServer.h> //Async Webserver support header
namespace FreeTouchDeck
{

    typedef struct 
    {
        char *ssid;
        char *password;
        char *wifimode;
        char *hostname;
        uint8_t attempts;
        uint16_t attemptdelay;
    } Wificonfig;
    bool saveWifiSSID(String ssid);
    void stopBT();
    bool startWebServer();
    bool startWifiStation();
    // Start the default AP
    bool startAP(const char *SSID, const char *PASSWORD);
    bool startDefaultAP();
    bool loadWifiConfig();
    bool resetconfig(String file);
    extern AsyncWebServer webserver;
    extern Wificonfig wificonfig;
    /**
* @brief This function stops Bluetooth and connects to the given 
         WiFi network. It the starts mDNS and starts the Async
         Webserver.
*
* @param none
*
* @return none
*
* @note none
*/
    bool ConfigMode();

    /**
* @brief This function allows for saving (updating) the WiFi Password
*
* @param String password
*
* @return boolean True if succeeded. False otherwise.
*
* @note Returns true if successful. To enable the new set password, you must reload the the 
         configuration using loadWifiConfig()
*/
    bool saveWifiPW(String password);
    /**
* @brief This function allows for saving (updating) the WiFi Mode
*
* @param String wifimode "WIFI_STA" of "WIFI_AP"
*
* @return boolean True if succeeded. False otherwise.
*
* @note Returns true if successful. To enable the new set WiFi Mode, you must reload the the 
         configuration using loadWifiConfig()
*/
    bool saveWifiMode(String wifimode);
}