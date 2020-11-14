/* ------------ Convert RGB888 colour code to RGB565  ---------------- 
Purpose: This function returns all the files in a given directory in a json 
         formatted string.
Input  : String path
Output : String
Note   : none
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

/* ------------------- Uploading a file ---------------- 
Purpose: This function handles a file upload used by the Webserver
         formatted string.
Input  : *request, String filename, size_t index, uint8_t *data, size_t len, bool final
Output : none
Note   : none
*/

void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
{
  if (!index)
  {
    Serial.printf("[INFO]: File Upload Start: %S\n", filename.c_str());
    filename = "/logos/" + filename; // TODO: Does the logo directory need to be hardcoded?
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
    Serial.printf("[INFO]: File Uploaded: %S\n", filename.c_str());
    // Close the file handle as the upload is now done
    request->_tempFile.close();
    request->send(FILESYSTEM, "/upload.htm");
  }
}

/* ----------------- Adding handelers to the Async Webserver ---------------- 
Purpose: This function adds all the handlers we need to the webserver. 
Input  : none
Output : none
Note   : Only need to call this once! This is also where the saving of config files is done.
*/

void handlerSetup()
{

  webserver.serveStatic("/", FILESYSTEM, "/").setDefaultFile("index.htm");

  //----------- index.htm handler -----------------

  webserver.on("/index.htm", HTTP_POST, [](AsyncWebServerRequest *request) {
    request->send(FILESYSTEM, "/index.htm");
  });

  //----------- saveconfig handler -----------------

  webserver.on("/saveconfig", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("save", true))
    {
      AsyncWebParameter *p = request->getParam("save", true);
      String savemode = p->value().c_str();

      if (savemode == "savecolors")
      {

        // --- Saving colours
        Serial.println("[INFO]: Saving Colours");

        FILESYSTEM.remove("/config/colors.json");
        File file = FILESYSTEM.open("/config/colors.json", "w");
        if (!file)
        {
          Serial.println("[WARNING]: Failed to create file");
          return;
        }

        DynamicJsonDocument doc(256);

        JsonObject colors = doc.to<JsonObject>();

        AsyncWebParameter *menubuttoncolor = request->getParam("menubuttoncolor", true);
        colors["menubuttoncolor"] = menubuttoncolor->value().c_str();
        AsyncWebParameter *functionbuttoncolor = request->getParam("functionbuttoncolor", true);
        colors["functionbuttoncolor"] = functionbuttoncolor->value().c_str();
        AsyncWebParameter *latchcolor = request->getParam("latchcolor", true);
        colors["latchcolor"] = latchcolor->value().c_str();
        AsyncWebParameter *background = request->getParam("background", true);
        colors["background"] = background->value().c_str();

        if (serializeJsonPretty(doc, file) == 0)
        {
          Serial.println("[WARNING]: Failed to write to file");
        }
        file.close();

        // Save sleep settings
        Serial.println("[INFO]: Saving Sleep Settings");

        FILESYSTEM.remove("/config/wificonfig.json");
        File sleep = FILESYSTEM.open("/config/wificonfig.json", "w");
        if (!sleep)
        {
          Serial.println("[WARNING]: Failed to create file");
          return;
        }

        DynamicJsonDocument doc2(256);

        JsonObject wificonfigobject = doc2.to<JsonObject>();

        wificonfigobject["ssid"] = wificonfig.ssid;
        wificonfigobject["password"] = wificonfig.password;
        wificonfigobject["wifihostname"] = wificonfig.hostname;

        AsyncWebParameter *sleepenable = request->getParam("sleepenable", true);
        String sleepEnable = sleepenable->value().c_str();

        if (sleepEnable == "true")
        {
          wificonfigobject["sleepenable"] = true;
        }
        else
        {
          wificonfigobject["sleepenable"] = false;
        }

        AsyncWebParameter *sleeptimer = request->getParam("sleeptimer", true);

        String sleepTimer = sleeptimer->value().c_str();
        wificonfigobject["sleeptimer"] = sleepTimer.toInt();

        if (serializeJsonPretty(doc2, sleep) == 0)
        {
          Serial.println("[WARNING]: Failed to write to file");
        }
        file.close();
      }
      else if (savemode == "homescreen")
      {

        // --- Saving Homescreen

        Serial.println("[INFO]: Saving Homescreen");

        FILESYSTEM.remove("/config/homescreen.json");
        File file = FILESYSTEM.open("/config/homescreen.json", "w");
        if (!file)
        {
          Serial.println("[WARNING]: Failed to create file");
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
          Serial.println("[WARNING]: Failed to write to file");
        }
        file.close();
      }
      else if (savemode == "menu1")
      {

        // --- Save menu 1

        Serial.println("[INFO]: Saving Menu 1");
        FILESYSTEM.remove("/config/menu1.json");
        File file = FILESYSTEM.open("/config/menu1.json", "w");
        if (!file)
        {
          Serial.println("[WARNING]: Failed to create menu1.json");
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
          Serial.println("[WARNING]: Failed to write to file");
        }
        file.close();
      }
      else if (savemode == "menu2")
      {

        // --- Save menu 2

        Serial.println("[INFO]: Saving Menu 2");
        FILESYSTEM.remove("/config/menu2.json");
        File file = FILESYSTEM.open("/config/menu2.json", "w");
        if (!file)
        {
          Serial.println("[WARNING]: Failed to create menu2.json");
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
          Serial.println("[WARNING]: Failed to write to file");
        }
        file.close();
      }
      else if (savemode == "menu3")
      {

        // --- Save menu 3

        Serial.println("[INFO]: Saving Menu 3");
        FILESYSTEM.remove("/config/menu3.json");
        File file = FILESYSTEM.open("/config/menu3.json", "w");
        if (!file)
        {
          Serial.println("[WARNING]: Failed to create menu3.json");
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
          Serial.println("[WARNING]: Failed to write to file");
        }
        file.close();
      }
      else if (savemode == "menu4")
      {

        // --- Save menu 4

        Serial.println("[INFO]: Saving Menu 4");
        FILESYSTEM.remove("/config/menu4.json");
        File file = FILESYSTEM.open("/config/menu4.json", "w");
        if (!file)
        {
          Serial.println("[WARNING]: Failed to create menu3.json");
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
          Serial.println("[WARNING]: Failed to write to file");
        }
        file.close();
      }
      else if (savemode == "menu5")
      {

        // --- Save menu 5

        Serial.println("[INFO]: Saving Menu 5");
        FILESYSTEM.remove("/config/menu5.json");
        File file = FILESYSTEM.open("/config/menu5.json", "w");
        if (!file)
        {
          Serial.println("[WARNING]: Failed to create menu5.json");
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
          Serial.println("[WARNING]: Failed to write to file");
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
    Serial.println("[WARNING]: Restarting");
    ESP.restart();
  });
}
