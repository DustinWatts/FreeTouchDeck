#include "globals.hpp"
#include "ImageFormatBMP.h"
static const char *module = "ImageFormatBMP";

namespace FreeTouchDeck
{
    String ImageFormatBMP::Description="Bitmap File";
    ImageFormatBMP::ImageFormatBMP():ImageWrapper()
    {

    }
    ImageFormatBMP::ImageFormatBMP(const std::string &imageName)  : ImageWrapper(imageName)
    {
        LOC_LOGD(module, "Instantiating BMP file");
        if (!LoadImageDetails())
        {
            LOC_LOGE(module, "Unable to load file %s. ", LogoName.c_str());
        }
    };
    bool ImageFormatBMP::LoadImageDetails()
    {
        
        char FileNameBuffer[101] = {0};
#if defined(ESP32) && defined(CONFIG_SPIRAM_SUPPORT)

        bool psramSupported = psramFound();
#else
        bool psramSupported = false;
#endif
        // Open File
        FileName(FileNameBuffer, sizeof(FileNameBuffer));
        valid = true;
        LOC_LOGD(module, "Loading details from file %s", FileNameBuffer);
        fs::File imageWrapper = ftdfs->open(FileNameBuffer, FILE_READ);
        if (!imageWrapper || imageWrapper.size() == 0)
        {
            LOC_LOGE(module, "Could not open file %s", FileNameBuffer);
            valid = false;
            return valid;
        }

        if (valid && read16(imageWrapper) == 0x4D42)
        {
            LOC_LOGV(module, "Valid bitmap header signature found");
            read32(imageWrapper);
            read32(imageWrapper);
            Offset = read32(imageWrapper);              // start of image data
            uint32_t headerSize = read32(imageWrapper); // header size
            w = read32(imageWrapper);
            h = read32(imageWrapper);
            Planes = read16(imageWrapper);
            Depth = read16(imageWrapper); // Bits per pixel
            if (headerSize > 12)
            {
                Compression = read32(imageWrapper);
                (void)read32(imageWrapper);    // Raw bitmap data size; ignore
                (void)read32(imageWrapper);    // Horizontal resolution, ignore
                (void)read32(imageWrapper);    // Vertical resolution, ignore
                Colors = read32(imageWrapper); // Number of colors in palette, or 0 for 2^depth
            }
            if (Depth != 24)
            {
                valid = false;
                LOC_LOGE(module, "Unsupported bit depth %d for image %s. Image should be 24bpp.", Depth, imageWrapper.name());
            }
            if (Compression != 0)
            {
                LOC_LOGE(module, "Compression not supported for %s. Image should be uncompressed.", imageWrapper.name());
                valid = false;
            }
            if (Planes != 1)
            {
                LOC_LOGE(module, "Unsupported number of planes %d for image %s.", Planes, imageWrapper.name());
                valid = false;
            }
            if (valid)
            {
                // each row in a bmp pixel array is padded to a multiple of 4 bytes
                //padding = (4 - ((w * 3) & 3)) & 3;
                padding = ((w * (Depth / 8)) % 4);
                padding = padding > 0 ? 4 - padding : padding;
                imageWrapper.seek(Offset); //skip bitmap header
                LOC_LOGD(module, "Depth: %d width is %d, padded bytes: %d, padding: %d", Depth, w, (Depth / 8 * w) + padding, padding);
                B = imageWrapper.read();
                G = imageWrapper.read();
                R = imageWrapper.read();
            }
        }
        else
        {
            LOC_LOGE(module, "Invalid bitmap file %s. Signature 0x4D42 not found in header", imageWrapper.name());
            valid = false;
        }
        LOC_LOGV(module, "Closing file %s", imageWrapper.name());
        imageWrapper.close();

        LOC_LOGV(module, "Done parsing file");
        return valid;
    }
    // move the drawing function to IRAM in an attempt to speed up drawing
     void ImageFormatBMP::Draw(int16_t x, int16_t y, bool transparent)
    {
        char FileNameBuffer[100] = {0};
        LOC_LOGD(module, "Drawing bitmap file %s at [%d,%d] ", LogoName.c_str(), x, y);
        if ((x >= tft.width()) || (y >= tft.height()))
        {
            LOC_LOGE(module, "Coordinates [%d,%d] overflow screen size", x, y);
            return;
        }
        if (!valid)
        {
            // won't draw an invalid image
            LOC_LOGW(module, "Not drawing an invalid image");
            return;
        }

        LOC_LOGV(module, "Getting background color");
        uint16_t BGColor = tft.color565(R, G, B);
        bool Transparent = ((BGColor == TFT_BLACK) || transparent);
        FileName(FileNameBuffer, sizeof(FileNameBuffer));
        bool oldSwapBytes = tft.getSwapBytes();
        tft.setSwapBytes(true);

        LOC_LOGV(module, "Opening file %s", FileNameBuffer);
        fs::File bmpFS = ftdfs->open(FileNameBuffer, "r");
        if (!bmpFS)
        {
            LOC_LOGE(module, "File not found: %s", FileNameBuffer);
            return;
        }
        LOC_LOGV(module, "Seeking offset: %d", Offset);

        bmpFS.seek(Offset);
        uint16_t row;
        uint8_t r, g, b;

        uint8_t bytesPerPixel = Depth / 8;
        size_t lineBufSpace = (bytesPerPixel * w) + padding;
        // reserve buffer memory enough to load all lines or up to the max buffer allowed
        size_t maxAlloclines = min(((size_t)(heap_caps_get_free_size(MALLOC_CAP_INTERNAL) * BITMAP_BUFFER_FREE_RAM_PCT)) / lineBufSpace, (size_t)h);
        //uint8_t lines = w > 80 ? 1 : 10;
        size_t bufferSize = maxAlloclines * lineBufSpace;
        uint8_t *lineBuffer = (uint8_t *)malloc(bufferSize);
        if (!lineBuffer)
        {
            LOC_LOGE(module, "Error allocating %d bytes of buffer for image drawing!", bufferSize);
            tft.setSwapBytes(oldSwapBytes);
            free(lineBuffer);
            LOC_LOGV(module, "Closing bitmap file %s", LogoName.c_str());
            bmpFS.close();
            return;
        }
        LOC_LOGV(module, "Drawing picture lines: %d, bytesPerPixel: %d, lineBufSpace: %d, bufferSize: %d ", maxAlloclines, bytesPerPixel, lineBufSpace, bufferSize);
        uint16_t lx = x - w / 2;
        uint16_t ly = y + h / 2 - 1;
        uint8_t *bptr = NULL;
        uint16_t *tptr = NULL;
        if (Transparent || transparent)
        {
            // Push the pixel row to screen, pushImage will crop the line if needed
            // y is decremented as the BMP image is drawn bottom up
            LOC_LOGV(module, "Drawing with transparent color 0x%04x", BGColor);
        }
        else
        {
            LOC_LOGV(module, "Drawing bitmap line opaque");
        }

        for (row = 0; row < h; row += maxAlloclines)
        {
            size_t readBuf = bmpFS.read(lineBuffer, bufferSize);
            bptr = lineBuffer;
            tptr = (uint16_t *)lineBuffer;
            uint8_t readLines = readBuf / lineBufSpace;
            //LOC_LOGD(module, "Read %d bytes (%d lines) from bmp",readBuf, readLines);
            // Convert 24 to 16 bit colours
            for (uint16_t r = 0; r < readLines; r++)
            {

#ifdef DRAWDEBUG
                uint8_t *bptr2 = bptr;
                Serial.print("=>");
                for (uint16_t col = 0; col < w; col++)
                {

                    Serial.printf("%s", *bptr2 + bptr2[1] + bptr2[2] > 0 ? "." : "");
                    bptr2 += bytesPerPixel;
                }
                Serial.println();
                Serial.print("<=");
                for (uint16_t col = 0; col < w; col++)
                {
                    *tptr = convertRGB888ToRGB565(bptr, Depth);
                    Serial.printf("%s", (*tptr) > 0 ? "." : "");
                    tptr++;
                    bptr += bytesPerPixel;
                }
                Serial.println();
#endif
                for (uint16_t col = 0; col < w; col++)
                {
                    *tptr = convertRGB888ToRGB565(bptr, Depth);
                    tptr++;
                    bptr += bytesPerPixel;
                }
                bptr += padding;
            }
            tptr = (uint16_t *)lineBuffer;
            for (uint16_t line = readLines; line > 0; line--)
            {
                if (Transparent || transparent)
                {
                    tft.pushImage(lx, ly--, w, 1, (uint16_t *)tptr, BGColor);
                }
                else
                {
                    // Push the pixel row to screen, pushImage will crop the line if needed
                    // y is decremented as the BMP image is drawn bottom up
                    tft.pushImage(lx, ly--, w, 1, (uint16_t *)tptr);
                }
                tptr += w;
            }
        }
        // }
        free(lineBuffer);
        LOC_LOGV(module, "Closing bitmap file %s", LogoName.c_str());
        bmpFS.close();
        tft.setSwapBytes(oldSwapBytes);
    }
    bool ImageFormatBMP::IsValid()
    {
        return valid;
    }
    const std::string &ImageFormatBMP::GetLogoName()
    {
        return LogoName;
    }
    const String& ImageFormatBMP::GetDescription()
    {
        return Description;
    }
     ImageFormatBMP * ImageFormatBMP::GetImageInstance(const std::string &imageName)
     {
         LOC_LOGD(module,"BMP handler checking if extension of %s is a match for 'bmp'",imageName);
         if(!IsExtensionMatch("bmp",imageName.c_str()))
         {
             LOC_LOGE(module, "Invalid file extension. ");
             return new ImageFormatBMP();
         }
         return new ImageFormatBMP(imageName);
     }
     uint16_t ImageFormatBMP::GetPixelColor()
     {
        return tft.color565(R, G, B);
     }
   
        
}