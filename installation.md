# Steps for installing FreeTouchDeck

* Maybe a bit obvious but just in case: Install the Arduino IDE and install the Arduino-ESP32 core
* Install the following libraries:
	- Adafruit-GFX-Library (version 1.10.0), available through Library Manager
	- TFT_eSPI (version 2.2.14), available through Library Manager
	- ESP32-BLE-Keyboard (latest version), download from: https://github.com/T-vK/ESP32-BLE-Keyboard
	- ArduinoJson (version 6.16.1), available through Library Manager

* Open FreeTouchDeck.ino and set your WiFi credentials under const char* ssid = " "; and const char* password = " ";
* Before compiling and uploading the FreeTouchDeck.ino sketch, you will have to edit the **user_setup.h** file included with the TFT_eSPI library. This can be found in your Arduino skechtbook folder under "libraries". If you have not renamed the TFT_eSPI library folder, the file **user_setup.h** can be found in **TFT_eSPI-master**. Here you will have to uncomment the lines that apply to you hardware configuration. For example: if you have an TFT with an ILI9488 driver, you will have to uncomment that line under `Section 1`. Make sure all the other drivers are commented out!

The next section is `Section 2`. This also depends on what hardware you are using. For example for an ESP32 you'll have to uncomment the correct #define(s) under `EDIT THE PIN NUMBERS IN THE LINES FOLLOWING TO SUIT YOUR ESP32 SETUP`. Also if your TFT has the blacklight control pin available you will have to uncomment the lines found under `#define TFT_BL` and `#define TFT_BACKLIGHT_ON`.

"Section 3" can be left alone. 

* Upload the contents of the "data" folder to the ESP32 SPIFFS using "ESP Sketch Data Upload" tool that can be found here: https://github.com/me-no-dev/arduino-esp32fs-plugin
* This is a large sketch, so we need to make room for it. The default partitioning scheme is not big enough. Under "Tools / Partion Scheme" there are, depending on your board, a few options. Select the option that says someting like "Large APP/No OTA" or "Huge APP/No OTA" or "NO OTA (2MB APP/2MB SPIFFS)".
* Upload the sketch.

After that, you can now customize the FreeTouchDeck. In the main menu the bottom right icon ("settings") takes you to the settings page. Then the top left ("Wifi") puts FreeTouchDeck in to configuration mode. Now you can go to http://freetouchdeck.local and change the button functions to your liking. Here you will also find some general configuration settings like colours that are being used and the logos for the home screen.

# Need help?

You can join my Discord server where I have a dedicated #freetouchdeck channel. https://discord.gg/RE3XevS
