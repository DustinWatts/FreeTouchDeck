// --------------------------------- Enter Config Mode ----------------------------------------

void configmode()
{

  Serial.println("[INFO]: Entering Config Mode");

  // Delete the task bleKeyboard had create to free memory and to not interfere with AsyncWebServer
  bleKeyboard.end();

  // Stop BLE from interfering with our WIFI signal
  btStop();
  esp_bt_controller_disable();
  esp_bt_controller_deinit();
  esp_bt_controller_mem_release(ESP_BT_MODE_BTDM);

  Serial.println("[INFO]: BLE Stopped");

  if (String(wificonfig.ssid) == "YOUR_WIFI_SSID" || String(wificonfig.password) == "YOUR_WIFI_PASSWORD") // Still default
  {
    drawErrorMessage("WiFi Config still set to default!");
    Serial.println("[ERROR]: WiFi Config still set to default!");
    while (1)
      yield(); // Stop!
  }

  if (String(wificonfig.ssid) == "FAILED" || String(wificonfig.password) == "FAILED") // The wificonfig.json failed to load
  {
    drawErrorMessage("WiFi Config Failed to load!");
    Serial.println("[ERROR]: WiFi Config Failed to load!");
    while (1)
      yield(); // Stop!
  }

  drawErrorMessage("Connecting to Wifi...");

  Serial.printf("[INFO]: Connecting to %s", wificonfig.ssid);
  if (String(WiFi.SSID()) != String(wificonfig.ssid))
  {
    WiFi.mode(WIFI_STA);
    WiFi.begin(wificonfig.ssid, wificonfig.password);
  }

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("[INFO]: Connected! IP address: ");
  Serial.println(WiFi.localIP());

  MDNS.addService("http", "tcp", 80);
  MDNS.begin(wificonfig.hostname);

  // Set pageNum to 7 so no buttons are displayed and touches are ignored
  pageNum = 7;

  // Start the webserver
  webserver.begin();
  Serial.println("[INFO]: Webserver started");
}

/* ------------------------ Check if config file exists function ---------------- 
Purpose: This function checks if a file exists and returns a boolean accordingly.
         It then prints a debug message to the serial as wel as the tft.
Input  : char *filename
Output : boolean
Note   : Pass the filename including a leading /
*/

bool checkfile(char *filename)
{

  if (!FILESYSTEM.exists(filename))
  {
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(1, 3);
    tft.setTextFont(2);
    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.printf("%s not found!\n\n", filename);
    tft.setTextSize(1);
    tft.printf("If this has happend after confguration, the data on the ESP may \nbe corrupted.");
    return false;
  }
}
