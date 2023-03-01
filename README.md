# FreeTouchDeck
[![](https://badgen.net/github/last-commit/DustinWatts/FreeTouchDeck)](https://github.com/DustinWatts/FreeTouchDeck/commits/master)
[![](https://badgen.net/github/release/DustinWatts/FreeTouchDeck)](https://github.com/DustinWatts/FreeTouchDeck/releases)
[![](https://img.shields.io/discord/693862273864827012?color=5165f6&label=chat%20on%20Discord)](https://discord.gg/RE3XevS)
[![](https://badgen.net/github/license/DustinWatts/FreeTouchDeck)](https://github.com/DustinWatts/FreeTouchDeck/blob/master/LICENSE)
[![](https://badgen.net/badge/watch/on%20youtube/ff0000)](https://www.youtube.com/watch?v=s2X4BQ9VmEU)
[![](https://img.shields.io/twitter/follow/DustinWattsNL)](https://twitter.com/DustinWattsNL)



For interfacing with Windows/macOS/Linux using an ESP32, a touchscreen and BLE. 

***Version 0.9.18a remark: This version might be unstable due to current work on adding support for the ESP32-S3. Last stable is 0.9.17 which you can find in the Releases section.***

## Install Using The Web Installer (recommended!)

Easy installation without the need for the Arduino IDE, and downloading and editing libraries is now available using ESP Web Tools. Visit this url to install FreeTouchDeck to your board via your browser: (https://install.freetouchdeck.com/) Chrome, Edge, and Opera only at the moment.

## User Guide
The User Guide will help you with installing and configuring if you want to build the project yourself: [User guide](https://github.com/DustinWatts/FreeTouchDeck/wiki)

## ESP32 TouchDown users

Make sure to uncomment the line `//#define TouchDown`!   
And if you wish to use the speaker uncomment the line `//#define speakerPin 26`

If FreeTouchDeck came pre-installed, you can find how to set up the configurator here:   
https://github.com/DustinWatts/esp32-touchdown/wiki/With-FreeTouchDeck-pre-installed

## ESP32-3248S035 Capative / GT911 touchscreen users

Make sure to uncomment the line `//#define esp323248s035c` if you use a ESP32-3248S035 Capacitive or `//#define GT911` If you have a GT911 touchscreen

## ESP32-2432S028 Resistive touchscreen users

Make sure to uncomment the line `//#define esp322432s028r`


## Resistive Touchscreen users

Make sure to uncomment the line `//#define ResistiveTouch`

## XPT2046 Touchscreen users

If FreeTouchDeck doesn't work by default, you can try it another way by doing the following things:

1. Install the following library: https://github.com/Bodmer/TFT_Touch
2. Uncomment the line `//#define esp322432s028r`
3. Comment the autobrightness option for the esp322432s028r
4. Change the resolution to match the resolution for your screen (the screen resolution in the line after #ifdef esp322432s028r)
5. Change the pins at the touchscreen part for the esp322432s028r (is a bit further down in the file) to match your pin layout
6. Try and see if it works

## Helper app

I wrote a helper app for Windows/macOS/Linux that will help you start applications, run scripts and can auto-switch FreeTouchDeck to a page you choose when an application comes in to focus. You can find it here: https://github.com/DustinWatts/FreeTouchDeck-Helper

## Delete the old clone and use the new

### Mixing files of different versions may cause some unexpected behavior!

The FreeTouchDeck.ino and other files (for example in the data folder) rely on each other, they come as one. So when you download the new version, make sure that you only use the files that come with the current download, and not files from other versions. Best practise is to completely delete the old version and then download/clone the new version to make sure you do not accidently mix files from different versions. **After downloading/cloning the latest version, make sure to also upload the "data" folder again using 'ESP32 Sketch Data Upload".**

## Hardware used

The hardware I currenlty use is:

For Resistive touch:
- an ESP32 DEVKIT V1 (WROOM32) (Partition scheme: NO OTA with 2MB app and 2MB SPIFFS)
- an 3.5" (480x320) TFT + Touchscreen with ILI9488 driver and XPT2046 resitive touch controller

For Capacitive touch:
- an ESP32 TouchDown: https://www.esp32touchdown.com/

Hardware used by Raycast:
- an ESP3248S035 capacitive: aliexpress.com/item/1005004632953455.html
or just a GT911 touch screen (in case of problems include @Raycast#7373 in your message on Discord or here)

## !- Library Dependencies -!
- Adafruit-GFX-Library (tested with version 1.10.4), available through Library Manager
- TFT_eSPI (tested with version 2.3.70), available through Library Manager
- ESP32-BLE-Keyboard (latest version) download from: https://github.com/T-vK/ESP32-BLE-Keyboard
- ESPAsyncWebserver (latest version) download from: https://github.com/me-no-dev/ESPAsyncWebServer
- AsyncTCP (latest version) download from: https://github.com/me-no-dev/AsyncTCP
- ArduinoJson (tested with version 6.17.3), available through Library Manager

If you use capacitive touch:
- Dustin Watts FT6236 Library (version 1.0.2), https://github.com/DustinWatts/FT6236

or in case of a GT911 touchscreen (like the ESP32-3248S035 capacitive has):
- TAMCTec GT911 Library (version 1.0.2), https://github.com/TAMCTec/gt911-arduino

Incase your XPT2046 touchscreen doesn't work by default (or when you use a ESP32-2432S028 resistive):
- TFT_Touch library (Latest version), https://github.com/Bodmer/TFT_Touch

## Combiner PCB for an ESP32 DevKit C (38-pin only) + ILI9488 Touch Module:

https://github.com/DustinWatts/ESP32_TFT_Combiner

## TFT_eSPI configuration

Before compiling and uploading the FreeTouchDeck.ino sketch, you will have to edit the **user_setup.h** file included with the TFT_eSPI library. This can be found in your Arduino skechtbook folder under "libraries". If you have not renamed the TFT_eSPI library folder, the file **user_setup.h** can be found in **TFT_eSPI-master**. Here you will have to uncomment the lines that apply to you hardware configuration. For example: if you have an TFT with an ILI9488 driver, you will have to uncomment that line under `Section 1`. Make sure all the other drivers are commented out!  

The next section is `Section 2`. This also depends on what hardware you are using. For example for an ESP32 you'll have to uncomment the correct #define(s) under `EDIT THE PIN NUMBERS IN THE LINES FOLLOWING TO SUIT YOUR ESP32 SETUP`. Also if your TFT has the blacklight control pin available you will have to uncomment the lines found under `#define TFT_BL` and `#define TFT_BACKLIGHT_ON`.  

"Section 3" can be left alone.

## Support Me

If you like what I am doing, there are a number of ways you can support me. 

| Platform | Link|
|:-----:|:-----|
| [<img src="https://github.com/DustinWatts/small_logos/blob/main/twitter_logo.png" alt="Twtter" width="24"/>](https://twitter.com/dustinwattsnl "Follow me on Twitter") | You can follow me on Twitter: [@dustinwattsnl](https://twitter.com/dustinwattsnl "Follow me on Twitter")|
| [<img src="https://github.com/DustinWatts/small_logos/blob/main/youtube_logo.png" alt="YouTube" width="32"/>](https://www.youtube.com/dustinwatts "Subscrive to my YouTube channel") | You can subscribe to my channel on Youtube: [/dustinWatts](https://www.youtube.com/dustinwatts "Subscribe to my YouTube channel") |
| [<img src="https://github.com/DustinWatts/small_logos/blob/main/patreon_logo.png" alt="Patreon" width="32"/>](https://www.patreon.com/dustinwatts) | You can support me by becoming a patron on Patreon: https://www.patreon.com/dustinwatts |
| [<img src="https://github.com/DustinWatts/small_logos/blob/main/paypalme_logo.png" alt="PayPal.me" width="32"/>](https://www.paypal.me/dustinwattsnl) | You can make a one time donation using PayPal.me: https://www.paypal.me/dustinwattsnl |

## Get help

For quick access to help you can join my Discord server where I have a dedicated #freetouchdeck channel. https://discord.gg/RE3XevS
