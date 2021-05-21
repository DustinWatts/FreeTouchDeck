#include "BMPImage.h"

#include "UserConfig.h"

static const char *module = "BMPImage";
namespace FreeTouchDeck
{
    std::list<BMPImage *>ImageList; // reserve room for 100
    uint8_t ImageCount = 0;
    BMPImage::BMPImage()
    {
        valid = false;
    }
    void BMPImage::ResetData()
    {
        MEMSET_SIZEOF(LogoName);
    }
    char *BMPImage::FileName(char *buffer, size_t buffSize)
    {
        const char *logoPathTemplate = "/logos/%s";
        size_t len = strlen(logoPathTemplate) + strlen((const char *)LogoName);
        memset(buffer, 0x00, buffSize);
        snprintf(buffer, buffSize, logoPathTemplate, LogoName);
        return buffer;
    }
    BMPImage::BMPImage(const char *imageName)
    {
        if (SetNameAndPath(imageName))
        {
            if (!GetBMPDetails())
            {
                ESP_LOGE(module, "Unable to load bitmap file %s. ", LogoName);
            }
        }
        else
        {
            ESP_LOGE(module, "Unknown image.");
            strncpy(LogoName, "Unknown", sizeof(LogoName));
            valid = false;
        }
    }
    BMPImage::~BMPImage()
    {
        BMPImage::ResetData();
    }
    bool BMPImage::SetNameAndPath(const char *imageName)
    {
        if (!imageName && strlen(imageName) == 0)
        {
            return false;
        }
        strncpy(LogoName, imageName, sizeof(LogoName));
        return true;
    }
    bool BMPImage::GetBMPDetails()
    {
        char FileNameBuffer[101] = {0};
#if defined(ESP32) && defined(CONFIG_SPIRAM_SUPPORT)

        bool psramSupported = psramFound();
#else
        bool psramSupported = false;
#endif
        // Open File
        FileName(FileNameBuffer, sizeof(FileNameBuffer));
        ESP_LOGD(module, "Loading details from file %s", FileNameBuffer);
        fs::File bmpImage = SPIFFS.open(FileNameBuffer, FILE_READ);
        valid = true;
        if (!bmpImage || bmpImage.size() == 0)
        {
            ESP_LOGE(module, "Could not open file %s", FileNameBuffer);
            valid = false;
            return valid;
        }
        if (valid && read16(bmpImage) == 0x4D42)
        {
            ESP_LOGV(module, "Valid bitmap header signature found");
            read32(bmpImage);
            read32(bmpImage);
            Offset = read32(bmpImage);              // start of image data
            uint32_t headerSize = read32(bmpImage); // header size
            w = read32(bmpImage);
            h = read32(bmpImage);
            Planes = read16(bmpImage);
            Depth = read16(bmpImage); // Bits per pixel
            if (headerSize > 12)
            {
                Compression = read32(bmpImage);
                (void)read32(bmpImage);    // Raw bitmap data size; ignore
                (void)read32(bmpImage);    // Horizontal resolution, ignore
                (void)read32(bmpImage);    // Vertical resolution, ignore
                Colors = read32(bmpImage); // Number of colors in palette, or 0 for 2^depth
            }
            if (Depth != 24)
            {
                valid = false;
                ESP_LOGE(module, "Unsupported bit depth %d for image %s. Image should be 24bpp.", Depth, bmpImage.name());
            }
            if (Compression != 0)
            {
                ESP_LOGE(module, "Compression not supported for %s. Image should be uncompressed.", bmpImage.name());
                valid = false;
            }
            if (Planes != 1)
            {
                ESP_LOGE(module, "Unsupported number of planes %d for image %s.", Planes, bmpImage.name());
                valid = false;
            }
            if (valid)
            {
                padding = (4 - ((w * 3) & 3)) & 3;
                bmpImage.seek(Offset); //skip bitmap header

                B = bmpImage.read();
                G = bmpImage.read();
                R = bmpImage.read();
            }
        }
        else
        {
            ESP_LOGE(module, "Invalid bitmap file %s. Signature 0x4D42 not found in header", bmpImage.name());
            valid = false;
        }
        ESP_LOGV(module, "Closing file %s", bmpImage.name());
        bmpImage.close();
        ESP_LOGV(module, "Done parsing file");
        return valid;
    }

