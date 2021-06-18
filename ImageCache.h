#pragma once
#include "globals.hpp"
#include "ImageWrapper.h"
namespace FreeTouchDeck
{
    
    class ImageCache 
    {
        public:
        static ImageWrapper *GetImage(const char *imageName);

        private:
        static std::list<ImageWrapper *> ImageList;
        static ImageInstanceGetMap_t ConstructorList;
        static ImageInstanceGet_t GetConstructorForImage(const char * imageName);

    };
    
}
