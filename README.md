# IMPORTANT NOTE!

The development version is intended for development only. Things might not work. Use the -master branch for a working and tested version of FreeTouchDeck. If you like to join development, feel free to do so! You can join my Discord server or send PR's. If you like to have a working verison of FreeTouchDeck, don't use this version.

# Development Version To-Do:

- [ ] RELEASEALL Doesn't work correctly when in one action.
- [ ] bug: NUM_ASTERIX should be NUM_ASTERISK
- [ ] Add a margin around the menus so we can have an offset from the edge off the screen
- [ ] Function buttons all have the same black colour
- [X] Dealing with the above -> we need a way to distinguish between MENU and FUNCTION button.
- [ ] Responsiveness is not always optimal (maybe switch back to task?).
- [ ] We need a configurator!
- [ ] Create a binary for easy flashing.
- [ ] Clean up icons, they are pretty noisy
- [ ] Write the docs! :)

# List of all Actions for use in menus.json

Below is a list of all built-in actions you can use in menus.json. Text (and also numbers) that are not between `{}` will be printed as plain text.

For example: `This is an example{BACKSPACE}`, will print: 

`This is an example`, then backspace will be pressed once and the final result will be `This is an exampl`. This will happen pretty fast so chances are that you wouldn't even see the last `e` being printed, you will just see the final result.

<details>
<summary>Modifier Keys:</summary>

- {LEFT_CTRL}
- {LEFT_SHIFT}
- {LEFT_ALT}
- {LEFT_GUI}
- {RIGHT_CTRL}
- {RIGHT_SHIFT}
- {RIGHT_ALT}
- {RIGHT_GUI}
</details>

<details>
<summary>Media Keys:</summary>

- {MUTE}
- {VOLUME_DOWN}
- {VOLUME_UP}
- {PLAY_PAUSE}
- {STOP}
- {NEXT_TRACK}
- {PREVIOUS_TRACK}
- {WWW_HOME}
- {LOCAL_MACHINE_BROWSER}
- {CALCULATOR}
- {WWW_BOOKMARKS}
- {WWW_SEARCH}
- {WWW_STOP}
- {WWW_BACK}
- {CONSUMER_CONTROL_CONFIGURATION}
- {EMAIL_READER}
</details>

<details>
<summary>Function Keys:</summary>

- {F1}
- {F2}
- {F3}
- {F4}
- {F5}
- {F6}
- {F7}
- {F8}
- {F9}
- {F10}
- {F11}
- {F12}
- {F13}
- {F14}
- {F15}
- {F16}
- {F17}
- {F18}
- {F19}
- {F20}
- {F21}
- {F22}
- {F23}
- {F24}
</details>

<details>
<summary>Return, tab, delete, etc.:</summary>

- {UP_ARROW}
- {DOWN_ARROW}
- {LEFT_ARROW}
- {RIGHT_ARROW}
- {BACKSPACE}
- {TAB}
- {RETURN}
- {PAGE_UP}
- {PAGE_DOWN}
- {DELETE}
- {ESC}
- {HOME}
- {END}
- {INSERT}
- {PRTSC}
- {HOME}
- {END}
- {CAPS_LOCK}
</details>

<details>
<summary>**Numpad Keys:**</summary>

- {NUM_0}
- {NUM_1}
- {NUM_2}
- {NUM_3}
- {NUM_4}
- {NUM_5}
- {NUM_6}
- {NUM_7}
- {NUM_8}
- {NUM_9}
- {NUM_SLASH}
- {NUM_ASTERISK}
- {NUM_MINUS}
- {NUM_PLUS}
- {NUM_ENTER}
- {NUM_PERIOD}
</details>

<details>
<summary>**Special Actions:**</summary>

- {MENU:_name_}
- {DELAY:_milliseconds_}
- {RELEASEALL}

</details>

<br>

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

Important to realise is that since this is a beta version, changes to te code happen frequently. The FreeTouchDeck.ino and other files (for example in the data folder) rely on each other, they come as one. So when you download the new version, make sure that you only use the files that come with the current download, and not files from other versions. Best practise is to completely delete the old version and then download/clone the new version to make sure you do not accidently mix files from different versions. **After downloading/cloning the latest version, make sure to also upload the "data" folder again using 'ESP32 Sketch Data Upload".**

# Beta Version!

This version is in it's very early stages of development. Chances are that if you are not using the exact
same setup as I am, you will run in to problems. But that is what this version is for: finding out what is needed
to make FreeTouchDeck work across most ESP's and TFT screens. Also there is a lack of documentation which is gradually being written.

# Hardware used

The hardware I currenlty use is:

For Resistive touch:
- an ESP32 DEVKIT V1 (WROOM32) (Partition scheme: NO OTA with 2MB app and 2MB SPIFFS)
- an 3.5" (480x320) TFT + Touchscreen with ILI9488 driver and XPT2046 resitive touch controller

For Capacitive touch:
- an ESP32 DEVKIT V1 (WROOM32) (Partition scheme: NO OTA with 2MB app and 2MB SPIFFS)
- an 3.5" (480x320) TFT + Touchscreen with ILI9488 driver and FT6236 capacitive touch controller

# !- Library Dependencies -!
- Adafruit-GFX-Library (version 1.10.0 or higher), available through Library Manager
- TFT_eSPI (version 2.2.14 or higher), available through Library Manager
- ESP32-BLE-Keyboard (forked) (latest version) download from: https://github.com/DustinWatts/ESP32-BLE-Keyboard
- ESPAsyncWebserver (latest version) download from: https://github.com/me-no-dev/ESPAsyncWebServer
- AsyncTCP (latest version) download from: https://github.com/me-no-dev/AsyncTCP
- TJpg_Decoder (version 0.0.3 or higher), available through Library Manager or https://github.com/Bodmer/TJpg_Decoder

If you use capacitive touch:
- FT6236 (latest version), download from: https://github.com/DustinWatts/FT6236

# Combiner PCB for an ESP32 DevKit C (38-pin only) + ILI9488 Touch Module:

https://github.com/DustinWatts/ESP32_TFT_Combiner

# TFT_eSPI configuration

Before compiling and uploading the FreeTouchDeck.ino sketch, you will have to edit the **user_setup.h** file included with the TFT_eSPI library. This can be found in your Arduino skechtbook folder under "libraries". If you have not renamed the TFT_eSPI library folder, the file **user_setup.h** can be found in **TFT_eSPI-master**. Here you will have to uncomment the lines that apply to you hardware configuration. For example: if you have an TFT with an ILI9488 driver, you will have to uncomment that line under `Section 1`. Make sure all the other drivers are commented out!  

The next section is `Section 2`. This also depends on what hardware you are using. For example for an ESP32 you'll have to uncomment the correct #define(s) under `EDIT THE PIN NUMBERS IN THE LINES FOLLOWING TO SUIT YOUR ESP32 SETUP`. Also if your TFT has the blacklight control pin available you will have to uncomment the lines found under `#define TFT_BL` and `#define TFT_BACKLIGHT_ON`.  

"Section 3" can be left alone.   

# Help

You can join my Discord server where I have a dedicated #freetouchdeck channel. https://discord.gg/RE3XevS
