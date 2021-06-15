#include "ImageWrapper.h"

#include "UserConfig.h"
#include "Storage.h"
static const char *module = "ImageWrapper";
namespace FreeTouchDeck
{

    std::list<ImageWrapper *> ImageList; // reserve room for 100
    ImageWrapper::ImageWrapper()
    {
        valid = false;
    }
    void ImageWrapper::ResetData()
    {
        MEMSET_SIZEOF(LogoName);
    }
    bool ImageWrapper::HasCache()
    {
        char FileNameBuffer[101] = {0};
        bool result = false;
        if (ftdfs->External)
        {
            CacheFileName(FileNameBuffer, sizeof(FileNameBuffer));
            return ftdfs->exists(FileNameBuffer);
        }
        return false;
    }
    bool ImageWrapper::CacheFile()
    {
        char FileNameBuffer[101] = {0};
        char cacheFileNameBuffer[101] = {0};
        bool result = true;
        if (ftdfs->External)
        {
            LOC_LOGD(module, "External storage found");
        }
        else
        {
            LOC_LOGD(module, "Internal storage does not support caching");
            return false;
        }
        if (!GetBMPDetails() || !valid)
        {
            LOC_LOGD(module, "Could not get file details. Cannot cache");
            return false;
        }
        LOC_LOGD(module, "Creating cache directory if it does not exist");
        ftdfs->mkdir("/cache");
        CacheFileName(cacheFileNameBuffer, sizeof(cacheFileNameBuffer));
        FileName(FileNameBuffer, sizeof(FileNameBuffer));
        LOC_LOGI(module, "Caching file %s to %s", FileNameBuffer, cacheFileNameBuffer);
        fs::File bmpFS = ftdfs->open(FileNameBuffer, "r");
        if (!bmpFS)
        {
            LOC_LOGE(module, "File not found: %s", FileNameBuffer);
            return false;
        }
        fs::File cacheBmpFS = ftdfs->open(cacheFileNameBuffer, "w");
        if (!cacheBmpFS)
        {
            bmpFS.close();
            LOC_LOGE(module, "Unable to open cache image");
            return false;
        }
        LOC_LOGD(module, "Caching from offset: %d", Offset);
        bmpFS.seek(Offset);

        uint8_t lineBuffer[4 * 101];
        uint8_t OutLineBuffer[4 * 101];
        size_t rLen = 0;
        do
        {
            rLen = bmpFS.read(lineBuffer, sizeof(lineBuffer));
            if (rLen == 0)
            {
                LOC_LOGD(module, "File read complete");
                break;
            }
            if (rLen % 4 > 0)
            {
                LOC_LOGE(module, "Error. Expected to read 32 bit values.  Found %d byte(s) extra ", rLen % 4);
                result = false;
                break;
            }
            LOC_LOGD(module, "Read %d bytes from source. Converting to RGB565", rLen);
            uint32_t *bptr = (uint32_t *)lineBuffer;
            uint16_t *optr = (uint16_t *)OutLineBuffer;
            // Convert 24 to 16 bit colours
            for (uint16_t v = 0; v < rLen; v += 4)
            {
                *optr = convertRGB888ToRGB565(*bptr);
                optr++;
                bptr++;
            }
            if (!cacheBmpFS.write(OutLineBuffer, rLen / 2))
            {
                result = false;
                LOC_LOGE(module, "Error writing cache file %s", cacheFileNameBuffer);
            }
        } while (result);
        if (!result)
        {
            ftdfs->remove(cacheFileNameBuffer);
        }
        else
        {
            LOC_LOGI(module, "Cache of image was successful! Size %d", cacheBmpFS.size());
        }
        bmpFS.close();
        cacheBmpFS.close();

        return result;
    }
    char *ImageWrapper::FileName(char *buffer, size_t buffSize)
    {
        const char *logoPathTemplate = "/logos/%s";
        size_t len = strlen(logoPathTemplate) + strlen((const char *)LogoName);
        memset(buffer, 0x00, buffSize);
        snprintf(buffer, buffSize, logoPathTemplate, LogoName);
        return buffer;
    }
    char *ImageWrapper::CacheFileName(char *buffer, size_t buffSize)
    {
        const char *logoPathTemplate = "/cache/%s";
        size_t len = strlen(logoPathTemplate) + strlen((const char *)LogoName);
        memset(buffer, 0x00, buffSize);
        snprintf(buffer, buffSize, logoPathTemplate, LogoName);
        return buffer;
    }
    ImageWrapper::ImageWrapper(const char *imageName)
    {
        if (SetNameAndPath(imageName))
        {
            if (!GetBMPDetails())
            {
                LOC_LOGE(module, "Unable to load bitmap file %s. ", LogoName);
            }
            // Cache WIP
            // else
            // {
            //     if (ftdfs->External && !HasCache())
            //     {
            //         LOC_LOGI(module, "File does not have a cache image on the external storage.");
            //         CacheFile();
            //     }
            // }
        }
        else
        {
            LOC_LOGE(module, "Unknown image.");
            strncpy(LogoName, "Unknown", sizeof(LogoName));
            valid = false;
        }
    }
    ImageWrapper::~ImageWrapper()
    {
        ImageWrapper::ResetData();
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
    bool ImageWrapper::GetBMPDetails()
    {
        char FileNameBuffer[101] = {0};
#if defined(ESP32) && defined(CONFIG_SPIRAM_SUPPORT)

        bool psramSupported = psramFound();
#else
        bool psramSupported = false;
#endif
        // Open File
        FileName(FileNameBuffer, sizeof(FileNameBuffer));
        LOC_LOGD(module, "Loading details from file %s", FileNameBuffer);
        fs::File imageWrapper = ftdfs->open(FileNameBuffer, FILE_READ);
        valid = true;
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
                padding = 4 - ((w * (Depth / 8)) % 4);
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
    IRAM_ATTR void ImageWrapper::Draw(int16_t x, int16_t y, bool transparent)
    {
        char FileNameBuffer[100] = {0};
        LOC_LOGD(module, "Drawing bitmap file %s at [%d,%d] ", LogoName, x, y);
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
        // if (HasCache())
        // {
        //     CacheFileName(FileNameBuffer, sizeof(FileNameBuffer));
        // }
        // else
        // {
        //     FileName(FileNameBuffer, sizeof(FileNameBuffer));
        // }
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
        // if (HasCache())
        // {
        //     uint16_t row;
        //     uint8_t r, g, b;

        //     //y += h - 1;
        //     bool oldSwapBytes = tft.getSwapBytes();
        //     tft.setSwapBytes(true);

        //     uint8_t lineBuffer[w * sizeof(uint16_t)];
        //     uint16_t lx = x - w / 2;
        //     uint16_t ly = y + h / 2 - 1;
        //     if (Transparent || transparent)
        //     {
        //         // Push the pixel row to screen, pushImage will crop the line if needed
        //         // y is decremented as the BMP image is drawn bottom up
        //         LOC_LOGV(module, "Drawing with transparent color 0x%04x", BGColor);
        //     }
        //     else
        //     {
        //         LOC_LOGV(module, "Drawing bitmap line opaque");
        //     }

        //     for (row = 0; row < h; row++)
        //     {
        //         bmpFS.read(lineBuffer, sizeof(lineBuffer));
        //         //Serial.println("Pushing line to screen");
        //         if (Transparent || transparent)
        //         {
        //             // Push the pixel row to screen, pushImage will crop the line if needed
        //             // y is decremented as the BMP image is drawn bottom up
        //             tft.pushImage(lx, ly--, w, 1, (uint16_t *)lineBuffer, BGColor);
        //         }
        //         else
        //         {
        //             // Push the pixel row to screen, pushImage will crop the line if needed
        //             // y is decremented as the BMP image is drawn bottom up
        //             tft.pushImage(lx, ly--, w, 1, (uint16_t *)lineBuffer);
        //         }
        //     }
        // }
        // else
        // {
        bmpFS.seek(Offset);
        uint16_t row;
        uint8_t r, g, b;

        //y += h - 1;

        
        uint8_t bytesPerPixel = Depth / 8;
        size_t lineBufSpace = (bytesPerPixel * w) + padding;
        // reserve 20% memory 
        size_t  maxAlloclines = heap_caps_get_free_size(MALLOC_CAP_INTERNAL)*0.20/lineBufSpace;
        //uint8_t lines = w > 80 ? 1 : 10;
        size_t bufferSize = maxAlloclines * lineBufSpace;
        uint8_t *lineBuffer = (uint8_t *)malloc(bufferSize);
        if (!lineBuffer)
        {
            LOC_LOGE(module, "Error allocating memory for image drawing!");
            tft.setSwapBytes(oldSwapBytes);
            free(lineBuffer);
            LOC_LOGV(module, "Closing bitmap file %s", LogoName);
            bmpFS.close();
            return;
        }
        LOC_LOGD(module, "Drawing picture lines: %d, bytesPerPixel: %d, lineBufSpace: %d, bufferSize: %d ", maxAlloclines, bytesPerPixel, lineBufSpace, bufferSize);
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
                uint8_t *bptr2 = bptr;
                //Serial.print("=>");
                for (uint16_t col = 0; col < w; col++)
                {

                    //Serial.printf("%s",*bptr2+bptr2[1]+bptr2[2]>0?".":"");
                    bptr2 += bytesPerPixel;
                }
                //Serial.println();
                //Serial.print("<=");

                for (uint16_t col = 0; col < w; col++)
                {
                    *tptr = convertRGB888ToRGB565(bptr, Depth);
                    //Serial.printf("%s",(*tptr)>0?".":"");

                    tptr++;
                    bptr += bytesPerPixel;
                }
                //Serial.println();
                bptr += padding;
            }
            //LOC_LOGD(module,"Converted to rgb565");
            tptr = (uint16_t *)lineBuffer;
            for (uint16_t line = readLines; line > 0; line--)
            {
                //LOC_LOGD(module,"Pushing line %d",line);
                //Serial.println("Pushing line to screen");
                if (Transparent || transparent)
                {
                    // Push the pixel row to screen, pushImage will crop the line if needed
                    // y is decremented as the BMP image is drawn bottom up
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
        tft.setSwapBytes(oldSwapBytes);
        free(lineBuffer);
        LOC_LOGV(module, "Closing bitmap file %s", LogoName);
        bmpFS.close();
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
    ImageWrapper *GetImage(const char *imageName)
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

            image = new ImageWrapper(imageName);
            if (image->valid)
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
        return image;
    }
}