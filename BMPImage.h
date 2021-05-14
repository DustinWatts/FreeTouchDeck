#pragma once
#include <stdint.h>
#include <TFT_eSPI.h>
#include <FS.h>
#include <vector>
namespace FreeTouchDeck
{
    class BMPImage
    {

    public:
        
        bool valid = false;
        char *FileName = NULL;
        char *LogoName = NULL;
        uint16_t w = 0;
        uint16_t h = 0;
        uint32_t Offset = 0;
        uint16_t Planes = 0;
        uint16_t Depth = 0;
        uint32_t Compression = 0;
        uint32_t Colors = 0;
        byte R, G, B;
        uint16_t padding = 0;
        BMPImage( const char *imageName);
        BMPImage() ;
        void Draw(TFT_eSPI *gfx,int16_t x, int16_t y, bool transparent);
        ~BMPImage();
    private:
        bool SetLogoPath(const char *imageName);
        void GetBMPDetails();
        uint16_t read16(fs::File &f);
        uint32_t read32(fs::File &f);
        uint8_t * ImageData;
    };
    extern std::vector<BMPImage *> ImageList;
    BMPImage * GetImage(const char *imageName);
}