#pragma once
#include "globals.hpp"
#include "ImageWrapper.h"

namespace FreeTouchDeck
{
    class ImageFormatBMP : ImageWrapper
    {
    public:
        void Draw(int16_t x, int16_t y, bool transparent);
        ImageFormatBMP(const char *imageName);
        static ImageFormatBMP * GetImageInstance(const char *imageName);
        const String& GetDescription();
        uint16_t GetPixelColor();
    private:
        static String Description;
        uint32_t Offset = 0;
        uint16_t Planes = 0;
        uint16_t Depth = 0;
        uint32_t Compression = 0;
        uint32_t Colors = 0;
        byte R, G, B;
        uint16_t padding = 0;
        bool LoadImageDetails();
    };
}