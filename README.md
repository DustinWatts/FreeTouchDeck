# FreeTouchDeck
[![](https://badgen.net/github/last-commit/DustinWatts/FreeTouchDeck)](https://github.com/DustinWatts/FreeTouchDeck/commits/master)
[![](https://badgen.net/badge/0.9.17%20compile/succes/green)](https://badgen.net/badge/0.9.17%20compile/succes/green)
[![](https://badgen.net/github/release/DustinWatts/FreeTouchDeck)](https://github.com/DustinWatts/FreeTouchDeck/releases)
[![](https://img.shields.io/discord/693862273864827012?color=5165f6&label=chat%20on%20Discord)](https://discord.gg/RE3XevS)
[![](https://badgen.net/github/license/DustinWatts/FreeTouchDeck)](https://github.com/DustinWatts/FreeTouchDeck/blob/master/LICENSE)
[![](https://badgen.net/badge/watch/on%20youtube/ff0000)](https://www.youtube.com/watch?v=s2X4BQ9VmEU)
[![](https://img.shields.io/twitter/follow/DustinWattsNL)](https://twitter.com/DustinWattsNL)



For interfacing with Windows/macOS/Linux using an ESP32, a touchscreen and BLE. 

***
***Due to constant improvements in the `-master` branche, getting the latest code straight from the `-master` branche (by dowloading it as a ZIP for example) might be unstable. If you want a stable and tested version, download the latest release from the [Releases](https://github.com/DustinWatts/FreeTouchDeck/releases) page.***

***The current latest stable is 0.9.17.***
***

## We are looking for (web) developers!

We are looking for (web) developers who are willing to help out updating the configurator (or changing it completely!) to work with all the new features that the Development branch offers. Once we have a working configurator, the Development branch will replace the current master as FreeTouchDeck version 2.0. As a token of my appreciation, you can get (if you will) an ESP32 TouchDown S3 at cost price before it releases! Also you will be credited here, in the source code and in the documentation. If you want to join, let us know on Discord: https://discord.gg/RE3XevS

## Install Using The Web Installer (recommended!)

Easy installation without the need for the Arduino IDE, and downloading and editing libraries is now available using ESP Web Tools. Visit this url to install FreeTouchDeck to your board via your browser: (https://install.freetouchdeck.com/) Chrome, Edge, and Opera only at the moment.

## User Guide
The User Guide will help you with installing and configuring if you want to build the project yourself: [User guide](https://github.com/DustinWatts/FreeTouchDeck/wiki).

There is also an 'Instructable' which is a complete build guide including wiring the screen and ESP32 on [Instructables](https://www.instructables.com/A-Bluetooth-ESP32-TFT-Touch-Macro-Keypad/).

## ESP32 TouchDown users

Make sure to uncomment the line `//#define USECAPTOUCH`!   
And if you wish to use the speaker uncomment the line `//#define speakerPin 26`

If FreeTouchDeck came pre-installed, you can find how to set up the configurator here:   
https://github.com/DustinWatts/esp32-touchdown/wiki/With-FreeTouchDeck-pre-installed

## Helper app

I wrote a helper app for Windows/macOS/Linux that will help you start applications, run scripts and can auto-switch FreeTouchDeck to a page you choose when an application comes in to focus. You can find it here: https://github.com/DustinWatts/FreeTouchDeck-Helper

## Delete the old clone and use the new

### Mixing files of different versions may cause some unexpected behavior!

The FreeTouchDeck.ino and other files (for example in the data folder) rely on each other, they come as one. So when you download the new version, make sure that you only use the files that come with the current download, and not files from other versions. Best practise is to completely delete the old version and then download/clone the new version to make sure you do not accidently mix files from different versions. **After downloading/cloning the latest version, make sure to also upload the "data" folder again using 'ESP32 Sketch Data Upload".**

## Supported Hardware

For Resistive touch:
- an ESP32 DEVKIT V1 (WROOM32) (Partition scheme: NO OTA with 2MB app and 2MB SPIFFS)
- an 3.5" (480x320) TFT + Touchscreen with ILI9488 driver and XPT2046 resitive touch controller

For Capacitive touch:
- an ESP32 TouchDown: https://www.esp32touchdown.com/

If you try to run FTD on something other than the boards above, it might not work. Support for custom combinations of boards and screens is out of the scope of support. I will try and help you if you run in to something, but issues here on Github are not dealt with and will be closed with this reason.

## Community support

Lot's of people have made FreeTouchDeck work on other hardware then the above hardware. Although not offically suported. I will list those forks here.

- ESP32-3248S035 Capacitive / GT911 touchscreen: https://github.com/Raycast7373/FreeTouchDeck
- ESP32-2432S028 Resistive touchscreen users: https://github.com/Raycast7373/FreeTouchDeck

I will also point you to my Discord (https://discord.gg/RE3XevS) Where these people are active. If you have a custom build/fork for a board you would like listed here, contact me and I will add it to the list. 

## Library Dependencies
- Adafruit-GFX-Library (tested with version 1.10.4), available through Library Manager
- TFT_eSPI (tested with version 2.3.70), available through Library Manager
- ESP32-BLE-Keyboard (latest version) download from: https://github.com/T-vK/ESP32-BLE-Keyboard
- ESPAsyncWebserver (latest version) download from: https://github.com/me-no-dev/ESPAsyncWebServer
- AsyncTCP (latest version) download from: https://github.com/me-no-dev/AsyncTCP
- ArduinoJson (tested with version 6.17.3), available through Library Manager

If you use capacitive touch:
- Dustin Watts FT6236 Library (version 1.0.2), https://github.com/DustinWatts/FT6236

## Combiner PCB for an ESP32 DevKit C (38-pin only) + ILI9488 Touch Module:

https://github.com/DustinWatts/ESP32_TFT_Combiner

## TFT_eSPI configuration

Before compiling and uploading the FreeTouchDeck.ino sketch, you will have to edit the **user_setup.h** file included with the TFT_eSPI library. This can be found in your Arduino skechtbook folder under "libraries". If you have not renamed the TFT_eSPI library folder, the file **user_setup.h** can be found in **TFT_eSPI-master**. 

You can find examples of **User_Setup.h** files in the folder called [user_setup.h Examples](https://github.com/DustinWatts/FreeTouchDeck/tree/master/user_setup.h%20Examples). Delete everything that is in the **User_Setup.h** file (so you have a blank file) and copy & paste the _contents_ of the file you need for your setup. Don't forget to save!

If you do it manually you will have to uncomment the lines that apply to you hardware configuration. For example: if you have an TFT with an ILI9488 driver, you will have to uncomment that line under `Section 1`. Make sure all the other drivers are commented out!  

The next section is `Section 2`. This also depends on what hardware you are using. For example for an ESP32 you'll have to uncomment the correct #define(s) under `EDIT THE PIN NUMBERS IN THE LINES FOLLOWING TO SUIT YOUR ESP32 SETUP`. Also if your TFT has the blacklight control pin available you will have to uncomment the lines found under `#define TFT_BL` and `#define TFT_BACKLIGHT_ON`.  

"Section 3" can be left alone.

## Cases

In the [case/ESP32_TFT_Combiner_Case](https://github.com/DustinWatts/FreeTouchDeck/tree/master/case/ESP32_TFT_Combiner_Case) you can find a case with different tops (front) and bottoms (back). You can also find user made cases on [Thingiverse](https://www.thingiverse.com/search?q=FreeTouchDeck) by following the link or searching for `FreeTouchDeck` on Thingiverse, Printables or good old Google.

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
