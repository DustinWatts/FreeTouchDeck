namespace FreeTouchDeck
{
  extern char * MenusToJson(bool withSystem);
}
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
String handleFileList(String path)
{

  File root = FILESYSTEM.open(path);
  path = String();
  int filecount = 0;

  String output = "[";
  if (root.isDirectory())
  {
    File file = root.openNextFile();
    while (file)
    {
      if (output != "[")
      {
        output += ',';
      }

      output += "{\"";
      output += filecount;
      output += "\":\"";
      output += String(file.name()).substring(7);
      output += "\"}";
      file = root.openNextFile();
      filecount++;
    }

    file.close();
  }
  output += "]";
  root.close();
  return output;
}

String handleAPISList()
{

  File root = FILESYSTEM.open("/uploads");

  int filecount = 0;

  String output = "[";
  if (root.isDirectory())
  {
    File file = root.openNextFile();
    while (file)
    {
      String filepath = String(file.name()).substring(0,16);
      if(filepath == "/uploads/config_"){

        file = root.openNextFile();
        filecount++;
        
      }
      else
      {
        String filename = String(file.name()).substring(9);
        if (output != "[")
        {
          output += ',';
        }
  
        output += "{\"";
        output += filecount;
        output += "\":\"";
        output += String(file.name()).substring(9);
        output += "\"}";
        file = root.openNextFile();
        filecount++;

      }
    }
    file.close();
  }
  output += "]";
  root.close();
  return output;
}


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
String handleInfo()
{

  float freemem = SPIFFS.totalBytes() - SPIFFS.usedBytes();

  String output = "[";

  output += "{\"";
  output += "Version";
  output += "\":\"";
  output += String(versionnumber);
  output += "\"},";

  output += "{\"";
  output += "Free Space";
  output += "\":\"";
  output += String(freemem / 1000);
  output += " kB\"},";

  output += "{\"";
  output += "BLE Keyboard Version";
  output += "\":\"";
  output += String(BLE_KEYBOARD_VERSION);
  output += "\"},";

  output += "{\"";
  output += "ArduinoJson Version";
  output += "\":\"";
  output += String(ARDUINOJSON_VERSION);
  output += "\"},";

  output += "{\"";
  output += "TFT_eSPI Version";
  output += "\":\"";
  output += String(TFT_ESPI_VERSION);
  output += "\"},";

  output += "{\"";
  output += "ESP-IDF";
  output += "\":\"";
  output += String(esp_get_idf_version());
  output += "\"},";

  output += "{\"";
  output += "WiFi Mode";
  output += "\":\"";
  output += String(wificonfig.wifimode);
  output += "\"},";

#ifdef touchInterruptPin
  output += "{\"";
  output += "Sleep";
  output += "\":\"";
  if (generalconfig.sleepenable)
  {
    output += String("Enabled. ");
    output += String("Timer: ");
    output += String(generalconfig.sleeptimer);
    output += String(" minutes");
    output += "\"}";
  }
  else
  {
    output += String("Disabled");
    output += "\"}";
  }
#else
  output += "{\"";
  output += "Sleep";
  output += "\":\"";
  output += String("Disabled");
  output += "\"}";

#endif

  output += "]";

  return output;
}

String errorCode;
String errorText;

/**
* @brief This function handles error.htm template processing.
*
* @param var const String& 
*
* @return String
*
* @note none
*/
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
void handleJSONUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
{
  if (filename != "menu1.json" && filename != "menu2.json" && filename != "menu3.json" && filename != "menu4.json" && filename != "menu5.json" && filename != "colors.json" && filename != "homescreen.json")
  {
    LOC_LOGI(module,"JSON has invalid name: %s\n", filename.c_str());
    errorCode = "102";
    errorText = "JSON file has an invalid name. You can only upload JSON files with the following file names:";
    errorText += "<ul><li>menu1.json</li><li>menu2.json</li><li>menu3.json</li><li>menu4.json</li><li>menu5.json</li>";
    errorText += "<li>colors.json</li><li>homescreen.json</li></ul>";
    request->send(FILESYSTEM, "/error.htm", String(), false, processor);
    return;
  }
  if (!index)
  {
    LOC_LOGI(module,"JSON Upload Start: %s\n", filename.c_str());
    filename = "/config/" + filename; // TODO: Does the config directory need to be hardcoded?

    // Open the file on first call and store the file handle in the request object
    request->_tempFile = SPIFFS.open(filename, "w");
  }
  if (len)
  {
    // Stream the incoming chunk to the opened file
    request->_tempFile.write(data, len);
  }
  if (final)
  {
    LOC_LOGI(module,"JSON Uploaded: %s\n", filename.c_str());
    // Close the file handle as the upload is now done
    request->_tempFile.close();
    request->send(FILESYSTEM, "/upload.htm");
  }
}

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
void handleAPIUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
{
  if (!index)
  {
    LOC_LOGI(module,"API file Upload Start: %s\n", filename.c_str());
    filename = "/uploads/" + filename; // TODO: Does the uploads directory need to be hardcoded?

    // Open the file on first call and store the file handle in the request object
    request->_tempFile = SPIFFS.open(filename, "w");
  }
  if (len)
  {
    // Stream the incoming chunk to the opened file
    request->_tempFile.write(data, len);
  }
  if (final)
  {
    LOC_LOGI(module,"API file Uploaded: %s\n", filename.c_str());
    // Close the file handle as the upload is now done
    request->_tempFile.close();
    request->send(FILESYSTEM, "/upload.htm");
  }
}

/* --------------- Checking for free space on SPIFFS ---------------- 
Purpose: This checks if the free memory on the SPIFFS is bigger then a set threshold
Input  : none
Output : boolean
Note   : none
*/

bool spaceLeft()
{
  float minmem = 100000.00; // Always leave 100 kB free pace on SPIFFS
  float freeMemory = SPIFFS.totalBytes() - SPIFFS.usedBytes();
  LOC_LOGI(module,"Free memory left: %f bytes\n", freeMemory);
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
    LOC_LOGI(module,"File Upload Start: %s\n", filename.c_str());
    filename = "/logos/" + filename;
    // Open the file on first call and store the file handle in the request object
    request->_tempFile = SPIFFS.open(filename, "w");
  }
  if (len)
  {
    // Stream the incoming chunk to the opened file
    request->_tempFile.write(data, len);
  }
  if (final)
  {
    LOC_LOGI(module,"File Uploaded: %s\n", filename.c_str());
    // Close the file handle as the upload is now done
    request->_tempFile.close();

    // If there is not enough space left, we have to delete the recently uploaded file
    if (!spaceLeft())
    {
      LOC_LOGD(module,"Not enough free space left");
      errorCode = "103";
      errorText = "There is not enough free space left to upload a logo. Please delete unused logos and try again.";
      request->send(FILESYSTEM, "/error.htm", String(), false, processor);

      // Remove the recently uploaded file
      String fileToDelete = "/logos/";
      fileToDelete += filename;
      FILESYSTEM.remove(fileToDelete);
      LOC_LOGD(module,"File removed to keep enough free space");
      return;
    }
    else
    {
      request->send(FILESYSTEM, "/upload.htm");
    }
  }
}

