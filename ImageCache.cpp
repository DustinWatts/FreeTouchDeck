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
    std::list<ImageWrapper *> ImageCache::ImageList; // reserve room for 100
    ImageInstanceGetMap_t ImageCache::ConstructorList=
    {
         {"bmp",[](const char * fileName){
             LOC_LOGD(module, "Getting Image instance from BMP constructor");
             return (ImageWrapper*)ImageFormatBMP::GetImageInstance(fileName);
             }},
         {"jpg",[](const char * fileName){
             LOC_LOGD(module, "Getting Image instance from JPG constructor");
             return (ImageWrapper*)ImageFormatJPG::GetImageInstance(fileName);
             }}
    };
    ImageInstanceGet_t ImageCache::GetConstructorForImage(const char * imageName)
    {
        const String ext = ImageWrapper::GetExtension(imageName);
        if(ext.isEmpty())
        {
            return NULL;   
        }
        LOC_LOGD(module,"Looking for constructor for extension %s", ext.c_str());
        for(auto c : ConstructorList) 
        {
            if(ext.equals(c.first.c_str()))
            {
                LOC_LOGD(module,"Found Constructor for image type %s", ext.c_str());
                return c.second;
            }
        }
        return NULL;
    }
    ImageWrapper *ImageCache::GetImage(const char *imageName)
    {
        ImageWrapper *image = NULL;
        if (!imageName || strlen(imageName) == 0)
        {
            //LOC_LOGE(module, "No image name passed");
            return NULL;
        }
        LOC_LOGV(module, "Looking for image %s", imageName);
        for (auto i : ImageList)
        {
            if (strcmp(i->LogoName, imageName) == 0)
            {
                LOC_LOGV(module, "Returning cache entry for image %s", imageName);
                image = i;
                break;
            }
            else
            {
                LOC_LOGV(module, "Cache image %s != %s", i->LogoName, imageName);
            }
        }
        if (!image)
        {
            LOC_LOGD(module, "Image cache entry not found for %s. Adding it.", imageName);
            PrintMemInfo();
            auto constructor = GetConstructorForImage(imageName);
            if(!constructor)
            {
                LOC_LOGE(module, "Unsupported file format %s", ImageWrapper::GetExtension(imageName));
            }
            else
            {
                image = constructor(imageName);
                if (image && image->valid)
                {
                    ImageList.push_back(image);
                }
                else
                {
                    LOC_LOGE(module, "Invalid image %s", imageName);
                    FREE_AND_NULL(image);
                }
                LOC_LOGD(module, "Done caching image");
                PrintMemInfo();
            }
        }
        return image;
    }
}