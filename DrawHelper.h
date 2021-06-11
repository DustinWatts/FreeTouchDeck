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
    unsigned int convertRGB888ToRGB565(unsigned long rgb);
    extern std::list<std::string> Messages;
}