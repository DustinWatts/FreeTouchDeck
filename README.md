# FreeTouchDeck
For interfacing with Windows/macOS/Linux using an ESP32, a touchscreen and BLE.

### [User guide](https://github.com/DustinWatts/FreeTouchDeck/wiki)

# ESP32 TouchDown users

Make sure to uncomment the line `//#define USECAPTOUCH`!   
And if you wish to use the speaker uncomment the line `//#define speakerPin 26`

If FreeTouchDeck came pre-installed, you can find how to set up the configurator here:   
https://github.com/DustinWatts/esp32-touchdown/wiki/With-FreeTouchDeck-pre-installed

# Delete the old clone and use the new

### Mixing files of different versions may cause some unexpected behavior!

The FreeTouchDeck.ino and other files (for example in the data folder) rely on each other, they come as one. So when you download the new version, make sure that you only use the files that come with the current download, and not files from other versions. Best practise is to completely delete the old version and then download/clone the new version to make sure you do not accidently mix files from different versions. **After downloading/cloning the latest version, make sure to also upload the "data" folder again using 'ESP32 Sketch Data Upload".**

# Hardware used

The hardware I currenlty use is:

For Resistive touch:
- an ESP32 DEVKIT V1 (WROOM32) (Partition scheme: NO OTA with 2MB app and 2MB SPIFFS)
- an 3.5" (480x320) TFT + Touchscreen with ILI9488 driver and XPT2046 resitive touch controller

For Capacitive touch:
- an ESP32 TouchDown: https://www.esp32touchdown.com/

# !- Library Dependencies -!
- Adafruit-GFX-Library (tested with version 1.10.4), available through Library Manager
- TFT_eSPI (tested with version 2.3.70), available through Library Manager
- ESP32-BLE-Keyboard (latest version) download from: https://github.com/T-vK/ESP32-BLE-Keyboard
- ESPAsyncWebserver (latest version) download from: https://github.com/me-no-dev/ESPAsyncWebServer
- AsyncTCP (latest version) download from: https://github.com/me-no-dev/AsyncTCP
- ArduinoJson (tested with version 6.17.3), available through Library Manager

If you use capacitive touch:
- Dustin Watts FT6236 Library (version 1.0.2), https://github.com/DustinWatts/FT6236

# Combiner PCB for an ESP32 DevKit C (38-pin only) + ILI9488 Touch Module:

https://github.com/DustinWatts/ESP32_TFT_Combiner

# TFT_eSPI configuration

Before compiling and uploading the FreeTouchDeck.ino sketch, you will have to edit the **user_setup.h** file included with the TFT_eSPI library. This can be found in your Arduino skechtbook folder under "libraries". If you have not renamed the TFT_eSPI library folder, the file **user_setup.h** can be found in **TFT_eSPI-master**. Here you will have to uncomment the lines that apply to you hardware configuration. For example: if you have an TFT with an ILI9488 driver, you will have to uncomment that line under `Section 1`. Make sure all the other drivers are commented out!  

The next section is `Section 2`. This also depends on what hardware you are using. For example for an ESP32 you'll have to uncomment the correct #define(s) under `EDIT THE PIN NUMBERS IN THE LINES FOLLOWING TO SUIT YOUR ESP32 SETUP`. Also if your TFT has the blacklight control pin available you will have to uncomment the lines found under `#define TFT_BL` and `#define TFT_BACKLIGHT_ON`.  

"Section 3" can be left alone.   

# Help

You can join my Discord server where I have a dedicated #freetouchdeck channel. https://discord.gg/RE3XevS
