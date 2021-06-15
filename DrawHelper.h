#pragma once
#include "globals.hpp"
#include <TFT_eSPI.h> // The TFT_eSPI library
namespace FreeTouchDeck
{
    void drawErrorMessage(bool stop, const char *module, const char *fmt, ...);
    void printDeviceAddress();
    void LoadFontsTable();
    void displayInit();
    void PrintScreenMessage(const char *message);
    void drawErrorMessage(String message);
    const GFXfont *GetCurrentFont();
    bool SetFont(const GFXfont *newFont);
    bool SetDefaultFont();
    void InitFontsTable();
    bool SetSmallerFont();
    bool SetSmallestFont(int whichOne = 0);
    bool SetLargestFont();
    bool SetLargerFont();
    void DrawSplash();
    void CacheBitmaps();
    extern TFT_eSPI tft;
    const char *convertRGB656ToHTMLRGB888(unsigned long rgb565);
    unsigned int convertHTMLRGB888ToRGB565(const char *html);
    unsigned long convertHTMLtoRGB888(const char *html);
    const char *convertRGB888oHTMLRGB888(unsigned long rgb888);
    unsigned long convertRGB656ToRGB888(unsigned long rgb565);
  /**
* @brief This function converts RGB888 to RGB565.
*
* @param rgb unsigned long
*
* @return unsigned int
*
* @note none
*/
  inline unsigned int convertRGB888ToRGB565(unsigned long rgb)
  {
    return (((rgb & 0xf80000) >> 8) | ((rgb & 0xfc00) >> 5) | ((rgb & 0xf8) >> 3));
  }
inline unsigned int convertRGB888ToRGB565(uint8_t * rgb, uint8_t depth)
  {
      // Values are received in LSB
      uint8_t * pV=(uint8_t *)rgb;
      switch (depth)
      {
      case 24:
        // colors are stored as BGR
        //*tptr++ = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
          return (pV[0] >>3) |  ((pV[1] & 0xfc)<<3)| ((pV[2] & 0xF8)<< 8);
          break;
      default:
          break;
      }
      return 0;
  }
    extern std::list<std::string> Messages;
}