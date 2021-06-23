#pragma once
#include "WString.h"
#include "ESPAsyncWebServer.h"
#include "cJSON.h"
namespace FreeTouchDeck
{
    /**
* @brief This function returns all the files in a given directory in a json 
         formatted string.
*
* @param path String
*
* @return String
*
* @note none
*/
cJSON * handleFileList(const char * path);
String handleAPISList();
/**
* @brief This function returns information about FreeTouchDeck in a json 
         formatted string.
*
* @param none
*
* @return String
*
* @note none
*/
cJSON * AllocGetInfoJson();
/**
* @brief This function handles error.htm template processing.
*
* @param var const String& 
*
* @return String
*
* @note none
*/
String processor(const String &var);
/**
* @brief This function handles JSON file uploads. only menu1.json, menu2.json, menu3.json, menu4.json
*        menu5.json, colors.json, and homescreen.json are accepted.
*
* @param *request AsyncWebServerRequest
* @param filename String
* @param index size_t
* @param *data uint8_t
* @param len size_t
* @param final boolean
*
* @return none
*
* @note none
*/
void handleJSONUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);
/**
* @brief This function handles API JSON file uploads. These are placed in the uploads folder.
*
* @param *request AsyncWebServerRequest
* @param filename String
* @param index size_t
* @param *data uint8_t
* @param len size_t
* @param final boolean
*
* @return none
*
* @note none
*/
void handleAPIUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);
void handlerSetup();

}