String resultHeader;
String resultText;
String resultFiles = "";

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

  webserver.serveStatic("/", FILESYSTEM, "/").setDefaultFile("index.htm");

  //----------- index.htm handler -----------------

  webserver.on("/index.htm", HTTP_POST, [](AsyncWebServerRequest *request) {
    request->send(FILESYSTEM, "/index.htm");
  });
  webserver.on("/index2.htm", HTTP_POST, [](AsyncWebServerRequest *request) {
    request->send(FILESYSTEM, "/index2.htm");
  });  

  //----------- saveconfig handler -----------------

  webserver.on("/saveconfig", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("save", true))
    {
      AsyncWebParameter *p = request->getParam("save", true);
      String savemode = p->value().c_str();
      if (savemode == "general")
      {

        AsyncWebParameter * value=  request->getParam("menubuttoncolor", true);
        if(value)
        {
          generalconfig.menuButtonColour = convertRGB888ToRGB565(convertHTMLtoRGB888(value->value().c_str()));
        }
        value=  request->getParam("functionbuttoncolor", true);
        if(value)
        {
          generalconfig.functionButtonColour = convertRGB888ToRGB565(convertHTMLtoRGB888(value->value().c_str()));
        }

        value=  request->getParam("latchcolor", true);
        if(value)
        {
          generalconfig.latchedColour = convertRGB888ToRGB565(convertHTMLtoRGB888(value->value().c_str()));
        }

        value=  request->getParam("backgroundcolor", true);
        if(value)
        {
          generalconfig.backgroundColour = convertRGB888ToRGB565(convertHTMLtoRGB888(value->value().c_str()));
        }
        value=  request->getParam("flip_touch_axis", true);
        if(value)
        {
          generalconfig.flip_touch_axis = value->value()=="true";
        }
        value=  request->getParam("reverse_x_touch", true);
        if(value)
        {
          generalconfig.reverse_x_touch = value->value()=="true";
        }        

        value=  request->getParam("reverse_y_touch", true);
        if(value)
        {
          generalconfig.reverse_y_touch = value->value()=="true";
        }
        value=  request->getParam("rotation", true);
        if(value)
        {
          uint8_t rot=value->value().toInt();
          generalconfig.screenrotation = rot>=0 && rot<=3?rot:generalconfig.screenrotation ;
        }                

        value=  request->getParam("sleepenable", true);
        if(value)
        {
          generalconfig.sleepenable = value->value()=="true";
        }
        value=  request->getParam("beep", true);
        if(value)
        {
          generalconfig.beep = value->value()=="true";
        }
        value=  request->getParam("sleeptimer", true);
        if(value)
        {
          generalconfig.sleeptimer = value->value().toInt();
        }
        value=  request->getParam("helperdelay", true);
        if(value)
        {
          generalconfig.helperdelay = value->value().toInt();
        }        

        value=  request->getParam("rowscount", true);
        if(value)
        {
          generalconfig.rowscount = value->value().toInt();
        }     
        value=  request->getParam("colscount", true);
        if(value)
        {
          generalconfig.colscount = value->value().toInt();
        }     
        saveConfig(false);

      }
      else if (savemode == "wifi")
      {

        // --- Saving wifi config
        LOC_LOGI(module,"Saving Wifi Config");

        FILESYSTEM.remove("/config/wificonfig.json");
        File file = FILESYSTEM.open("/config/wificonfig.json", "w");
        if (!file)
        {
          LOC_LOGD(module,"Failed to create file");
          return;
        }

        DynamicJsonDocument doc(384);

        JsonObject wifi = doc.to<JsonObject>();

        AsyncWebParameter *menubuttoncolor = request->getParam("ssid", true);
        wifi["ssid"] = menubuttoncolor->value().c_str();
        AsyncWebParameter *functionbuttoncolor = request->getParam("password", true);
        wifi["password"] = functionbuttoncolor->value().c_str();
        AsyncWebParameter *latchcolor = request->getParam("wifimode", true);
        wifi["wifimode"] = latchcolor->value().c_str();
        AsyncWebParameter *background = request->getParam("wifihostname", true);
        wifi["wifihostname"] = background->value().c_str();

        AsyncWebParameter *attempts = request->getParam("attempts", true);
        String Attempts = attempts->value().c_str();
        wifi["attempts"] = Attempts.toInt();

        AsyncWebParameter *attemptdelay = request->getParam("attemptdelay", true);
        String Attemptdelay = attemptdelay->value().c_str();
        wifi["attemptdelay"] = Attemptdelay.toInt();

        if (serializeJsonPretty(doc, file) == 0)
        {
          LOC_LOGD(module,"Failed to write to file");
        }
        file.close();
        
      }
      else if (savemode == "homescreen")
      {

        // --- Saving Homescreen

        LOC_LOGI(module,"Saving Homescreen");

        FILESYSTEM.remove("/config/homescreen.json");
        File file = FILESYSTEM.open("/config/homescreen.json", "w");
        if (!file)
        {
          LOC_LOGD(module,"Failed to create file");
          return;
        }

        DynamicJsonDocument doc(256);

        JsonObject homescreen = doc.to<JsonObject>();

        AsyncWebParameter *homescreenlogo0 = request->getParam("homescreenlogo0", true);
        homescreen["logo0"] = homescreenlogo0->value().c_str();
        AsyncWebParameter *homescreenlogo1 = request->getParam("homescreenlogo1", true);
        homescreen["logo1"] = homescreenlogo1->value().c_str();
        AsyncWebParameter *homescreenlogo2 = request->getParam("homescreenlogo2", true);
        homescreen["logo2"] = homescreenlogo2->value().c_str();
        AsyncWebParameter *homescreenlogo3 = request->getParam("homescreenlogo3", true);
        homescreen["logo3"] = homescreenlogo3->value().c_str();
        AsyncWebParameter *homescreenlogo4 = request->getParam("homescreenlogo4", true);
        homescreen["logo4"] = homescreenlogo4->value().c_str();
        AsyncWebParameter *homescreenlogo5 = request->getParam("homescreenlogo5", true);
        homescreen["logo5"] = homescreenlogo5->value().c_str();

        if (serializeJsonPretty(doc, file) == 0)
        {
          LOC_LOGD(module,"Failed to write to file");
        }
        file.close();
      }
      else if (savemode == "menu1")
      {

        // --- Save menu 1

        LOC_LOGI(module,"Saving Menu 1");
        FILESYSTEM.remove("/config/menu1.json");
        File file = FILESYSTEM.open("/config/menu1.json", "w");
        if (!file)
        {
          LOC_LOGD(module,"Failed to create menu1.json");
          return;
        }

        DynamicJsonDocument doc(1200);

        JsonObject menu = doc.to<JsonObject>();

        AsyncWebParameter *screen1logo0 = request->getParam("screen1logo0", true);
        menu["logo0"] = screen1logo0->value().c_str();
        AsyncWebParameter *screen1logo1 = request->getParam("screen1logo1", true);
        menu["logo1"] = screen1logo1->value().c_str();
        AsyncWebParameter *screen1logo2 = request->getParam("screen1logo2", true);
        menu["logo2"] = screen1logo2->value().c_str();
        AsyncWebParameter *screen1logo3 = request->getParam("screen1logo3", true);
        menu["logo3"] = screen1logo3->value().c_str();
        AsyncWebParameter *screen1logo4 = request->getParam("screen1logo4", true);
        menu["logo4"] = screen1logo4->value().c_str();

        JsonObject button0 = doc.createNestedObject("button0");

        if (request->hasParam("screen1button0latch", true))
        {
          button0["latch"] = true;
        }
        else
        {
          button0["latch"] = false;
        }

        AsyncWebParameter *screen1latchlogo0 = request->getParam("screen1latchlogo0", true);
        LOC_LOGI(module,"%s",screen1latchlogo0->value().c_str());
        if (strcmp(screen1latchlogo0->value().c_str(), "---") == 0)
        {
          button0["latchlogo"] = "";
        }
        else
        {
          button0["latchlogo"] = screen1latchlogo0->value().c_str();
        }

        JsonArray button0_actionarray = button0.createNestedArray("actionarray");
        AsyncWebParameter *screen1button0action0 = request->getParam("screen1button0action0", true);
        button0_actionarray.add(screen1button0action0->value().c_str());
        AsyncWebParameter *screen1button0action1 = request->getParam("screen1button0action1", true);
        button0_actionarray.add(screen1button0action1->value().c_str());
        AsyncWebParameter *screen1button0action2 = request->getParam("screen1button0action2", true);
        button0_actionarray.add(screen1button0action2->value().c_str());

        JsonArray button0_valuearray = button0.createNestedArray("valuearray");
        AsyncWebParameter *screen1button0value0 = request->getParam("screen1button0value0", true);
        button0_valuearray.add(screen1button0value0->value().c_str());
        AsyncWebParameter *screen1button0value1 = request->getParam("screen1button0value1", true);
        button0_valuearray.add(screen1button0value1->value().c_str());
        AsyncWebParameter *screen1button0value2 = request->getParam("screen1button0value2", true);
        button0_valuearray.add(screen1button0value2->value().c_str());

        JsonObject button1 = doc.createNestedObject("button1");

        if (request->hasParam("screen1button1latch", true))
        {
          button1["latch"] = true;
        }
        else
        {
          button1["latch"] = false;
        }

        AsyncWebParameter *screen1latchlogo1 = request->getParam("screen1latchlogo1", true);
        LOC_LOGI(module,"%s",screen1latchlogo1->value().c_str());
        if (strcmp(screen1latchlogo1->value().c_str(), "---") == 0)
        {
          LOC_LOGI(module,"%s",screen1latchlogo1->value().c_str());
          button1["latchlogo"] = "";
        }
        else
        {
          LOC_LOGI(module,"%s",screen1latchlogo1->value().c_str());
          button1["latchlogo"] = screen1latchlogo1->value().c_str();
        }

        JsonArray button1_actionarray = button1.createNestedArray("actionarray");
        AsyncWebParameter *screen1button1action0 = request->getParam("screen1button1action0", true);
        button1_actionarray.add(screen1button1action0->value().c_str());
        AsyncWebParameter *screen1button1action1 = request->getParam("screen1button1action1", true);
        button1_actionarray.add(screen1button1action1->value().c_str());
        AsyncWebParameter *screen1button1action2 = request->getParam("screen1button1action2", true);
        button1_actionarray.add(screen1button1action2->value().c_str());

        JsonArray button1_valuearray = button1.createNestedArray("valuearray");
        AsyncWebParameter *screen1button1value0 = request->getParam("screen1button1value0", true);
        button1_valuearray.add(screen1button1value0->value().c_str());
        AsyncWebParameter *screen1button1value1 = request->getParam("screen1button1value1", true);
        button1_valuearray.add(screen1button1value1->value().c_str());
        AsyncWebParameter *screen1button1value2 = request->getParam("screen1button1value2", true);
        button1_valuearray.add(screen1button1value2->value().c_str());

        JsonObject button2 = doc.createNestedObject("button2");

        if (request->hasParam("screen1button2latch", true))
        {
          button2["latch"] = true;
        }
        else
        {
          button2["latch"] = false;
        }

        AsyncWebParameter *screen1latchlogo2 = request->getParam("screen1latchlogo2", true);
        LOC_LOGI(module,"%s",screen1latchlogo2->value().c_str());
        if (strcmp(screen1latchlogo2->value().c_str(), "---") == 0)
        {
          button2["latchlogo"] = "";
        }
        else
        {
          button2["latchlogo"] = screen1latchlogo2->value().c_str();
        }

        JsonArray button2_actionarray = button2.createNestedArray("actionarray");
        AsyncWebParameter *screen1button2action0 = request->getParam("screen1button2action0", true);
        button2_actionarray.add(screen1button2action0->value().c_str());
        AsyncWebParameter *screen1button2action1 = request->getParam("screen1button2action1", true);
        button2_actionarray.add(screen1button2action1->value().c_str());
        AsyncWebParameter *screen1button2action2 = request->getParam("screen1button2action2", true);
        button2_actionarray.add(screen1button2action2->value().c_str());

        JsonArray button2_valuearray = button2.createNestedArray("valuearray");
        AsyncWebParameter *screen1button2value0 = request->getParam("screen1button2value0", true);
        button2_valuearray.add(screen1button2value0->value().c_str());
        AsyncWebParameter *screen1button2value1 = request->getParam("screen1button2value1", true);
        button2_valuearray.add(screen1button2value1->value().c_str());
        AsyncWebParameter *screen1button2value2 = request->getParam("screen1button2value2", true);
        button2_valuearray.add(screen1button2value2->value().c_str());

        JsonObject button3 = doc.createNestedObject("button3");

        if (request->hasParam("screen1button3latch", true))
        {
          button3["latch"] = true;
        }
        else
        {
          button3["latch"] = false;
        }

        AsyncWebParameter *screen1latchlogo3 = request->getParam("screen1latchlogo3", true);
        LOC_LOGI(module,"%s",screen1latchlogo3->value().c_str());
        if (strcmp(screen1latchlogo3->value().c_str(), "---") == 0)
        {
          button3["latchlogo"] = "";
        }
        else
        {
          button3["latchlogo"] = screen1latchlogo3->value().c_str();
        }

        JsonArray button3_actionarray = button3.createNestedArray("actionarray");
        AsyncWebParameter *screen1button3action0 = request->getParam("screen1button3action0", true);
        button3_actionarray.add(screen1button3action0->value().c_str());
        AsyncWebParameter *screen1button3action1 = request->getParam("screen1button3action1", true);
        button3_actionarray.add(screen1button3action1->value().c_str());
        AsyncWebParameter *screen1button3action2 = request->getParam("screen1button3action2", true);
        button3_actionarray.add(screen1button3action2->value().c_str());

        JsonArray button3_valuearray = button3.createNestedArray("valuearray");
        AsyncWebParameter *screen1button3value0 = request->getParam("screen1button3value0", true);
        button3_valuearray.add(screen1button3value0->value().c_str());
        AsyncWebParameter *screen1button3value1 = request->getParam("screen1button3value1", true);
        button3_valuearray.add(screen1button3value1->value().c_str());
        AsyncWebParameter *screen1button3value2 = request->getParam("screen1button3value2", true);
        button3_valuearray.add(screen1button3value2->value().c_str());

        JsonObject button4 = doc.createNestedObject("button4");

        if (request->hasParam("screen1button4latch", true))
        {
          button4["latch"] = true;
        }
        else
        {
          button4["latch"] = false;
        }

        AsyncWebParameter *screen1latchlogo4 = request->getParam("screen1latchlogo4", true);
        LOC_LOGI(module,"%s",screen1latchlogo4->value().c_str());
        if (strcmp(screen1latchlogo4->value().c_str(), "---") == 0)
        {
          button4["latchlogo"] = "";
        }
        else
        {
          button4["latchlogo"] = screen1latchlogo4->value().c_str();
        }

        JsonArray button4_actionarray = button4.createNestedArray("actionarray");
        AsyncWebParameter *screen1button4action0 = request->getParam("screen1button4action0", true);
        button4_actionarray.add(screen1button4action0->value().c_str());
        AsyncWebParameter *screen1button4action1 = request->getParam("screen1button4action1", true);
        button4_actionarray.add(screen1button4action1->value().c_str());
        AsyncWebParameter *screen1button4action2 = request->getParam("screen1button4action2", true);
        button4_actionarray.add(screen1button4action2->value().c_str());

        JsonArray button4_valuearray = button4.createNestedArray("valuearray");
        AsyncWebParameter *screen1button4value0 = request->getParam("screen1button4value0", true);
        button4_valuearray.add(screen1button4value0->value().c_str());
        AsyncWebParameter *screen1button4value1 = request->getParam("screen1button4value1", true);
        button4_valuearray.add(screen1button4value1->value().c_str());
        AsyncWebParameter *screen1button4value2 = request->getParam("screen1button4value2", true);
        button4_valuearray.add(screen1button4value2->value().c_str());

        if (serializeJsonPretty(doc, file) == 0)
        {
          LOC_LOGD(module,"Failed to write to file");
        }
        file.close();
      }
      else if (savemode == "menu2")
      {

        // --- Save menu 2

        LOC_LOGI(module,"Saving Menu 2");
        FILESYSTEM.remove("/config/menu2.json");
        File file = FILESYSTEM.open("/config/menu2.json", "w");
        if (!file)
        {
          LOC_LOGD(module,"Failed to create menu2.json");
          return;
        }

        DynamicJsonDocument doc(1200);

        JsonObject menu = doc.to<JsonObject>();

        AsyncWebParameter *screen2logo0 = request->getParam("screen2logo0", true);
        menu["logo0"] = screen2logo0->value().c_str();
        AsyncWebParameter *screen2logo1 = request->getParam("screen2logo1", true);
        menu["logo1"] = screen2logo1->value().c_str();
        AsyncWebParameter *screen2logo2 = request->getParam("screen2logo2", true);
        menu["logo2"] = screen2logo2->value().c_str();
        AsyncWebParameter *screen2logo3 = request->getParam("screen2logo3", true);
        menu["logo3"] = screen2logo3->value().c_str();
        AsyncWebParameter *screen2logo4 = request->getParam("screen2logo4", true);
        menu["logo4"] = screen2logo4->value().c_str();

        JsonObject button0 = doc.createNestedObject("button0");

        if (request->hasParam("screen2button0latch", true))
        {
          button0["latch"] = true;
        }
        else
        {
          button0["latch"] = false;
        }

        AsyncWebParameter *screen2latchlogo0 = request->getParam("screen2latchlogo0", true);
        LOC_LOGI(module,"%s",screen2latchlogo0->value().c_str());
        if (strcmp(screen2latchlogo0->value().c_str(), "---") == 0)
        {
          button0["latchlogo"] = "";
        }
        else
        {
          button0["latchlogo"] = screen2latchlogo0->value().c_str();
        }

        JsonArray button0_actionarray = button0.createNestedArray("actionarray");
        AsyncWebParameter *screen2button0action0 = request->getParam("screen2button0action0", true);
        button0_actionarray.add(screen2button0action0->value().c_str());
        AsyncWebParameter *screen2button0action1 = request->getParam("screen2button0action1", true);
        button0_actionarray.add(screen2button0action1->value().c_str());
        AsyncWebParameter *screen2button0action2 = request->getParam("screen2button0action2", true);
        button0_actionarray.add(screen2button0action2->value().c_str());

        JsonArray button0_valuearray = button0.createNestedArray("valuearray");
        AsyncWebParameter *screen2button0value0 = request->getParam("screen2button0value0", true);
        button0_valuearray.add(screen2button0value0->value().c_str());
        AsyncWebParameter *screen2button0value1 = request->getParam("screen2button0value1", true);
        button0_valuearray.add(screen2button0value1->value().c_str());
        AsyncWebParameter *screen2button0value2 = request->getParam("screen2button0value2", true);
        button0_valuearray.add(screen2button0value2->value().c_str());

        JsonObject button1 = doc.createNestedObject("button1");

        if (request->hasParam("screen2button1latch", true))
        {
          button1["latch"] = true;
        }
        else
        {
          button1["latch"] = false;
        }

        AsyncWebParameter *screen2latchlogo1 = request->getParam("screen2latchlogo1", true);
        LOC_LOGI(module,"%s",screen2latchlogo1->value().c_str());
        if (strcmp(screen2latchlogo1->value().c_str(), "---") == 0)
        {
          button1["latchlogo"] = "";
        }
        else
        {
          button1["latchlogo"] = screen2latchlogo1->value().c_str();
        }

        JsonArray button1_actionarray = button1.createNestedArray("actionarray");
        AsyncWebParameter *screen2button1action0 = request->getParam("screen2button1action0", true);
        button1_actionarray.add(screen2button1action0->value().c_str());
        AsyncWebParameter *screen2button1action1 = request->getParam("screen2button1action1", true);
        button1_actionarray.add(screen2button1action1->value().c_str());
        AsyncWebParameter *screen2button1action2 = request->getParam("screen2button1action2", true);
        button1_actionarray.add(screen2button1action2->value().c_str());

        JsonArray button1_valuearray = button1.createNestedArray("valuearray");
        AsyncWebParameter *screen2button1value0 = request->getParam("screen2button1value0", true);
        button1_valuearray.add(screen2button1value0->value().c_str());
        AsyncWebParameter *screen2button1value1 = request->getParam("screen2button1value1", true);
        button1_valuearray.add(screen2button1value1->value().c_str());
        AsyncWebParameter *screen2button1value2 = request->getParam("screen2button1value2", true);
        button1_valuearray.add(screen2button1value2->value().c_str());

        JsonObject button2 = doc.createNestedObject("button2");

        if (request->hasParam("screen2button2latch", true))
        {
          button2["latch"] = true;
        }
        else
        {
          button2["latch"] = false;
        }

        AsyncWebParameter *screen2latchlogo2 = request->getParam("screen2latchlogo2", true);
        LOC_LOGI(module,"%s",screen2latchlogo2->value().c_str());
        if (strcmp(screen2latchlogo2->value().c_str(), "---") == 0)
        {
          button2["latchlogo"] = "";
        }
        else
        {
          button2["latchlogo"] = screen2latchlogo2->value().c_str();
        }

        JsonArray button2_actionarray = button2.createNestedArray("actionarray");
        AsyncWebParameter *screen2button2action0 = request->getParam("screen2button2action0", true);
        button2_actionarray.add(screen2button2action0->value().c_str());
        AsyncWebParameter *screen2button2action1 = request->getParam("screen2button2action1", true);
        button2_actionarray.add(screen2button2action1->value().c_str());
        AsyncWebParameter *screen2button2action2 = request->getParam("screen2button2action2", true);
        button2_actionarray.add(screen2button2action2->value().c_str());

        JsonArray button2_valuearray = button2.createNestedArray("valuearray");
        AsyncWebParameter *screen2button2value0 = request->getParam("screen2button2value0", true);
        button2_valuearray.add(screen2button2value0->value().c_str());
        AsyncWebParameter *screen2button2value1 = request->getParam("screen2button2value1", true);
        button2_valuearray.add(screen2button2value1->value().c_str());
        AsyncWebParameter *screen2button2value2 = request->getParam("screen2button2value2", true);
        button2_valuearray.add(screen2button2value2->value().c_str());

        JsonObject button3 = doc.createNestedObject("button3");

        if (request->hasParam("screen2button3latch", true))
        {
          button3["latch"] = true;
        }
        else
        {
          button3["latch"] = false;
        }

        AsyncWebParameter *screen2latchlogo3 = request->getParam("screen2latchlogo3", true);
        LOC_LOGI(module,"%s",screen2latchlogo3->value().c_str());
        if (strcmp(screen2latchlogo3->value().c_str(), "---") == 0)
        {
          button3["latchlogo"] = "";
        }
        else
        {
          button3["latchlogo"] = screen2latchlogo3->value().c_str();
        }

        JsonArray button3_actionarray = button3.createNestedArray("actionarray");
        AsyncWebParameter *screen2button3action0 = request->getParam("screen2button3action0", true);
        button3_actionarray.add(screen2button3action0->value().c_str());
        AsyncWebParameter *screen2button3action1 = request->getParam("screen2button3action1", true);
        button3_actionarray.add(screen2button3action1->value().c_str());
        AsyncWebParameter *screen2button3action2 = request->getParam("screen2button3action2", true);
        button3_actionarray.add(screen2button3action2->value().c_str());

        JsonArray button3_valuearray = button3.createNestedArray("valuearray");
        AsyncWebParameter *screen2button3value0 = request->getParam("screen2button3value0", true);
        button3_valuearray.add(screen2button3value0->value().c_str());
        AsyncWebParameter *screen2button3value1 = request->getParam("screen2button3value1", true);
        button3_valuearray.add(screen2button3value1->value().c_str());
        AsyncWebParameter *screen2button3value2 = request->getParam("screen2button3value2", true);
        button3_valuearray.add(screen2button3value2->value().c_str());

        JsonObject button4 = doc.createNestedObject("button4");

        if (request->hasParam("screen2button4latch", true))
        {
          button4["latch"] = true;
        }
        else
        {
          button4["latch"] = false;
        }

        AsyncWebParameter *screen2latchlogo4 = request->getParam("screen2latchlogo4", true);
        LOC_LOGI(module,"%s",screen2latchlogo4->value().c_str());
        if (strcmp(screen2latchlogo4->value().c_str(), "---") == 0)
        {
          button4["latchlogo"] = "";
        }
        else
        {
          button4["latchlogo"] = screen2latchlogo4->value().c_str();
        }

        JsonArray button4_actionarray = button4.createNestedArray("actionarray");
        AsyncWebParameter *screen2button4action0 = request->getParam("screen2button4action0", true);
        button4_actionarray.add(screen2button4action0->value().c_str());
        AsyncWebParameter *screen2button4action1 = request->getParam("screen2button4action1", true);
        button4_actionarray.add(screen2button4action1->value().c_str());
        AsyncWebParameter *screen2button4action2 = request->getParam("screen2button4action2", true);
        button4_actionarray.add(screen2button4action2->value().c_str());

        JsonArray button4_valuearray = button4.createNestedArray("valuearray");
        AsyncWebParameter *screen2button4value0 = request->getParam("screen2button4value0", true);
        button4_valuearray.add(screen2button4value0->value().c_str());
        AsyncWebParameter *screen2button4value1 = request->getParam("screen2button4value1", true);
        button4_valuearray.add(screen2button4value1->value().c_str());
        AsyncWebParameter *screen2button4value2 = request->getParam("screen2button4value2", true);
        button4_valuearray.add(screen2button4value2->value().c_str());

        if (serializeJsonPretty(doc, file) == 0)
        {
          LOC_LOGD(module,"Failed to write to file");
        }
        file.close();
      }
      else if (savemode == "menu3")
      {

        // --- Save menu 3

        LOC_LOGI(module,"Saving Menu 3");
        FILESYSTEM.remove("/config/menu3.json");
        File file = FILESYSTEM.open("/config/menu3.json", "w");
        if (!file)
        {
          LOC_LOGD(module,"Failed to create menu3.json");
          return;
        }

        DynamicJsonDocument doc(1200);

        JsonObject menu = doc.to<JsonObject>();

        AsyncWebParameter *screen3logo0 = request->getParam("screen3logo0", true);
        menu["logo0"] = screen3logo0->value().c_str();
        AsyncWebParameter *screen3logo1 = request->getParam("screen3logo1", true);
        menu["logo1"] = screen3logo1->value().c_str();
        AsyncWebParameter *screen3logo2 = request->getParam("screen3logo2", true);
        menu["logo2"] = screen3logo2->value().c_str();
        AsyncWebParameter *screen3logo3 = request->getParam("screen3logo3", true);
        menu["logo3"] = screen3logo3->value().c_str();
        AsyncWebParameter *screen3logo4 = request->getParam("screen3logo4", true);
        menu["logo4"] = screen3logo4->value().c_str();

        JsonObject button0 = doc.createNestedObject("button0");

        if (request->hasParam("screen3button0latch", true))
        {
          button0["latch"] = true;
        }
        else
        {
          button0["latch"] = false;
        }

        AsyncWebParameter *screen3latchlogo0 = request->getParam("screen3latchlogo0", true);
        LOC_LOGI(module,"%s",screen3latchlogo0->value().c_str());
        if (strcmp(screen3latchlogo0->value().c_str(), "---") == 0)
        {
          button0["latchlogo"] = "";
        }
        else
        {
          button0["latchlogo"] = screen3latchlogo0->value().c_str();
        }

        JsonArray button0_actionarray = button0.createNestedArray("actionarray");
        AsyncWebParameter *screen3button0action0 = request->getParam("screen3button0action0", true);
        button0_actionarray.add(screen3button0action0->value().c_str());
        AsyncWebParameter *screen3button0action1 = request->getParam("screen3button0action1", true);
        button0_actionarray.add(screen3button0action1->value().c_str());
        AsyncWebParameter *screen3button0action2 = request->getParam("screen3button0action2", true);
        button0_actionarray.add(screen3button0action2->value().c_str());

        JsonArray button0_valuearray = button0.createNestedArray("valuearray");
        AsyncWebParameter *screen3button0value0 = request->getParam("screen3button0value0", true);
        button0_valuearray.add(screen3button0value0->value().c_str());
        AsyncWebParameter *screen3button0value1 = request->getParam("screen3button0value1", true);
        button0_valuearray.add(screen3button0value1->value().c_str());
        AsyncWebParameter *screen3button0value2 = request->getParam("screen3button0value2", true);
        button0_valuearray.add(screen3button0value2->value().c_str());

        JsonObject button1 = doc.createNestedObject("button1");

        if (request->hasParam("screen3button1latch", true))
        {
          button1["latch"] = true;
        }
        else
        {
          button1["latch"] = false;
        }

        AsyncWebParameter *screen3latchlogo1 = request->getParam("screen3latchlogo1", true);
        LOC_LOGI(module,"%s",screen3latchlogo1->value().c_str());
        if (strcmp(screen3latchlogo1->value().c_str(), "---") == 0)
        {
          button1["latchlogo"] = "";
        }
        else
        {
          button1["latchlogo"] = screen3latchlogo1->value().c_str();
        }

        JsonArray button1_actionarray = button1.createNestedArray("actionarray");
        AsyncWebParameter *screen3button1action0 = request->getParam("screen3button1action0", true);
        button1_actionarray.add(screen3button1action0->value().c_str());
        AsyncWebParameter *screen3button1action1 = request->getParam("screen3button1action1", true);
        button1_actionarray.add(screen3button1action1->value().c_str());
        AsyncWebParameter *screen3button1action2 = request->getParam("screen3button1action2", true);
        button1_actionarray.add(screen3button1action2->value().c_str());

        JsonArray button1_valuearray = button1.createNestedArray("valuearray");
        AsyncWebParameter *screen3button1value0 = request->getParam("screen3button1value0", true);
        button1_valuearray.add(screen3button1value0->value().c_str());
        AsyncWebParameter *screen3button1value1 = request->getParam("screen3button1value1", true);
        button1_valuearray.add(screen3button1value1->value().c_str());
        AsyncWebParameter *screen3button1value2 = request->getParam("screen3button1value2", true);
        button1_valuearray.add(screen3button1value2->value().c_str());

        JsonObject button2 = doc.createNestedObject("button2");

        if (request->hasParam("screen3button2latch", true))
        {
          button2["latch"] = true;
        }
        else
        {
          button2["latch"] = false;
        }

        AsyncWebParameter *screen3latchlogo2 = request->getParam("screen3latchlogo2", true);
        LOC_LOGI(module,"%s",screen3latchlogo2->value().c_str());
        if (strcmp(screen3latchlogo2->value().c_str(), "---") == 0)
        {
          button2["latchlogo"] = "";
        }
        else
        {
          button2["latchlogo"] = screen3latchlogo2->value().c_str();
        }

        JsonArray button2_actionarray = button2.createNestedArray("actionarray");
        AsyncWebParameter *screen3button2action0 = request->getParam("screen3button2action0", true);
        button2_actionarray.add(screen3button2action0->value().c_str());
        AsyncWebParameter *screen3button2action1 = request->getParam("screen3button2action1", true);
        button2_actionarray.add(screen3button2action1->value().c_str());
        AsyncWebParameter *screen3button2action2 = request->getParam("screen3button2action2", true);
        button2_actionarray.add(screen3button2action2->value().c_str());

        JsonArray button2_valuearray = button2.createNestedArray("valuearray");
        AsyncWebParameter *screen3button2value0 = request->getParam("screen3button2value0", true);
        button2_valuearray.add(screen3button2value0->value().c_str());
        AsyncWebParameter *screen3button2value1 = request->getParam("screen3button2value1", true);
        button2_valuearray.add(screen3button2value1->value().c_str());
        AsyncWebParameter *screen3button2value2 = request->getParam("screen3button2value2", true);
        button2_valuearray.add(screen3button2value2->value().c_str());

        JsonObject button3 = doc.createNestedObject("button3");

        if (request->hasParam("screen3button3latch", true))
        {
          button3["latch"] = true;
        }
        else
        {
          button3["latch"] = false;
        }

        AsyncWebParameter *screen3latchlogo3 = request->getParam("screen3latchlogo3", true);
        LOC_LOGI(module,"%s",screen3latchlogo3->value().c_str());
        if (strcmp(screen3latchlogo3->value().c_str(), "---") == 0)
        {
          button3["latchlogo"] = "";
        }
        else
        {
          button3["latchlogo"] = screen3latchlogo3->value().c_str();
        }

        JsonArray button3_actionarray = button3.createNestedArray("actionarray");
        AsyncWebParameter *screen3button3action0 = request->getParam("screen3button3action0", true);
        button3_actionarray.add(screen3button3action0->value().c_str());
        AsyncWebParameter *screen3button3action1 = request->getParam("screen3button3action1", true);
        button3_actionarray.add(screen3button3action1->value().c_str());
        AsyncWebParameter *screen3button3action2 = request->getParam("screen3button3action2", true);
        button3_actionarray.add(screen3button3action2->value().c_str());

        JsonArray button3_valuearray = button3.createNestedArray("valuearray");
        AsyncWebParameter *screen3button3value0 = request->getParam("screen3button3value0", true);
        button3_valuearray.add(screen3button3value0->value().c_str());
        AsyncWebParameter *screen3button3value1 = request->getParam("screen3button3value1", true);
        button3_valuearray.add(screen3button3value1->value().c_str());
        AsyncWebParameter *screen3button3value2 = request->getParam("screen3button3value2", true);
        button3_valuearray.add(screen3button3value2->value().c_str());

        JsonObject button4 = doc.createNestedObject("button4");

        if (request->hasParam("screen3button4latch", true))
        {
          button4["latch"] = true;
        }
        else
        {
          button4["latch"] = false;
        }

        AsyncWebParameter *screen3latchlogo4 = request->getParam("screen3latchlogo4", true);
        LOC_LOGI(module,"%s",screen3latchlogo4->value().c_str());
        if (strcmp(screen3latchlogo4->value().c_str(), "---") == 0)
        {
          button4["latchlogo"] = "";
        }
        else
        {
          button4["latchlogo"] = screen3latchlogo4->value().c_str();
        }

        JsonArray button4_actionarray = button4.createNestedArray("actionarray");
        AsyncWebParameter *screen3button4action0 = request->getParam("screen3button4action0", true);
        button4_actionarray.add(screen3button4action0->value().c_str());
        AsyncWebParameter *screen3button4action1 = request->getParam("screen3button4action1", true);
        button4_actionarray.add(screen3button4action1->value().c_str());
        AsyncWebParameter *screen3button4action2 = request->getParam("screen3button4action2", true);
        button4_actionarray.add(screen3button4action2->value().c_str());

        JsonArray button4_valuearray = button4.createNestedArray("valuearray");
        AsyncWebParameter *screen3button4value0 = request->getParam("screen3button4value0", true);
        button4_valuearray.add(screen3button4value0->value().c_str());
        AsyncWebParameter *screen3button4value1 = request->getParam("screen3button4value1", true);
        button4_valuearray.add(screen3button4value1->value().c_str());
        AsyncWebParameter *screen3button4value2 = request->getParam("screen3button4value2", true);
        button4_valuearray.add(screen3button4value2->value().c_str());

        if (serializeJsonPretty(doc, file) == 0)
        {
          LOC_LOGD(module,"Failed to write to file");
        }
        file.close();
      }
      else if (savemode == "menu4")
      {

        // --- Save menu 4

        LOC_LOGI(module,"Saving Menu 4");
        FILESYSTEM.remove("/config/menu4.json");
        File file = FILESYSTEM.open("/config/menu4.json", "w");
        if (!file)
        {
          LOC_LOGD(module,"Failed to create menu3.json");
          return;
        }

        DynamicJsonDocument doc(1200);

        JsonObject menu = doc.to<JsonObject>();

        AsyncWebParameter *screen4logo0 = request->getParam("screen4logo0", true);
        menu["logo0"] = screen4logo0->value().c_str();
        AsyncWebParameter *screen4logo1 = request->getParam("screen4logo1", true);
        menu["logo1"] = screen4logo1->value().c_str();
        AsyncWebParameter *screen4logo2 = request->getParam("screen4logo2", true);
        menu["logo2"] = screen4logo2->value().c_str();
        AsyncWebParameter *screen4logo3 = request->getParam("screen4logo3", true);
        menu["logo3"] = screen4logo3->value().c_str();
        AsyncWebParameter *screen4logo4 = request->getParam("screen4logo4", true);
        menu["logo4"] = screen4logo4->value().c_str();

        JsonObject button0 = doc.createNestedObject("button0");

        if (request->hasParam("screen4button0latch", true))
        {
          button0["latch"] = true;
        }
        else
        {
          button0["latch"] = false;
        }

        AsyncWebParameter *screen4latchlogo0 = request->getParam("screen4latchlogo0", true);
        LOC_LOGI(module,"%s",screen4latchlogo0->value().c_str());
        if (strcmp(screen4latchlogo0->value().c_str(), "---") == 0)
        {
          button0["latchlogo"] = "";
        }
        else
        {
          button0["latchlogo"] = screen4latchlogo0->value().c_str();
        }

        JsonArray button0_actionarray = button0.createNestedArray("actionarray");
        AsyncWebParameter *screen4button0action0 = request->getParam("screen4button0action0", true);
        button0_actionarray.add(screen4button0action0->value().c_str());
        AsyncWebParameter *screen4button0action1 = request->getParam("screen4button0action1", true);
        button0_actionarray.add(screen4button0action1->value().c_str());
        AsyncWebParameter *screen4button0action2 = request->getParam("screen4button0action2", true);
        button0_actionarray.add(screen4button0action2->value().c_str());

        JsonArray button0_valuearray = button0.createNestedArray("valuearray");
        AsyncWebParameter *screen4button0value0 = request->getParam("screen4button0value0", true);
        button0_valuearray.add(screen4button0value0->value().c_str());
        AsyncWebParameter *screen4button0value1 = request->getParam("screen4button0value1", true);
        button0_valuearray.add(screen4button0value1->value().c_str());
        AsyncWebParameter *screen4button0value2 = request->getParam("screen4button0value2", true);
        button0_valuearray.add(screen4button0value2->value().c_str());

        JsonObject button1 = doc.createNestedObject("button1");

        if (request->hasParam("screen4button1latch", true))
        {
          button1["latch"] = true;
        }
        else
        {
          button1["latch"] = false;
        }

        AsyncWebParameter *screen4latchlogo1 = request->getParam("screen4latchlogo1", true);
        LOC_LOGI(module,"%s",screen4latchlogo1->value().c_str());
        if (strcmp(screen4latchlogo1->value().c_str(), "---") == 0)
        {
          button1["latchlogo"] = "";
        }
        else
        {
          button1["latchlogo"] = screen4latchlogo1->value().c_str();
        }

        JsonArray button1_actionarray = button1.createNestedArray("actionarray");
        AsyncWebParameter *screen4button1action0 = request->getParam("screen4button1action0", true);
        button1_actionarray.add(screen4button1action0->value().c_str());
        AsyncWebParameter *screen4button1action1 = request->getParam("screen4button1action1", true);
        button1_actionarray.add(screen4button1action1->value().c_str());
        AsyncWebParameter *screen4button1action2 = request->getParam("screen4button1action2", true);
        button1_actionarray.add(screen4button1action2->value().c_str());

        JsonArray button1_valuearray = button1.createNestedArray("valuearray");
        AsyncWebParameter *screen4button1value0 = request->getParam("screen4button1value0", true);
        button1_valuearray.add(screen4button1value0->value().c_str());
        AsyncWebParameter *screen4button1value1 = request->getParam("screen4button1value1", true);
        button1_valuearray.add(screen4button1value1->value().c_str());
        AsyncWebParameter *screen4button1value2 = request->getParam("screen4button1value2", true);
        button1_valuearray.add(screen4button1value2->value().c_str());

        JsonObject button2 = doc.createNestedObject("button2");

        if (request->hasParam("screen4button2latch", true))
        {
          button2["latch"] = true;
        }
        else
        {
          button2["latch"] = false;
        }

        AsyncWebParameter *screen4latchlogo2 = request->getParam("screen4latchlogo2", true);
        LOC_LOGI(module,"%s",screen4latchlogo2->value().c_str());
        if (strcmp(screen4latchlogo2->value().c_str(), "---") == 0)
        {
          button2["latchlogo"] = "";
        }
        else
        {
          button2["latchlogo"] = screen4latchlogo2->value().c_str();
        }

        JsonArray button2_actionarray = button2.createNestedArray("actionarray");
        AsyncWebParameter *screen4button2action0 = request->getParam("screen4button2action0", true);
        button2_actionarray.add(screen4button2action0->value().c_str());
        AsyncWebParameter *screen4button2action1 = request->getParam("screen4button2action1", true);
        button2_actionarray.add(screen4button2action1->value().c_str());
        AsyncWebParameter *screen4button2action2 = request->getParam("screen4button2action2", true);
        button2_actionarray.add(screen4button2action2->value().c_str());

        JsonArray button2_valuearray = button2.createNestedArray("valuearray");
        AsyncWebParameter *screen4button2value0 = request->getParam("screen4button2value0", true);
        button2_valuearray.add(screen4button2value0->value().c_str());
        AsyncWebParameter *screen4button2value1 = request->getParam("screen4button2value1", true);
        button2_valuearray.add(screen4button2value1->value().c_str());
        AsyncWebParameter *screen4button2value2 = request->getParam("screen4button2value2", true);
        button2_valuearray.add(screen4button2value2->value().c_str());

        JsonObject button3 = doc.createNestedObject("button3");

        if (request->hasParam("screen4button3latch", true))
        {
          button3["latch"] = true;
        }
        else
        {
          button3["latch"] = false;
        }

        AsyncWebParameter *screen4latchlogo3 = request->getParam("screen4latchlogo3", true);
        LOC_LOGI(module,"%s",screen4latchlogo3->value().c_str());
        if (strcmp(screen4latchlogo3->value().c_str(), "---") == 0)
        {
          button3["latchlogo"] = "";
        }
        else
        {
          button3["latchlogo"] = screen4latchlogo3->value().c_str();
        }

        JsonArray button3_actionarray = button3.createNestedArray("actionarray");
        AsyncWebParameter *screen4button3action0 = request->getParam("screen4button3action0", true);
        button3_actionarray.add(screen4button3action0->value().c_str());
        AsyncWebParameter *screen4button3action1 = request->getParam("screen4button3action1", true);
        button3_actionarray.add(screen4button3action1->value().c_str());
        AsyncWebParameter *screen4button3action2 = request->getParam("screen4button3action2", true);
        button3_actionarray.add(screen4button3action2->value().c_str());

        JsonArray button3_valuearray = button3.createNestedArray("valuearray");
        AsyncWebParameter *screen4button3value0 = request->getParam("screen4button3value0", true);
        button3_valuearray.add(screen4button3value0->value().c_str());
        AsyncWebParameter *screen4button3value1 = request->getParam("screen4button3value1", true);
        button3_valuearray.add(screen4button3value1->value().c_str());
        AsyncWebParameter *screen4button3value2 = request->getParam("screen4button3value2", true);
        button3_valuearray.add(screen4button3value2->value().c_str());

        JsonObject button4 = doc.createNestedObject("button4");

        if (request->hasParam("screen4button4latch", true))
        {
          button4["latch"] = true;
        }
        else
        {
          button4["latch"] = false;
        }

        AsyncWebParameter *screen4latchlogo4 = request->getParam("screen4latchlogo4", true);
        LOC_LOGI(module,"%s",screen4latchlogo4->value().c_str());
        if (strcmp(screen4latchlogo4->value().c_str(), "---") == 0)
        {
          button4["latchlogo"] = "";
        }
        else
        {
          button4["latchlogo"] = screen4latchlogo4->value().c_str();
        }

        JsonArray button4_actionarray = button4.createNestedArray("actionarray");
        AsyncWebParameter *screen4button4action0 = request->getParam("screen4button4action0", true);
        button4_actionarray.add(screen4button4action0->value().c_str());
        AsyncWebParameter *screen4button4action1 = request->getParam("screen4button4action1", true);
        button4_actionarray.add(screen4button4action1->value().c_str());
        AsyncWebParameter *screen4button4action2 = request->getParam("screen4button4action2", true);
        button4_actionarray.add(screen4button4action2->value().c_str());

        JsonArray button4_valuearray = button4.createNestedArray("valuearray");
        AsyncWebParameter *screen4button4value0 = request->getParam("screen4button4value0", true);
        button4_valuearray.add(screen4button4value0->value().c_str());
        AsyncWebParameter *screen4button4value1 = request->getParam("screen4button4value1", true);
        button4_valuearray.add(screen4button4value1->value().c_str());
        AsyncWebParameter *screen4button4value2 = request->getParam("screen4button4value2", true);
        button4_valuearray.add(screen4button4value2->value().c_str());

        if (serializeJsonPretty(doc, file) == 0)
        {
          LOC_LOGD(module,"Failed to write to file");
        }
        file.close();
      }
      else if (savemode == "menu5")
      {

        // --- Save menu 5

        LOC_LOGI(module,"Saving Menu 5");
        FILESYSTEM.remove("/config/menu5.json");
        File file = FILESYSTEM.open("/config/menu5.json", "w");
        if (!file)
        {
          LOC_LOGD(module,"Failed to create menu5.json");
          return;
        }

        DynamicJsonDocument doc(1200);

        JsonObject menu = doc.to<JsonObject>();

        AsyncWebParameter *screen5logo0 = request->getParam("screen5logo0", true);
        menu["logo0"] = screen5logo0->value().c_str();
        AsyncWebParameter *screen5logo1 = request->getParam("screen5logo1", true);
        menu["logo1"] = screen5logo1->value().c_str();
        AsyncWebParameter *screen5logo2 = request->getParam("screen5logo2", true);
        menu["logo2"] = screen5logo2->value().c_str();
        AsyncWebParameter *screen5logo3 = request->getParam("screen5logo3", true);
        menu["logo3"] = screen5logo3->value().c_str();
        AsyncWebParameter *screen5logo4 = request->getParam("screen5logo4", true);
        menu["logo4"] = screen5logo4->value().c_str();

        JsonObject button0 = doc.createNestedObject("button0");

        if (request->hasParam("screen5button0latch", true))
        {
          button0["latch"] = true;
        }
        else
        {
          button0["latch"] = false;
        }

        AsyncWebParameter *screen5latchlogo0 = request->getParam("screen5latchlogo0", true);
        LOC_LOGI(module,"%s",screen5latchlogo0->value().c_str());
        if (strcmp(screen5latchlogo0->value().c_str(), "---") == 0)
        {
          button0["latchlogo"] = "";
        }
        else
        {
          button0["latchlogo"] = screen5latchlogo0->value().c_str();
        }

        JsonArray button0_actionarray = button0.createNestedArray("actionarray");
        AsyncWebParameter *screen5button0action0 = request->getParam("screen5button0action0", true);
        button0_actionarray.add(screen5button0action0->value().c_str());
        AsyncWebParameter *screen5button0action1 = request->getParam("screen5button0action1", true);
        button0_actionarray.add(screen5button0action1->value().c_str());
        AsyncWebParameter *screen5button0action2 = request->getParam("screen5button0action2", true);
        button0_actionarray.add(screen5button0action2->value().c_str());

        JsonArray button0_valuearray = button0.createNestedArray("valuearray");
        AsyncWebParameter *screen5button0value0 = request->getParam("screen5button0value0", true);
        button0_valuearray.add(screen5button0value0->value().c_str());
        AsyncWebParameter *screen5button0value1 = request->getParam("screen5button0value1", true);
        button0_valuearray.add(screen5button0value1->value().c_str());
        AsyncWebParameter *screen5button0value2 = request->getParam("screen5button0value2", true);
        button0_valuearray.add(screen5button0value2->value().c_str());

        JsonObject button1 = doc.createNestedObject("button1");

        if (request->hasParam("screen5button1latch", true))
        {
          button1["latch"] = true;
        }
        else
        {
          button1["latch"] = false;
        }

        AsyncWebParameter *screen5latchlogo1 = request->getParam("screen5latchlogo1", true);
        LOC_LOGI(module,"%s",screen5latchlogo1->value().c_str());
        if (strcmp(screen5latchlogo1->value().c_str(), "---") == 0)
        {
          button1["latchlogo"] = "";
        }
        else
        {
          button1["latchlogo"] = screen5latchlogo1->value().c_str();
        }

        JsonArray button1_actionarray = button1.createNestedArray("actionarray");
        AsyncWebParameter *screen5button1action0 = request->getParam("screen5button1action0", true);
        button1_actionarray.add(screen5button1action0->value().c_str());
        AsyncWebParameter *screen5button1action1 = request->getParam("screen5button1action1", true);
        button1_actionarray.add(screen5button1action1->value().c_str());
        AsyncWebParameter *screen5button1action2 = request->getParam("screen5button1action2", true);
        button1_actionarray.add(screen5button1action2->value().c_str());

        JsonArray button1_valuearray = button1.createNestedArray("valuearray");
        AsyncWebParameter *screen5button1value0 = request->getParam("screen5button1value0", true);
        button1_valuearray.add(screen5button1value0->value().c_str());
        AsyncWebParameter *screen5button1value1 = request->getParam("screen5button1value1", true);
        button1_valuearray.add(screen5button1value1->value().c_str());
        AsyncWebParameter *screen5button1value2 = request->getParam("screen5button1value2", true);
        button1_valuearray.add(screen5button1value2->value().c_str());

        JsonObject button2 = doc.createNestedObject("button2");

        if (request->hasParam("screen5button2latch", true))
        {
          button2["latch"] = true;
        }
        else
        {
          button2["latch"] = false;
        }

        AsyncWebParameter *screen5latchlogo2 = request->getParam("screen5latchlogo2", true);
        LOC_LOGI(module,"%s",screen5latchlogo2->value().c_str());
        if (strcmp(screen5latchlogo2->value().c_str(), "---") == 0)
        {
          button2["latchlogo"] = "";
        }
        else
        {
          button2["latchlogo"] = screen5latchlogo2->value().c_str();
        }

        JsonArray button2_actionarray = button2.createNestedArray("actionarray");
        AsyncWebParameter *screen5button2action0 = request->getParam("screen5button2action0", true);
        button2_actionarray.add(screen5button2action0->value().c_str());
        AsyncWebParameter *screen5button2action1 = request->getParam("screen5button2action1", true);
        button2_actionarray.add(screen5button2action1->value().c_str());
        AsyncWebParameter *screen5button2action2 = request->getParam("screen5button2action2", true);
        button2_actionarray.add(screen5button2action2->value().c_str());

        JsonArray button2_valuearray = button2.createNestedArray("valuearray");
        AsyncWebParameter *screen5button2value0 = request->getParam("screen5button2value0", true);
        button2_valuearray.add(screen5button2value0->value().c_str());
        AsyncWebParameter *screen5button2value1 = request->getParam("screen5button2value1", true);
        button2_valuearray.add(screen5button2value1->value().c_str());
        AsyncWebParameter *screen5button2value2 = request->getParam("screen5button2value2", true);
        button2_valuearray.add(screen5button2value2->value().c_str());

        JsonObject button3 = doc.createNestedObject("button3");

        if (request->hasParam("screen5button3latch", true))
        {
          button3["latch"] = true;
        }
        else
        {
          button3["latch"] = false;
        }

        AsyncWebParameter *screen5latchlogo3 = request->getParam("screen5latchlogo3", true);
        LOC_LOGI(module,"%s",screen5latchlogo3->value().c_str());
        if (strcmp(screen5latchlogo3->value().c_str(), "---") == 0)
        {
          button3["latchlogo"] = "";
        }
        else
        {
          button3["latchlogo"] = screen5latchlogo3->value().c_str();
        }

        JsonArray button3_actionarray = button3.createNestedArray("actionarray");
        AsyncWebParameter *screen5button3action0 = request->getParam("screen5button3action0", true);
        button3_actionarray.add(screen5button3action0->value().c_str());
        AsyncWebParameter *screen5button3action1 = request->getParam("screen5button3action1", true);
        button3_actionarray.add(screen5button3action1->value().c_str());
        AsyncWebParameter *screen5button3action2 = request->getParam("screen5button3action2", true);
        button3_actionarray.add(screen5button3action2->value().c_str());

        JsonArray button3_valuearray = button3.createNestedArray("valuearray");
        AsyncWebParameter *screen5button3value0 = request->getParam("screen5button3value0", true);
        button3_valuearray.add(screen5button3value0->value().c_str());
        AsyncWebParameter *screen5button3value1 = request->getParam("screen5button3value1", true);
        button3_valuearray.add(screen5button3value1->value().c_str());
        AsyncWebParameter *screen5button3value2 = request->getParam("screen5button3value2", true);
        button3_valuearray.add(screen5button3value2->value().c_str());

        JsonObject button4 = doc.createNestedObject("button4");

        if (request->hasParam("screen5button4latch", true))
        {
          button4["latch"] = true;
        }
        else
        {
          button4["latch"] = false;
        }

        AsyncWebParameter *screen5latchlogo4 = request->getParam("screen5latchlogo4", true);
        LOC_LOGI(module,"%s",screen5latchlogo4->value().c_str());
        if (strcmp(screen5latchlogo4->value().c_str(), "---") == 0)
        {
          button4["latchlogo"] = "";
        }
        else
        {
          button4["latchlogo"] = screen5latchlogo4->value().c_str();
        }

        JsonArray button4_actionarray = button4.createNestedArray("actionarray");
        AsyncWebParameter *screen5button4action0 = request->getParam("screen5button4action0", true);
        button4_actionarray.add(screen5button4action0->value().c_str());
        AsyncWebParameter *screen5button4action1 = request->getParam("screen5button4action1", true);
        button4_actionarray.add(screen5button4action1->value().c_str());
        AsyncWebParameter *screen5button4action2 = request->getParam("screen5button4action2", true);
        button4_actionarray.add(screen5button4action2->value().c_str());

        JsonArray button4_valuearray = button4.createNestedArray("valuearray");
        AsyncWebParameter *screen5button4value0 = request->getParam("screen5button4value0", true);
        button4_valuearray.add(screen5button4value0->value().c_str());
        AsyncWebParameter *screen5button4value1 = request->getParam("screen5button4value1", true);
        button4_valuearray.add(screen5button4value1->value().c_str());
        AsyncWebParameter *screen5button4value2 = request->getParam("screen5button4value2", true);
        button4_valuearray.add(screen5button4value2->value().c_str());

        if (serializeJsonPretty(doc, file) == 0)
        {
          LOC_LOGD(module,"Failed to write to file");
        }
        file.close();
      }

      request->send(FILESYSTEM, "/saveconfig.htm");
    }
  });

  //----------- File list handler -----------------

  webserver.on("/list", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("dir"))
    {
      AsyncWebParameter *p = request->getParam("dir");
      request->send(200, "application/json", handleFileList(p->value().c_str()));
    }
  });

  webserver.on("/menus.json", HTTP_GET, [](AsyncWebServerRequest *request) {
    char * menusJson=FreeTouchDeck::MenusToJson(false);
    if(menusJson)
    {
      request->send(200, "application/json", menusJson);
      free(menusJson);
    }
    else
    {
      request->send(500,"Error Generating JSON structure");
    } 
  });
  webserver.on("/apislist", HTTP_GET, [](AsyncWebServerRequest *request) {

      request->send(200, "application/json", handleAPISList());

  });

  webserver.on("/info", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "application/json", handleInfo());
  });

  //----------- 404 handler -----------------

  webserver.onNotFound([](AsyncWebServerRequest *request) {
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

  webserver.onFileUpload([](AsyncWebServerRequest *request, const String &filename, size_t index, uint8_t *data, size_t len, bool final) {
    if (!index)
      Serial.printf("UploadStart: %s\n", filename.c_str());
    Serial.printf("%s", (const char *)data);
    if (final)
      Serial.printf("UploadEnd: %s (%u)\n", filename.c_str(), index + len);
  });

  webserver.onRequestBody([](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
    if (!index)
      Serial.printf("BodyStart: %u\n", total);
    Serial.printf("%s", (const char *)data);
    if (index + len == total)
      Serial.printf("BodyEnd: %u\n", total);
  });

  webserver.on(
      "/upload", HTTP_POST, [](AsyncWebServerRequest *request) {
        request->send(FILESYSTEM, "/upload.htm");
      },
      handleUpload);

  webserver.on("/restart", HTTP_POST, [](AsyncWebServerRequest *request) {
    // First send some text to the browser otherwise an ugly browser error shows up
    request->send(200, "text/plain", "FreeTouchDeck is restarting...");
    // Then restart the ESP
    LOC_LOGD(module,"Restarting");
    ESP.restart();
  });

  // ----------------------------- Error Handle ---------------------------------

  webserver.on("/error", HTTP_GET, [](AsyncWebServerRequest *request) {
    errorCode = "123";
    errorText = "Nothing went wrong, all is good. This is just a test";
    request->send(FILESYSTEM, "/error.htm", String(), false, processor);
  });

  // ----------------------------- Editor Handle ---------------------------------

  webserver.on("/editor", HTTP_POST, [](AsyncWebServerRequest *request) {
    int params = request->params();
    int i;
    int filecount = 0;
    for (i = 0; i < params; i++)
    {
      AsyncWebParameter *p = request->getParam(i);
      LOC_LOGI(module,"Deleting file: %s\n", p->value().c_str());
      String filename = "/logos/";
      filename += p->value().c_str();
      if (SPIFFS.exists(filename))
      {
        SPIFFS.remove(filename);
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
    request->send(FILESYSTEM, "/editor.htm", String(), false, deleteProcessor);
    resultFiles = "";
  });

  // ----------------------------- JSON Download Handle ---------------------------------

  webserver.on("/download", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncWebParameter *p = request->getParam("file");
    String filerequest = p->value().c_str();
    LOC_LOGI(module,"Requested file: %s\n", filerequest.c_str());

    String downloadfile = "/config/" + filerequest;
    LOC_LOGI(module,"Full path: %s\n", downloadfile.c_str());

    if (FILESYSTEM.exists(downloadfile))
    {
      LOC_LOGI(module,"Download file %s\n", downloadfile.c_str());
      request->send(FILESYSTEM, downloadfile, String(), true);
    }
    else
    {
      LOC_LOGI(module,"Download file %s doesn't exits!\n", downloadfile.c_str());
    }
  });

  // ----------------------------- JSON Upload Handle ---------------------------------

  webserver.on(
      "/uploadJSON", HTTP_POST, [](AsyncWebServerRequest *request) {}, handleJSONUpload);
}
