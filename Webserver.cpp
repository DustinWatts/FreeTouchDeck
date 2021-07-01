#include "Webserver.h"
#include "globals.hpp"
#include "Storage.h"
#include "ConfigLoad.h"
#include "ConfigHelper.h"
namespace FreeTouchDeck
{
  extern cJSON * MenusToJsonObject(bool withSystem);
  using namespace fs;

  String resultHeader;
  String resultText;
  String resultFiles = "";

  cJSON * handleFileList(const char * path)
  {

    File root = ftdfs->open(path,FILE_READ);
    cJSON * doc = cJSON_CreateArray();
    cJSON * fileEntry = NULL;
    String fileName;
    if (root.isDirectory())
    {
      File file = root.openNextFile();
      while (file)
      {
        fileName = file.name();
        fileName.substring(fileName.lastIndexOf('/'));
        fileEntry= cJSON_CreateObject();
        cJSON_AddStringToObject(fileEntry,"name",fileName.c_str());
        cJSON_AddNumberToObject(fileEntry,"size",file.size());
        cJSON_AddStringToObject(fileEntry,"type",file.isDirectory()?"D":"F");
        cJSON_AddItemToArray(doc,fileEntry);
        file = root.openNextFile();
      }
      file.close();
    }
    root.close();
    return doc;
  }
  bool RespondWithJSON(AsyncWebServerRequest *request, cJSON * doc)
  {
    char * result = AllocPrintJson(doc);
    if (result)
    {
      request->send(200, "application/json", result);
      free(result);
    }
    else
    {
      request->send(500, "Error Generating JSON structure");
    }
  }
  
  cJSON * AllocGetGeneralJson()
  {
    
    cJSON * generalConfig = cJSON_CreateObject();
    float freemem = ftdfs->totalBytes() - ftdfs->usedBytes();
    
    cJSON_AddStringToObject(generalConfig,"Manufacturer",generalconfig.manufacturer);
    cJSON_AddNumberToObject(generalConfig,"LogLevel",int(generalconfig.LogLevel));
    cJSON_AddNumberToObject(generalConfig,"menuButtonColour",generalconfig.menuButtonColour);
    cJSON_AddNumberToObject(generalConfig,"functionButtonColour",generalconfig.functionButtonColour);
    cJSON_AddNumberToObject(generalConfig,"backgroundColour",generalconfig.backgroundColour);
    cJSON_AddNumberToObject(generalConfig,"latchedColour",generalconfig.latchedColour);
    cJSON_AddNumberToObject(generalConfig,"DefaultOutline",generalconfig.DefaultOutline);
    cJSON_AddNumberToObject(generalConfig,"DefaultTextColor",generalconfig.DefaultTextColor);
    cJSON_AddNumberToObject(generalConfig,"DefaultTextSize",generalconfig.DefaultTextSize);
    cJSON_AddNumberToObject(generalConfig,"colscount",generalconfig.colscount);
    cJSON_AddNumberToObject(generalConfig,"rowscount",generalconfig.rowscount);
    cJSON_AddStringToObject(generalConfig,"sleepenable",generalconfig.sleepenable?"Enabled":"Disabled");
    cJSON_AddNumberToObject(generalConfig,"keyDelay",generalconfig.keyDelay);
    cJSON_AddStringToObject(generalConfig,"beep",generalconfig.beep?"Enabled":"Disabled");
    cJSON_AddStringToObject(generalConfig,"flip_touch_axis",generalconfig.flip_touch_axis?"Yes":"No");
    cJSON_AddStringToObject(generalConfig,"reverse_x_touch",generalconfig.reverse_x_touch?"Yes":"No");
    cJSON_AddStringToObject(generalConfig,"reverse_y_touch",generalconfig.reverse_y_touch?"Yes":"No");
    cJSON_AddNumberToObject(generalConfig,"screenrotation",generalconfig.screenrotation);
    cJSON_AddNumberToObject(generalConfig,"helperdelay",generalconfig.helperdelay);
    cJSON_AddNumberToObject(generalConfig,"ledBrightness",generalconfig.ledBrightness);
    
    return generalConfig;

  }

