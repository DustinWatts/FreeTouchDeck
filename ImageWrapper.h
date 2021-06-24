#pragma once
#include "globals.hpp"
#include "Storage.h"
#include <functional>
namespace FreeTouchDeck
{

    class ImageWrapper
    {

    public:
        bool valid = false;
        char *FileName(char *buffer, size_t buffSize);
        static char *FileName(char *buffer, size_t buffSize,const std::string& name);
        char *CacheFileName(char *buffer, size_t buffSize);
        std::string LogoName ;
        static char * Extension[31];
        virtual const String& GetDescription()=0;
        static const std::string &GetExtension(const std::string &fileName);
        uint16_t w = 0;
        uint16_t h = 0;
        ImageWrapper(const std::string &imageName);
        virtual uint16_t GetPixelColor()=0;
        ImageWrapper();
        ~ImageWrapper();
        virtual const std::string &GetLogoName()=0;
        virtual void Draw(int16_t x, int16_t y, bool transparent)=0;
        virtual bool IsValid()=0;
    protected:
        static uint16_t read16(fs::File &f);
        static uint32_t read32(fs::File &f);
        static bool IsExtensionMatch(const char * extension,const std::string &fileName);
        bool SetNameAndPath(const std::string &imageName);
        virtual bool LoadImageDetails()=0;
    };
    typedef std::function<ImageWrapper *(const std::string &)> ImageInstanceGet_t;
    typedef std::map<const std::string, ImageInstanceGet_t> ImageInstanceGetMap_t;
}