# FreeDeckTouch
For interfacing with Windows/macOS/Linux using an ESP32, a touchscreen and BLE.

### [Installation](https://github.com/DustinWatts/FreeTouchDeck/blob/master/installation.md)

# Pre-Alpha Version!

This version is in it's very early stages of development. Chances are that if you are not using the exact
same testup as I am, you will run in to problems. But that is what this version is for: finding out what is needed
to make FreeTouchDeck work across most ESP's and TFT screens. Also there is a lack of documentation which is gradually being written.

# Hardware used

The hardware I currenlty use is:

- an ESP32 DEVKIT V1 (WROOM32) (Partition scheme: NO OTA with 2MB app and 2MB SPIFFS)
- an 3.5" (480x320) TFT + Touchscreen with ILI9488 driver and XPT2046 touch controller

# !- Library Dependencies -!
- Adafruit-GFX-Library (version 1.10.0), available through Library Manager
- TFT_eSPI (version 2.2.14), available through Library Manager
- ESP32-BLE-Keyboard (latest version), download from: https://github.com/T-vK/ESP32-BLE-Keyboard
- ArduinoJson (version 6.16.1), available through Library Manager

# tft_config.h

This is where the TFT and touchscreen configuration for TFT_eSPI lives. As you can see, the current configuration is set up for my hardware. You'll need to change this to suit your hardware. On my TFT screen the MOSI and CLK where not shared on the PCB. They can however share the same pin on the ESP32.
