#pragma once
#include "globals.hpp"
#include "ImageWrapper.h"


namespace FreeTouchDeck
{
    class ImageFormatJPG : ImageWrapper
    {
    public:
        void Draw(int16_t x, int16_t y, bool transparent);
        ImageFormatJPG(const char *imageName);
        static ImageFormatJPG * GetImageInstance(const char *imageName);
        const String& GetDescription();
        uint16_t GetPixelColor();
    private:
        uint16_t PixelColor;
        static bool Transparent;
        static uint16_t BGColor;
        static String Description;
        bool LoadImageDetails();
        static bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap);
        static bool pixelcheck(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap);
        static uint16_t firstPixColor;
    };
}