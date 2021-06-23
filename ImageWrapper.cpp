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
    // uint16_t ImageWrapper::GetPixelColor() {return 0;}
    // bool ImageWrapper::LoadImageDetails() {return false;}
    // const String& ImageWrapper::GetDescription() { static String strDesc=""; return strDesc;}
    // void ImageWrapper::Draw(int16_t x, int16_t y, bool transparent){LOC_LOGE(module,"Unsupported");};
    bool ImageWrapper::IsExtensionMatch(const char * extension, const std::string &name)
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
    const std::string &ImageWrapper::GetExtension(const std::string &fileName)
    {
        static std::string ext;
        ext=fileName.substr(fileName.find_last_of(".")+1);
        LOC_LOGD(module,"%s has extension %s",fileName.c_str(),ext.c_str());
        return ext;
    }
    char *ImageWrapper::FileName(char *buffer, size_t buffSize, const std::string& name)
    {
        const char *logoPathTemplate = "/logos/%s";
        size_t len = strlen(logoPathTemplate) + name.length();
        memset(buffer, 0x00, buffSize);
        snprintf(buffer, buffSize, logoPathTemplate, name.c_str());
        return buffer;
    }
    char *ImageWrapper::FileName(char *buffer, size_t buffSize)
    {
        return FileName(buffer,buffSize,LogoName);
    }

    ImageWrapper::ImageWrapper(const std::string  &imageName)
    {
        LOC_LOGD(module,"Instantiating Image Wrapper for %s",imageName.c_str());
        if (!SetNameAndPath(imageName))
        {
            LOC_LOGE(module, "Unknown image.");
            LogoName = "?";
            valid = false;
        }
    }
    bool ImageWrapper::SetNameAndPath(const std::string &imageName)
    {
        if (imageName.length() == 0)
        {
            return false;
        }
        LogoName = imageName;
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