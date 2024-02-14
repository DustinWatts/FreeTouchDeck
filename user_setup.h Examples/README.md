# TFT_eSPI configuration (User_Setup.h)

Before compiling and uploading the FreeTouchDeck.ino sketch, you will have to edit the `User_Setup.h` file included with the TFT_eSPI library to match the setup you have. This can be found in your Arduino skechtbook "libraries" folder. If you have not renamed the TFT_eSPI library folder, the file `User_Setup.h` can be found in **/Documents/Arduino/libraries/TFT_eSPI-master/**.

You can use the files in this repository as an example for different boards and screens. Simply rename the file that matches your combination of board and screen to `User_Setup.h` and replace the file that is in the library folder. You can also copy the contents of a file and replace the contenst in the existing `User_Setup.h`. Make sure to copy all the contents and don't leave any of the original content!

If there is a wiring diagram available, it is a .png image with the same name.

# Configuration in FreeTouchDeck.ino

Some screens also need **FreeTouchDeck.ino** to be modified. By default it is setup for the ILI9488 with resistive touch and an ESP32. 

## For the ESP32 TouchDown you will need to change the following things:

- Uncomment: `//#define USECAPTOUCH`
- Uncomment: `//#define speakerPin 26`

# Help

You can join my Discord server where I have a dedicated #freetouchdeck channel. https://discord.gg/RE3XevS