    void BMPImage::Draw(int16_t x, int16_t y, bool transparent)
    {
        char FileNameBuffer[100] = {0};
        ESP_LOGD(module, "Drawing bitmap file %s", LogoName);
        if ((x >= tft.width()) || (y >= tft.height()))
        {
            ESP_LOGE(module, "Coordinates [%d,%d] overflow screen size", x, y);
            return;
        }
        if (!valid)
        {
            // won't draw an invalid image
            ESP_LOGW(module, "Not drawing an invalid image");
            return;
        }
        ESP_LOGD(module, "Getting background color");
        uint16_t BGColor = tft.color565(R, G, B);
        bool Transparent = ((BGColor == 0) || transparent);
        FileName(FileNameBuffer, sizeof(FileNameBuffer));
        ESP_LOGD(module, "Opening file %s", FileNameBuffer);
        fs::File bmpFS = FILESYSTEM.open(FileNameBuffer, "r");
        if (!bmpFS)
        {
            ESP_LOGE(module, "File not found: %s", FileNameBuffer);
            return;
        }
        ESP_LOGV(module, "Seeking offset: %d", Offset);
        bmpFS.seek(Offset);

        uint16_t row;
        uint8_t r, g, b;

        //y += h - 1;
        bool oldSwapBytes = tft.getSwapBytes();
        tft.setSwapBytes(true);

        uint8_t lineBuffer[w * 3 + padding];
        uint16_t lx = x - w / 2;
        uint16_t ly = y + h / 2 - 1;
        uint8_t *bptr = NULL;
        uint16_t *tptr = NULL;
        if (Transparent || transparent)
        {
            // Push the pixel row to screen, pushImage will crop the line if needed
            // y is decremented as the BMP image is drawn bottom up
            ESP_LOGV(module, "Drawing with transparent color 0x%04x", BGColor);
        }
        else
        {
            ESP_LOGV(module, "Drawing bitmap line opaque");
        }

        for (row = 0; row < h; row++)
        {
            bmpFS.read(lineBuffer, sizeof(lineBuffer));
            bptr = lineBuffer;
            tptr = (uint16_t *)lineBuffer;

            // Convert 24 to 16 bit colours
            for (uint16_t col = 0; col < w; col++)
            {
                b = *bptr++;
                g = *bptr++;
                r = *bptr++;
                *tptr++ = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
            }

            //Serial.println("Pushing line to screen");
            if (Transparent || transparent)
            {
                // Push the pixel row to screen, pushImage will crop the line if needed
                // y is decremented as the BMP image is drawn bottom up
                tft.pushImage(lx, ly--, w, 1, (uint16_t *)lineBuffer, BGColor);
            }
            else
            {
                // Push the pixel row to screen, pushImage will crop the line if needed
                // y is decremented as the BMP image is drawn bottom up
                tft.pushImage(lx, ly--, w, 1, (uint16_t *)lineBuffer);
            }
        }
        tft.setSwapBytes(oldSwapBytes);
        ESP_LOGD(module, "Closing bitmap file %s", LogoName);
        bmpFS.close();
    }

    uint16_t BMPImage::read16(fs::File &f)
    {
        uint16_t result;
        ((uint8_t *)&result)[0] = f.read(); // LSB
        ((uint8_t *)&result)[1] = f.read(); // MSB
        return result;
    }

    uint32_t BMPImage::read32(fs::File &f)
    {
        uint32_t result;
        ((uint8_t *)&result)[0] = f.read(); // LSB
        ((uint8_t *)&result)[1] = f.read();
        ((uint8_t *)&result)[2] = f.read();
        ((uint8_t *)&result)[3] = f.read(); // MSB
        return result;
    }
    BMPImage *GetImage(const char *imageName)
    {
        BMPImage *image = NULL;
        if (!imageName || strlen(imageName) == 0)
        {
            //ESP_LOGE(module, "No image name passed");
            return NULL;
        }
        ESP_LOGV(module, "Looking for image %s", imageName);
        for (auto i: ImageList )
        {
            if (strcmp(i->LogoName, imageName) == 0)
            {
                ESP_LOGV(module, "Returning cache entry for image %s", imageName);
                image = i;
                break;
            }
            else
            {
                ESP_LOGV(module, "Cache image %s != %s", i->LogoName, imageName);
            }
        }
        if (!image)
        {
            ESP_LOGD(module, "Image cache entry not found for %s. Adding it.", imageName);
            PrintMemInfo();

            image = new BMPImage(imageName);
            if (image->valid)
            {
                ImageList.push_back(image);
            }
            else
            {
                ESP_LOGE(module, "Invalid image %s", imageName);
                FREE_AND_NULL(image);
            }
            ESP_LOGD(module,"Done caching image");
            PrintMemInfo();
        }
        return image;
    }

}