  cJSON * AllocGetInfoJson()
  {

    cJSON * infoDoc = cJSON_CreateArray();
    float freemem = ftdfs->totalBytes() - ftdfs->usedBytes();
    cJSON * element = cJSON_CreateObject();
    cJSON_AddStringToObject(element,"Version",versionnumber);
    cJSON_AddItemToArray(infoDoc,element);
    element = cJSON_CreateObject();
    cJSON_AddNumberToObject(element,"Free Space",freemem / 1000);
    cJSON_AddItemToArray(infoDoc,element);
    element = cJSON_CreateObject();
    cJSON_AddStringToObject(element,"BLE Keyboard Version",BLE_KEYBOARD_VERSION);
    cJSON_AddItemToArray(infoDoc,element);
        element = cJSON_CreateObject();
    cJSON_AddStringToObject(element,"TFT_eSPI Version",TFT_ESPI_VERSION);
    cJSON_AddItemToArray(infoDoc,element);
        element = cJSON_CreateObject();
    cJSON_AddStringToObject(element,"ESP-IDF",esp_get_idf_version());
    cJSON_AddItemToArray(infoDoc,element);
        element = cJSON_CreateObject();
    cJSON_AddStringToObject(element,"WiFi Mode",wificonfig.wifimode);
    cJSON_AddItemToArray(infoDoc,element);

#ifdef touchInterruptPin
    element = cJSON_CreateObject();
    cJSON_AddStringToObject(element,"Sleep",generalconfig.sleepenable?"Enabled":"Disabled");
    cJSON_AddItemToArray(infoDoc,element);
    element = cJSON_CreateObject();
    cJSON_AddNumberToObject(element,"Sleep Timer",generalconfig.sleeptimer);
    cJSON_AddItemToArray(infoDoc,element);

#else
    element = cJSON_CreateObject();
    cJSON_AddStringToObject(element,"Sleep","Disabled");
    cJSON_AddItemToArray(infoDoc,element);
#endif
    return infoDoc;
  }

  String errorCode;
  String errorText;

  String processor(const String &var)
  {
    if (var == "ERROR_CODE")
    {
      return errorCode;
    }
    if (var == "ERROR_TEXT")
    {
      return errorText;
    }
    return String();
  }

  void handleJSONUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
  {
    if (filename != "menu1.json" && filename != "menu2.json" && filename != "menu3.json" && filename != "menu4.json" && filename != "menu5.json" && filename != "colors.json" && filename != "homescreen.json")
    {
      LOC_LOGI(module, "JSON has invalid name: %s\n", filename.c_str());
      errorCode = "102";
      errorText = "JSON file has an invalid name. You can only upload JSON files with the following file names:";
      errorText += "<ul><li>menu1.json</li><li>menu2.json</li><li>menu3.json</li><li>menu4.json</li><li>menu5.json</li>";
      errorText += "<li>colors.json</li><li>homescreen.json</li></ul>";
      request->send((ftdfs->fileSystem), "/error.htm", String(), false, processor);
      return;
    }
    if (!index)
    {
      LOC_LOGI(module, "JSON Upload Start: %s\n", filename.c_str());
      filename = "/config/" + filename; // TODO: Does the config directory need to be hardcoded?

      // Open the file on first call and store the file handle in the request object
      request->_tempFile = ftdfs->stopen(filename, FILE_WRITE);
    }
    if (len)
    {
      // Stream the incoming chunk to the opened file
      request->_tempFile.write(data, len);
    }
    if (final)
    {
      LOC_LOGI(module, "JSON Uploaded: %s\n", filename.c_str());
      // Close the file handle as the upload is now done
      request->_tempFile.close();
      request->send((ftdfs->fileSystem), "/upload.htm");
    }
  }

  void handleAPIUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
  {
    if (!index)
    {
      LOC_LOGI(module, "API file Upload Start: %s\n", filename.c_str());
      filename = "/uploads/" + filename; // TODO: Does the uploads directory need to be hardcoded?

      // Open the file on first call and store the file handle in the request object
      request->_tempFile = ftdfs->stopen(filename, "w");
    }
    if (len)
    {
      // Stream the incoming chunk to the opened file
      request->_tempFile.write(data, len);
    }
    if (final)
    {
      LOC_LOGI(module, "API file Uploaded: %s\n", filename.c_str());
      // Close the file handle as the upload is now done
      request->_tempFile.close();
      request->send((ftdfs->fileSystem), "/upload.htm");
    }
  }

  /* --------------- Checking for free space on storage ---------------- 
Purpose: This checks if the free memory on the storage is bigger then a set threshold
Input  : none
Output : boolean
Note   : none
*/

