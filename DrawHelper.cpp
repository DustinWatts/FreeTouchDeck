#include "DrawHelper.h"
#include "globals.hpp"
#include "Menu.h"
#include "MenuNavigation.h"
#include <stdarg.h>
#include "esp_bt_main.h"   // Additional BLE functionaity
#include "esp_bt_device.h" // Additional BLE functionaity
#include "Tiny3x3a2pt7b.h"
#include "Storage.h"
#include "ImageCache.h"
namespace FreeTouchDeck
{
  using namespace fs;
  TFT_eSPI tft = TFT_eSPI();
  /* ------------- Print an error message the TFT screen  ---------------- 
Purpose: This function prints an message to the TFT screen on a black 
         background. 
Input  : const char * message
         const char * module
Output : none
Note   : none
*/
  std::vector<std::string> Messages;
  void drawErrorMessage(bool stop, const char *module, const char *fmt, ...)
  {
    va_list args;
    va_start(args, fmt);
    char *message = NULL;
    const char *printmsg = fmt;
    size_t msg_size = 0;

    msg_size = vsnprintf(NULL, 0, fmt, args) + 4;
    message = (char *)malloc_fn(msg_size);

    if (!message)
    {
      ESP_LOGE(module, "Could not allocate %d bytes of memory to display message on screen", msg_size);
      stop = true;
      tft.println(fmt);
    }
    else
    {
      vsprintf(message, fmt, args);
      printmsg = message;
      LOC_LOGE(module, "%s", message);
      displayInit();
      ClearScreen();
      tft.println(printmsg);
      FREE_AND_NULL(message);
    }
    va_end(args);
  
    TFTPrintMemInfo();
    
    while (stop)
    {
      if (isTouched())
      {
        tft.println("Restarting...");
        delay(1000);
        ESP.restart();
      }
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
    SetSmallestFont(1);
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
  std::vector<const GFXfont *> FontsList;
  const GFXfont *CurrentFont = LABEL_FONT;
  const GFXfont *DefaultFont = LABEL_FONT;
  const GFXfont *GetCurrentFont()
  {
    return CurrentFont;
  }
  bool SetFont(const GFXfont *newFont)
  {
    CurrentFont = newFont;
    tft.setFreeFont(newFont);
  }
  bool SetDefaultFont()
  {
    SetFont(DefaultFont);
  }
  void InitFontsTable()
  {
    //  FontsList.push_back(&Tiny3x3a2pt7b);
    FontsList.push_back(&TomThumb);
    LOC_LOGV(module, "Pushed font TomThumb to list with address %d ", &TomThumb);
    // FontsList.push_back(&FreeSans9pt7b);
    // LOC_LOGV(module,"Pushed font FreeSans9pt7b to list with address %d ",&FreeSans9pt7b);
    FontsList.push_back(&FreeSansBold9pt7b);
    LOC_LOGV(module, "Pushed font FreeSansBold9pt7b to list with address %d ", &FreeSansBold9pt7b);
    // FontsList.push_back(&FreeSans12pt7b);
    // LOC_LOGV(module,"Pushed font FreeSans12pt7b to list with address %d ",&FreeSans12pt7b);
    FontsList.push_back(&FreeSansBold12pt7b);
    LOC_LOGV(module, "Pushed font FreeSansBold12pt7b to list with address %d ", &FreeSansBold12pt7b);
    SetDefaultFont();
  }
  bool SetSmallestFont(int whichOne)
  {
    std::vector<const GFXfont *>::iterator it = FontsList.begin();
    std::advance(it, whichOne);
    if (it != FontsList.end())
    {
      SetFont(*it);
    }
  }
  bool SetLargestFont()
  {
    SetFont(FontsList.back());
  }
  template <typename iter, typename t>
  iter iterator_from_ptr(iter first, iter last, t ptr)
  {
    while (first != last)
    {
      if (*first == ptr)
      {
        LOC_LOGV(module, "%d == %d. Returning iterator at this position", (uint32_t)*first, (uint32_t)ptr);
        return first;
      }
      LOC_LOGV(module, "%d != %d. Getting next", (uint32_t)*first, (uint32_t)ptr);
      ++first;
    }
    LOC_LOGV(module, "Returning last pointer ");
    return last;
  }
  bool SetSmallerFont()
  {
    auto newFont = iterator_from_ptr(FontsList.begin(), FontsList.end(), CurrentFont);

    if (newFont != FontsList.end())
    {
      LOC_LOGV(module, "Found current font. Getting the next one");
      std::advance(newFont, -1);
    }
    if (newFont != FontsList.end())
    {
      LOC_LOGD(module, "Setting new font");
      SetFont(*newFont);
      return true;
    }
    else
    {
      LOC_LOGD(module, "Already using smallest font");
    }
    return false;
  }
  bool SetLargerFont()
  {
    auto newFont = iterator_from_ptr(FontsList.begin(), FontsList.end(), CurrentFont);

    if (newFont != FontsList.end())
    {
      LOC_LOGV(module, "Found current font. Getting the next one");
      newFont++;
    }
    if (newFont != FontsList.end())
    {
      LOC_LOGD(module, "Setting new font");
      SetFont(*newFont);
      return true;
    }
    else
    {
      LOC_LOGD(module, "Already using largest font");
    }

    return false;
  }
  void ClearScreen()
  {
    SetSmallestFont(1);
    tft.setCursor(0, tft.fontHeight() + 1);
    tft.fillScreen(generalconfig.backgroundColour);
    tft.setTextColor(generalconfig.DefaultTextColor, generalconfig.backgroundColour);
  }
  void displayInit()
  {
    static bool initialized = false;
    if (initialized)
      return;

    initialized = true;
    // --------------- Init Display -------------------------
    LOC_LOGI(module, "Initializing display");
    // Initialise the TFT screen
    tft.init();
    // Set the rotation
    tft.setRotation(generalconfig.screenrotation);
    // Clear the screen
    // Setup the Font used for plain text
    InitFontsTable();
    ClearScreen();
    powerInit();
    LOC_LOGI(module, "Screen size is %dx%d", tft.width(), tft.height());
  }

  void PrintScreenMessage(bool clear, const char *message, ...)
  {
    static std::list<std::string> screenbuffer;
    displayInit();
    char *formatted_message = NULL;
    size_t msg_size = 0;
    va_list args;
    va_start(args, message);
    msg_size = vsnprintf(NULL, 0, message, args) + 1;
    formatted_message = (char *)malloc_fn(msg_size);

    if (!formatted_message)
    {
      ESP_LOGE(module, "Could not allocate %d bytes of memory to display message on screen", msg_size);
      tft.printf("Could not allocate %d bytes of memory to display message on screen\n", msg_size);
      tft.println(message);
      return;
    }
    vsprintf(formatted_message, message, args);
    LOC_LOGI(module, "%s", formatted_message);
    va_end(args);
    screenbuffer.push_back(formatted_message);
    FREE_AND_NULL(formatted_message);
    if (clear)
    {
      screenbuffer.clear();
      ClearScreen();
    }
    if ((tft.getCursorY() + tft.fontHeight() ) > tft.height())
    {
      // This is where we start removing top lines
      ClearScreen();
      screenbuffer.pop_front();
      for (auto m : screenbuffer)
      {
        tft.println(m.c_str());
      }
    }
    else
    {
      tft.println(screenbuffer.back().c_str());
    }
  }

  void DrawSplash()
  {
    LOC_LOGD(module, "Loading splash screen bitmap.");
    auto splash = ImageCache::GetImage("freetouchdeck_logo.jpg");
    if (splash->valid)
    {
      LOC_LOGD(module, "splash screen bitmap loaded. Drawing");
      splash->Draw(tft.width() / 2, tft.height() / 2, false);
    }
    else
    {
      LOC_LOGW(module, "Unable to draw the splash screen.");
    }
  }


  /**
* @brief This functions accepts a HTML including the # colour code 
         (eg. #FF00FF)  and returns it in RGB888 format.
*
* @param *html char (including #)
*
* @return unsigned long
*
* @note none
*/
  unsigned long convertHTMLtoRGB888(const char *html)
  {
    const char *hex = html; // remove the #
    if (ISNULLSTRING(html))
    {
      LOC_LOGE(module, "Invalid color.  Null string passed");
      return 0;
    }
    if (html[0] == '#')
    {
      hex += 1;
    }
    return (unsigned long)strtoul(hex, NULL, 16);
  }

  /**
* @brief This function converts RGB888 to RGB565.
*
* @param rgb unsigned long
*
* @return unsigned int
*
* @note none
*/
  unsigned int convertHTMLRGB888ToRGB565(const char *html)
  {
    unsigned long rgb = convertHTMLtoRGB888(html);
    return convertRGB888ToRGB565(rgb);
  }

  unsigned long convertRGB656ToRGB888(unsigned long rgb565)
  {
    return tft.color16to24(rgb565);
  }
  const char *convertRGB656ToHTMLRGB888(unsigned long rgb565)
  {
    static char buffer[8] = {0};
    snprintf(buffer, sizeof(buffer), "#%06x", convertRGB656ToRGB888(rgb565));
    return buffer;
  }
  const char *convertRGB888oHTMLRGB888(unsigned long rgb888)
  {
    static char buffer[8] = {0};
    snprintf(buffer, sizeof(buffer), "#%06x", rgb888);
    return buffer;
  }
}