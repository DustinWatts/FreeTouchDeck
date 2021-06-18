#include "UserConfig.h"
#include "Storage.h"
#include "ImageWrapper.h"
static const char *module = "ImageWrapper";
namespace FreeTouchDeck
{
    ImageWrapper::ImageWrapper()
    {
        valid = false;
    }
    bool ImageWrapper::IsExtensionMatch(const char * extension, const char * name)
    {
        char fileName[101] = {0};
        String ext = FileName(fileName,sizeof(fileName), name);
        String strExtension=String(extension);
        strExtension.toLowerCase();
        ext.toLowerCase();
        return ext.endsWith(extension);
    }

    ImageWrapper::~ImageWrapper()
    {

    }
    const char * ImageWrapper::GetExtension(const char * fileName)
    {
        const char * ext = strstr(fileName,".");
        static const char * empty="";
        if(ext)
        {
            ext++;
            LOC_LOGD(module,"%s has extension %s",fileName,ext);
        }
        else 
        {
            LOC_LOGE(module,"Extension for file %s not found", fileName);
            ext=empty;
        }
        return ext;
    }
    char *ImageWrapper::FileName(char *buffer, size_t buffSize, const char * name)
    {
        const char *logoPathTemplate = "/logos/%s";
        size_t len = strlen(logoPathTemplate) + strlen((const char *)name);
        memset(buffer, 0x00, buffSize);
        snprintf(buffer, buffSize, logoPathTemplate, name);
        return buffer;
    }
    char *ImageWrapper::FileName(char *buffer, size_t buffSize)
    {
        return FileName(buffer,buffSize,LogoName);
    }

    ImageWrapper::ImageWrapper(const char *imageName)
    {
        LOC_LOGD(module,"Instantiating Image Wrapper for %s", STRING_OR_DEFAULT(imageName,""));
        if (!SetNameAndPath(imageName))
        {
            LOC_LOGE(module, "Unknown image.");
            strncpy(LogoName, "Unknown", sizeof(LogoName));
            valid = false;
        }
    }
    bool ImageWrapper::SetNameAndPath(const char *imageName)
    {
        if (!imageName && strlen(imageName) == 0)
        {
            return false;
        }
        strncpy(LogoName, imageName, sizeof(LogoName));
        return true;
    }

    uint16_t ImageWrapper::read16(fs::File &f)
    {
        uint16_t result;
        ((uint8_t *)&result)[0] = f.read(); // LSB
        ((uint8_t *)&result)[1] = f.read(); // MSB
        return result;
    }

    uint32_t ImageWrapper::read32(fs::File &f)
    {
        uint32_t result;
        ((uint8_t *)&result)[0] = f.read(); // LSB
        ((uint8_t *)&result)[1] = f.read();
        ((uint8_t *)&result)[2] = f.read();
        ((uint8_t *)&result)[3] = f.read(); // MSB
        return result;
    }

}