#include "globals.hpp"
#include "ImageCache.h"
#include "UserConfig.h"
#include "ImageFormatBMP.h"
#include "ImageFormatJPG.h"
#include "Storage.h"
#include "ImageWrapper.h"
static const char *module = "ImageCache";
namespace FreeTouchDeck
{
    std::vector<ImageWrapper *> ImageCache::ImageList; // reserve room for 100
    ImageInstanceGetMap_t ImageCache::ConstructorList =
        {
            {"bmp", [](const std::string &fileName)
             {
                 LOC_LOGD(module, "Getting Image instance from BMP constructor");
                 return (ImageWrapper *)ImageFormatBMP::GetImageInstance(fileName);
             }},
            {"jpg", [](const std::string &fileName)
             {
                 LOC_LOGD(module, "Getting Image instance from JPG constructor");
                 return (ImageWrapper *)ImageFormatJPG::GetImageInstance(fileName);
             }}};
    ImageInstanceGet_t ImageCache::GetConstructorForImage(const std::string &imageName)
    {
        const std::string ext = ImageWrapper::GetExtension(imageName);
        LOC_LOGD(module, "Looking for constructor for extension %s", ext.c_str());
        for (auto c : ConstructorList)
        {
            if (ext == c.first)
            {
                LOC_LOGD(module, "Found Constructor for image type %s", ext.c_str());
                return c.second;
            }
        }
        return NULL;
    }
    ImageWrapper *ImageCache::GetImage(const std::string &imageName)
    {
        if (ImageList.size() == 0)
        {
            // On the first call, insert a generic invalid image as the first element
            PrintMemInfo(__FUNCTION__, __LINE__);
            ImageList.push_back((ImageWrapper *)new ImageFormatJPG());
            PrintMemInfo(__FUNCTION__, __LINE__);
        }

        if(imageName.empty())
        {
            return ImageList.front();
        }
        else
        {
            LOC_LOGV(module, "Looking for image %s", imageName.c_str());
            for (auto i : ImageList)
            {
                if (i->LogoName == imageName)
                {
                    LOC_LOGV(module, "Returning cache entry for image %s", imageName.c_str());
                    return i;
                }
            }
        }
        LOC_LOGD(module, "Image cache entry not found for %s. Adding it.", imageName.c_str());
        PrintMemInfo(__FUNCTION__, __LINE__);
        ImageInstanceGet_t constructor = GetConstructorForImage(imageName);
        if (!constructor)
        {
            LOC_LOGE(module, "Unsupported file format %s", ImageWrapper::GetExtension(imageName).c_str());
            return ImageList.front();
        }
        else
        {
            PrintMemInfo(__FUNCTION__, __LINE__);
            ImageWrapper * newImage=constructor(imageName);
            LOC_LOGD(module,"Caching image name %s [%s]",newImage->LogoName.c_str(), newImage->valid?"VALID":"INVALID");
            ImageList.push_back(newImage);
            PrintMemInfo(__FUNCTION__, __LINE__);
        }
        ImageWrapper * returnedImage=ImageList.back();
        LOC_LOGD(module,"Returning image name %s [%s]",returnedImage->LogoName.c_str(), returnedImage->valid?"VALID":"INVALID");
        return ImageList.back(); // guaranteed to return at least the empty image
    }
}