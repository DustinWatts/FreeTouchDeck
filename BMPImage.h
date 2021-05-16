#pragma once
#include "globals.hpp"
namespace FreeTouchDeck
{
    class BMPImage
    {

    public:
        bool valid = false;
        char *FileName(char *buffer, size_t buffSize);
        char LogoName[21] = {0};
        uint16_t w = 0;
        uint16_t h = 0;
        uint32_t Offset = 0;
        uint16_t Planes = 0;
        uint16_t Depth = 0;
        uint32_t Compression = 0;
        uint32_t Colors = 0;
        byte R, G, B;
        uint16_t padding = 0;
        BMPImage(const char *imageName);
        BMPImage();
        void Draw(int16_t x, int16_t y, bool transparent);
        ~BMPImage();
        void *operator new(size_t sz)
        {
            void *p = malloc_fn(sz);
            if (!p)
                ESP_LOGE("BMPImage", "operator new %d returned null pointer! ", sz);
            return p;
        }

    private:
        bool SetNameAndPath(const char *imageName);
        bool GetBMPDetails();
        uint16_t read16(fs::File &f);
        uint32_t read32(fs::File &f);
        void ResetData();
    };
    extern BMPImage *GetImage(const char *imageName);
}