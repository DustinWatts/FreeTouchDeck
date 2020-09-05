# Steps for installing FreeTouchDeck

* Maybe a bit obvious but just in case: Install the Arduino IDE and install the Arduino-ESP32 core
* Install the following libraries:
	- Adafruit-GFX-Library (version 1.10.0), available through Library Manager
	- TFT_eSPI (version 2.2.14), available through Library Manager
	- ESP32-BLE-Keyboard (latest version), download from: https://github.com/T-vK/ESP32-BLE-Keyboard
	- ArduinoJson (version 6.16.1), available through Library Manager

* Open FreeTouchDeck.ino and set your WiFi credentials under const char* ssid = " "; and const char* password = " ";
* Change the settings in tft_config.h to match your hardware
* Upload the contents of the "data" folder to the ESP32 SPIFFS using "ESP Sketch Data Upload" tool that can be found here: https://github.com/me-no-dev/arduino-esp32fs-plugin
* Make sure you have enough room for the sketch, by setting the correct Partition scheme. (NO OTA (2MB APP/2MB SPIFFS) should be ok.)
* Upload the sketch.

After that, you can now customize the FreeTouchDeck. In the main menu the bottom right icon ("settings") takes you to the settings page. Then the top left ("Wifi") puts FreeTouchDeck in to configuration mode. Now you can go to http://freetouchdeck.local and change the button functions to your liking. Here you will also find some general configuration settings like colors that are being used and the logos for the home screen.

# Need help?

You can join my Discord server where I have a dedicated #freetouchdeck channel. https://discord.gg/RE3XevS
