#pragma once
#include "globals.hpp"
namespace FreeTouchDeck
{
    class ImageWrapper
    {

    public:
        bool valid = false;
        char *FileName(char *buffer, size_t buffSize);
        char *CacheFileName(char *buffer, size_t buffSize);
        char LogoName[31] = {0};
        uint16_t w = 0;
        uint16_t h = 0;
        uint32_t Offset = 0;
        uint16_t Planes = 0;
        uint16_t Depth = 0;
        uint32_t Compression = 0;
        uint32_t Colors = 0;
        byte R, G, B;
        uint16_t padding = 0;
        bool IsCache=false;
        ImageWrapper(const char *imageName);
        ImageWrapper();
        void Draw(int16_t x, int16_t y, bool transparent);
        ~ImageWrapper();
        bool HasCache();
        bool CacheFile();

    private:
        bool SetNameAndPath(const char *imageName);
        bool GetBMPDetails();
        uint16_t read16(fs::File &f);
        uint32_t read32(fs::File &f);
        void ResetData();
    };
    extern ImageWrapper *GetImage(const char *imageName);
}