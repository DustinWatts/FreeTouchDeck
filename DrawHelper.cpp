#include "DrawHelper.h"
#include "globals.hpp"
#include "Menu.h"
#include "MenuNavigation.h"
#include <stdarg.h>
#include "esp_bt_main.h"   // Additional BLE functionaity
#include "esp_bt_device.h" // Additional BLE functionaity
using namespace FreeTouchDeck;

/* ------------- Print an error message the TFT screen  ---------------- 
Purpose: This function prints an message to the TFT screen on a black 
         background. 
Input  : const char * message
         const char * module
Output : none
Note   : none
*/


void drawErrorMessage(bool stop, const char *module, const char *fmt, ...)
{
  displayInit();
  char *message = NULL;
  size_t msg_size = 0;
  if(stop)
  {
    SetActiveScreen("criticalmessage");
  }
  
  va_list args;
  va_start(args, fmt);
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(20, 20);
  tft.setTextFont(2);
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  msg_size = vsnprintf(NULL, 0, fmt, args) + 1;
  message = (char *)malloc_fn(msg_size);

  if (!message)
  {
    ESP_LOGE(module, "Could not allocate %d bytes of memory to display message on screen", msg_size);
    tft.println(fmt);
    stop=true;
  }
  else
  {
    vsprintf(message, fmt, args);
    LOC_LOGE(module, "%s", message);
    tft.println(message);
  }
  va_end(args);
  free(message);
  while (stop)
  {
    yield();
  }
  // hold a bit to let user read message
  // if it's not a full stop
    delay(5000);
}
/* ------------- Print an error message the TFT screen  ---------------- 
Purpose: This function prints an message to the TFT screen on a black 
         background. 
Input  : String message
Output : none
Note   : none
*/

void drawErrorMessage(String message)
{

  tft.fillScreen(TFT_BLACK);
  tft.setCursor(20, 20);
  tft.setTextFont(2);
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.println(message);
}

/**
* @brief This function gets the Bluetooth device address and prints it to the serial monitor
         and the TFT screen in a 6 byte format, seperating each byte by ":".
*
* @param none
*
* @return none
*
* @note e.g. 00:11:22:33:22:EE
*/

void printDeviceAddress()
{

  const uint8_t *point = esp_bt_dev_get_address();

  for (int i = 0; i < 6; i++)
  {

    char str[3];

    sprintf(str, "%02X", (int)point[i]);
    //Serial.print(str);
    tft.print(str);

    if (i < 5)
    {
      // Serial.print(":");
      tft.print(":");
    }
  }
}

/**
* @brief This function prints some information about the current version 
         and setup of FreetouchDeck to the TFT screen.
*
* @param none
*
* @return none
*
* @note none
*/
bool printinfo(FTAction *dummy)
{
  tft.setCursor(1, 3);
  tft.setTextFont(2);
  if (tft.width() < 480)
  {
    tft.setTextSize(1);
  }
  else
  {
    tft.setTextSize(2);
  }
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.printf("Version: %s\n", versionnumber);

#ifdef touchInterruptPin
  if (generalconfig.sleepenable)
  {
    tft.println("Sleep: Enabled");
    tft.printf("Sleep timer: %u minutes\n", generalconfig.sleeptimer);
  }
  else
  {
    tft.println("Sleep: Disabled");
  }
#else
  tft.println("Sleep: Disabled");
#endif

#ifdef speakerPin
  if (generalconfig.beep)
  {
    tft.println("Speaker: Enabled");
  }
  else
  {
    tft.println("Speaker: Disabled");
  }
#else
  tft.println("Speaker: Disabled");
#endif

  tft.print("Free Storage: ");
  float freemem = SPIFFS.totalBytes() - SPIFFS.usedBytes();
  tft.print(freemem / 1000);
  tft.println(" kB");
  tft.print("BLE Keyboard version: ");
  tft.println(BLE_KEYBOARD_VERSION);
  tft.print("ArduinoJson version: ");
  tft.println(ARDUINOJSON_VERSION);
  tft.print("TFT_eSPI version: ");
  tft.println(TFT_ESPI_VERSION);
  tft.println("ESP-IDF: ");
  tft.println(esp_get_idf_version());
  return true;
}
