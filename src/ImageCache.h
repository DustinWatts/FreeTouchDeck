#pragma once
#include "globals.hpp"
#include "ImageWrapper.h"
namespace FreeTouchDeck
{
    
    class ImageCache 
    {
        public:
        static ImageWrapper *GetImage(const std::string &imageName);

        private:
        static std::vector<ImageWrapper *> ImageList;
        static ImageInstanceGetMap_t ConstructorList;
        static ImageInstanceGet_t GetConstructorForImage(const std::string &imageName);

    };
    
}