  bool spaceLeft()
  {
    float minmem = 100000.00; // Always leave 100 kB free pace on storage
    float freeMemory = ftdfs->totalBytes() - ftdfs->usedBytes();
    LOC_LOGI(module, "Free storage: %f bytes\n", freeMemory);
    if (freeMemory < minmem)
    {
      return false;
    }

    return true;
  }
  /**
* @brief This function handles a file upload used by the Webserver 
*
* @param *request
* @param filename String
* @param index size_t
* @param *data uint8_t
* @param len size_t
* @param final boolean
*
* @return none
*
* @note The reason the file is first uploaded and then deleted if there is not enough free space, is that
         if the request is not handled, the ESP32 craches. So we have to accept the upload but
         can delete it.
*/
  void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
  {
    if (!index)
    {
      LOC_LOGI(module, "File Upload Start: %s\n", filename.c_str());
      filename = "/logos/" + filename;
      // Open the file on first call and store the file handle in the request object
      request->_tempFile = ftdfs->stopen(filename, "w");
    }
    if (len)
    {
      // Stream the incoming chunk to the opened file
      request->_tempFile.write(data, len);
    }
    if (final)
    {
      LOC_LOGI(module, "File Uploaded: %s\n", filename.c_str());
      // Close the file handle as the upload is now done
      request->_tempFile.close();

      // If there is not enough space left, we have to delete the recently uploaded file
      if (!spaceLeft())
      {
        LOC_LOGD(module, "Not enough free space left");
        errorCode = "103";
        errorText = "There is not enough free space left to upload data. Please delete unused logos and try again.";
        request->send((ftdfs->fileSystem), "/error.htm", String(), false, processor);

        // Remove the recently uploaded file
        String fileToDelete = "/logos/";
        fileToDelete += filename;
        ftdfs->stremove(fileToDelete);
        LOC_LOGD(module, "File removed to keep enough free space");
        return;
      }
      else
      {
        request->send((ftdfs->fileSystem), "/upload.htm");
      }
    }
  }

  /**
* @brief This function handles delete.htm template processing. 
*
* @param  &var const String
*
* @return String
*
* @note Only need to call this once! This is also where the saving of config files is done.
*/
  String deleteProcessor(const String &var)
  {
    if (var == "RESULT")
    {
      return resultHeader;
    }
    if (var == "TEXT")
    {
      return resultText;
    }
    if (var == "FILES")
    {
      return resultFiles;
    }
    return String();
  }

  /**
* @brief This function adds all the handlers we need to the webserver. 
*
* @param none
*
* @return none
*
* @note Only need to call this once! This is also where the saving of config files is done.
*/
  void handlerSetup()
  {

    webserver.serveStatic("/", ftdfs->fileSystem, "/").setDefaultFile("index.htm");

    //----------- index.htm handler -----------------

    webserver.on("/index.htm", HTTP_POST, [](AsyncWebServerRequest *request)
                 { request->send((ftdfs->fileSystem), "/index.htm"); });
    webserver.on("/index2.htm", HTTP_POST, [](AsyncWebServerRequest *request)
                 { request->send((ftdfs->fileSystem), "/index2.htm"); });

    //----------- saveconfig handler -----------------

    webserver.on("/saveconfig", HTTP_POST, [](AsyncWebServerRequest *request)
                 {
                   if (request->hasParam("save", true))
                   {
                     AsyncWebParameter *p = request->getParam("save", true);
                     String savemode = p->value().c_str();
                     if (savemode == "general")
                     {

                       AsyncWebParameter *value = request->getParam("menubuttoncolor", true);
                       if (value)
                       {
                         generalconfig.menuButtonColour = convertRGB888ToRGB565(convertHTMLtoRGB888(value->value().c_str()));
                       }
                       value = request->getParam("functionbuttoncolor", true);
                       if (value)
                       {
                         generalconfig.functionButtonColour = convertRGB888ToRGB565(convertHTMLtoRGB888(value->value().c_str()));
                       }

                       value = request->getParam("latchcolor", true);
                       if (value)
                       {
                         generalconfig.latchedColour = convertRGB888ToRGB565(convertHTMLtoRGB888(value->value().c_str()));
                       }

                       value = request->getParam("backgroundcolor", true);
                       if (value)
                       {
                         generalconfig.backgroundColour = convertRGB888ToRGB565(convertHTMLtoRGB888(value->value().c_str()));
                       }
                       value = request->getParam("flip_touch_axis", true);
                       if (value)
                       {
                         generalconfig.flip_touch_axis = value->value() == "true";
                       }
                       value = request->getParam("reverse_x_touch", true);
                       if (value)
                       {
                         generalconfig.reverse_x_touch = value->value() == "true";
                       }

                       value = request->getParam("reverse_y_touch", true);
                       if (value)
                       {
                         generalconfig.reverse_y_touch = value->value() == "true";
                       }
                       value = request->getParam("rotation", true);
                       if (value)
                       {
                         uint8_t rot = value->value().toInt();
                         generalconfig.screenrotation = rot >= 0 && rot <= 3 ? rot : generalconfig.screenrotation;
                       }

                       value = request->getParam("sleepenable", true);
                       if (value)
                       {
                         generalconfig.sleepenable = value->value() == "true";
                       }
                       value = request->getParam("beep", true);
                       if (value)
                       {
                         generalconfig.beep = value->value() == "true";
                       }
                       value = request->getParam("sleeptimer", true);
                       if (value)
                       {
                         generalconfig.sleeptimer = value->value().toInt();
                       }
                       value = request->getParam("helperdelay", true);
                       if (value)
                       {
                         generalconfig.helperdelay = value->value().toInt();
                       }

                       value = request->getParam("rowscount", true);
                       if (value)
                       {
                         generalconfig.rowscount = value->value().toInt();
                       }
                       value = request->getParam("colscount", true);
                       if (value)
                       {
                         generalconfig.colscount = value->value().toInt();
                       }
                       saveConfig(false);
                     }
                     else if (savemode == "wifi")
                     {

                       // --- Saving wifi config
                       LOC_LOGI(module, "Saving Wifi Config");

                       ftdfs->remove("/config/wificonfig.json");
                       File file = ftdfs->open("/config/wificonfig.json", "w");
                       if (!file)
                       {
                         LOC_LOGD(module, "Failed to create file");
                         return;
                       }

                      
                      EraseWifiConfig(&wificonfig);
                       AsyncWebParameter *ssid = request->getParam("ssid", true);
                       wificonfig.ssid = ps_strdup(ssid->value().c_str());
                       AsyncWebParameter *password = request->getParam("password", true);
                       wificonfig.password = ps_strdup(password->value().c_str());
                       AsyncWebParameter *wifimode = request->getParam("wifimode", true);
                       wificonfig.wifimode = ps_strdup(wifimode->value().c_str());
                       AsyncWebParameter *hostname = request->getParam("wifihostname", true);
                       wificonfig.hostname = ps_strdup(hostname->value().c_str());
                       AsyncWebParameter *attempts = request->getParam("attempts", true);
                       wificonfig.attempts=attempts->value().toInt();

                       AsyncWebParameter *attemptdelay = request->getParam("attemptdelay", true);
                       wificonfig.attemptdelay=attemptdelay->value().toInt();
                        Commit(&wificonfig);
                     }

                     request->send((ftdfs->fileSystem), "/saveconfig.htm");
                   }
                 });

    //----------- File list handler -----------------

    webserver.on("/list", HTTP_GET, [](AsyncWebServerRequest *request)
                 {
                   if (request->hasParam("dir"))
                   {
                     AsyncWebParameter *p = request->getParam("dir");
                     RespondWithJSON(request, handleFileList(p->value().c_str())) ;
                   }
                 });

    // Configuration return Handlers

    webserver.on("/menus.json", HTTP_GET, [](AsyncWebServerRequest *request){RespondWithJSON(request,MenusToJsonObject(false));});
    webserver.on("/useractions.json", HTTP_GET, [](AsyncWebServerRequest *request){RespondWithJSON(request,UserActionsJson());});
    webserver.on("/keynames.json", HTTP_GET, [](AsyncWebServerRequest *request){RespondWithJSON(request,KeyNamesJson());});
    webserver.on("/info", HTTP_GET, [](AsyncWebServerRequest *request) { RespondWithJSON(request, AllocGetInfoJson()) ; });
    webserver.on("/general.json", HTTP_GET, [](AsyncWebServerRequest *request) { RespondWithJSON(request, AllocGetGeneralJson()) ; });

    //----------- 404 handler -----------------

    webserver.onNotFound([](AsyncWebServerRequest *request)
                         {
                           Serial.printf("NOT_FOUND: ");
                           if (request->method() == HTTP_GET)
                             Serial.printf("GET");
                           else if (request->method() == HTTP_POST)
                             Serial.printf("POST");
                           else if (request->method() == HTTP_DELETE)
                             Serial.printf("DELETE");
                           else if (request->method() == HTTP_PUT)
                             Serial.printf("PUT");
                           else if (request->method() == HTTP_PATCH)
                             Serial.printf("PATCH");
                           else if (request->method() == HTTP_HEAD)
                             Serial.printf("HEAD");
                           else if (request->method() == HTTP_OPTIONS)
                             Serial.printf("OPTIONS");
                           else
                             Serial.printf("UNKNOWN");
                           Serial.printf(" http://%s%s\n", request->host().c_str(), request->url().c_str());

                           if (request->contentLength())
                           {
                             Serial.printf("_CONTENT_TYPE: %s\n", request->contentType().c_str());
                             Serial.printf("_CONTENT_LENGTH: %u\n", request->contentLength());
                           }

                           int headers = request->headers();
                           int i;
                           for (i = 0; i < headers; i++)
                           {
                             AsyncWebHeader *h = request->getHeader(i);
                             Serial.printf("_HEADER[%s]: %s\n", h->name().c_str(), h->value().c_str());
                           }

                           int params = request->params();
                           for (i = 0; i < params; i++)
                           {
                             AsyncWebParameter *p = request->getParam(i);
                             if (p->isFile())
                             {
                               Serial.printf("_FILE[%s]: %s, size: %u\n", p->name().c_str(), p->value().c_str(), p->size());
                             }
                             else if (p->isPost())
                             {
                               Serial.printf("_POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
                             }
                             else
                             {
                               Serial.printf("_GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
                             }
                           }

                           request->send(404);
                         });

    webserver.onFileUpload([](AsyncWebServerRequest *request, const String &filename, size_t index, uint8_t *data, size_t len, bool final)
                           {
                             if (!index)
                               Serial.printf("UploadStart: %s\n", filename.c_str());
                             Serial.printf("%s", (const char *)data);
                             if (final)
                               Serial.printf("UploadEnd: %s (%u)\n", filename.c_str(), index + len);
                           });

    webserver.onRequestBody([](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
                            {
                              if (!index)
                                Serial.printf("BodyStart: %u\n", total);
                              Serial.printf("%s", (const char *)data);
                              if (index + len == total)
                                Serial.printf("BodyEnd: %u\n", total);
                            });

    webserver.on(
        "/upload", HTTP_POST, [](AsyncWebServerRequest *request)
        { request->send((ftdfs->fileSystem), "/upload.htm"); },
        handleUpload);

    webserver.on("/restart", HTTP_POST, [](AsyncWebServerRequest *request)
                 {
                   // First send some text to the browser otherwise an ugly browser error shows up
                   request->send(200, "text/plain", "FreeTouchDeck is restarting...");
                   // Then restart the ESP
                   LOC_LOGD(module, "Restarting");
                   ESP.restart();
                 });

    // ----------------------------- Error Handle ---------------------------------

    webserver.on("/error", HTTP_GET, [](AsyncWebServerRequest *request)
                 {
                   errorCode = "123";
                   errorText = "Nothing went wrong, all is good. This is just a test";
                   request->send((ftdfs->fileSystem), "/error.htm", String(), false, processor);
                 });

    // ----------------------------- Editor Handle ---------------------------------

    webserver.on("/editor", HTTP_POST, [](AsyncWebServerRequest *request)
                 {
                   int params = request->params();
                   int i;
                   int filecount = 0;
                   for (i = 0; i < params; i++)
                   {
                     AsyncWebParameter *p = request->getParam(i);
                     LOC_LOGI(module, "Deleting file: %s\n", p->value().c_str());
                     String filename = "/logos/";
                     filename += p->value().c_str();
                     if (ftdfs->stexists(filename))
                     {
                       ftdfs->stremove(filename);
                     }

                     resultFiles += p->value().c_str();
                     resultFiles += "<br>";
                     filecount++;
                   }
                   if (filecount > 0)
                   {
                     resultHeader = "Succes!";
                   }
                   else
                   {
                     resultHeader = "Fail!";
                   }
                   resultText = String(filecount);
                   request->send((ftdfs->fileSystem), "/editor.htm", String(), false, deleteProcessor);
                   resultFiles = "";
                 });

    // ----------------------------- JSON Download Handle ---------------------------------

    webserver.on("/download", HTTP_GET, [](AsyncWebServerRequest *request)
                 {
                   AsyncWebParameter *p = request->getParam("file");
                   String filerequest = p->value().c_str();
                   LOC_LOGI(module, "Requested file: %s\n", filerequest.c_str());

                   String downloadfile = "/config/" + filerequest;
                   LOC_LOGI(module, "Full path: %s\n", downloadfile.c_str());

                   if (ftdfs->stexists(downloadfile))
                   {
                     LOC_LOGI(module, "Download file %s\n", downloadfile.c_str());
                     request->send((ftdfs->fileSystem), downloadfile, String(), true);
                   }
                   else
                   {
                     LOC_LOGI(module, "Download file %s doesn't exits!\n", downloadfile.c_str());
                   }
                 });

    // ----------------------------- JSON Upload Handle ---------------------------------

    webserver.on(
        "/uploadJSON", HTTP_POST, [](AsyncWebServerRequest *request) {}, handleJSONUpload);
  }
}