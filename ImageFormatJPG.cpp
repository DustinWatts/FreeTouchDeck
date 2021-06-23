#include "globals.hpp"
#include "ImageFormatJPG.h"
#include <TJpg_Decoder.h>
static const char *module = "ImageFormatJPG";

namespace FreeTouchDeck
{

    const char *enum_to_string(JRESULT res)
    {
        switch (res)
        {
        case JDR_OK:
            return "Succeeded";
        case JDR_INTR:
            return "Interrupted by output function";
        case JDR_INP:
            return "Device error or wrong termination of input stream";
        case JDR_MEM1:
            return "Insufficient memory pool for the image";
        case JDR_MEM2:
            return "Insufficient stream input buffer";
        case JDR_PAR:
            return "Parameter error";
        case JDR_FMT1:
            return "Data format error (may be damaged data)";
        case JDR_FMT2:
            return "Right format but not supported";
        case JDR_FMT3:
            return "Not supported JPEG standard";
        default:
            return "Unknown";
            break;
        }
    }

    String ImageFormatJPG::Description = "JPG File";
    ImageFormatJPG::ImageFormatJPG(const std::string &imageName) : ImageWrapper(imageName)
    {
        LOC_LOGD(module, "Instantiating JPG file");
        if (!LoadImageDetails())
        {
            LOC_LOGE(module, "Unable to load file %s. ", LogoName.c_str());
        }
    };
    ImageFormatJPG::ImageFormatJPG():ImageWrapper(){};
    bool ImageFormatJPG::LoadImageDetails()
    {

        char FileNameBuffer[101] = {0};
#if defined(ESP32) && defined(CONFIG_SPIRAM_SUPPORT)

        bool psramSupported = psramFound();
#else
        bool psramSupported = false;
#endif
        // Open File
        FileName(FileNameBuffer, sizeof(FileNameBuffer));
        valid = false;
        LOC_LOGD(module, "Loading details from file %s", FileNameBuffer);
        PrintMemInfo(__FUNCTION__, __LINE__);
        JRESULT res = JDR_OK;
        File imageFile = ftdfs->open(FileNameBuffer, FILE_READ);
        if (!imageFile)
        {
            LOC_LOGE(module, "Error opening %s", FileNameBuffer);
            return false;
        }
        //LOC_LOGD(module, "Getting size from SD card file");
        res = TJpgDec.getFsJpgSize(&w, &h, imageFile);

        if (res == JDR_OK)
        {
            // need to open the file again, since get size will close it
            TJpgDec.setCallback(ImageFormatJPG::pixelcheck);
            //LOC_LOGD(module, "Getting pixel color from SD file");
            TJpgDec.drawFsJpg(0, 0, imageFile);

            PixelColor = firstPixColor;
            valid = true;
            LOC_LOGD(module, "JPG File dimensions are %dx%d, background color is RGB565 0x%04d", w, h, PixelColor);
        }
        else
        {
            LOC_LOGE(module, "Unable to get JPG size. Return code was %s ", enum_to_string(res));
        }
        imageFile.close();
        PrintMemInfo(__FUNCTION__, __LINE__);
        // don't close the file; the getSdJpgSize call does it
        LOC_LOGV(module, "Done parsing file");
        return valid;
    }
    bool ImageFormatJPG::IsValid()
    {
        return valid;
    }
    const std::string &ImageFormatJPG::GetLogoName()
    {
        return LogoName;
    }
    void ImageFormatJPG::Draw(int16_t x, int16_t y, bool transparent)
    {
        char FileNameBuffer[100] = {0};
        LOC_LOGD(module, "Drawing jpg file %s at [%d,%d] ", LogoName.c_str(), x, y);
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
        BGColor = PixelColor;
        Transparent = ((BGColor == TFT_BLACK) || transparent);
        FileName(FileNameBuffer, sizeof(FileNameBuffer));
        bool oldSwapBytes = tft.getSwapBytes();
        tft.setSwapBytes(true);

        uint16_t cornerX = max((uint16_t)(x - (w / 2)), (uint16_t)0);
        uint16_t cornerY = max((uint16_t)(y - (h / 2)), (uint16_t)0);

        TJpgDec.setCallback(ImageFormatJPG::tft_output);
        File imageFile = ftdfs->open(FileNameBuffer, FILE_READ);
        if (!imageFile)
        {
            LOC_LOGE(module, "Error opening %s", FileNameBuffer);
            return;
        }

        TJpgDec.drawFsJpg(cornerX, cornerY, imageFile);
        imageFile.close();
        PrintMemInfo(__FUNCTION__, __LINE__);
        LOC_LOGV(module, "Closing bitmap file %s", LogoName.c_str());
        tft.setSwapBytes(oldSwapBytes);
    }
    const String &ImageFormatJPG::GetDescription()
    {
        return Description;
    }
    ImageFormatJPG * ImageFormatJPG::GetImageInstance(const std::string &imageName)
    {
        LOC_LOGD(module, "JPG handler checking if extension of %s is a match for 'JPG'", imageName.c_str());
        if (!IsExtensionMatch("jpg", imageName))
        {
            LOC_LOGE(module, "Invalid file extension. ");
            return new ImageFormatJPG();
        }
        return new ImageFormatJPG(imageName);
    }
    uint16_t ImageFormatJPG::GetPixelColor()
    {
        return PixelColor;
    }

    // This next function will be called during decoding of the jpeg file to
    // render each block to the TFT.  If you use a different TFT library
    // you will need to adapt this function to suit.
    bool ImageFormatJPG::tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t *bitmap)
    {
        // Stop further decoding as image is running off bottom of screen
        if (y >= tft.height())
        {
            return 0;
        }

        // This function will clip the image block rendering automatically at the TFT boundaries

        if (Transparent)
        {
            tft.pushImage(x, y, w, h, bitmap, BGColor);
        }
        else
        {
            // Push the pixel row to screen, pushImage will crop the line if needed
            // y is decremented as the BMP image is drawn bottom up
            tft.pushImage(x, y, w, h, bitmap);
        }

        // Return 1 to decode next block
        return 1;
    }
    bool ImageFormatJPG::Transparent = false;
    uint16_t ImageFormatJPG::BGColor = TFT_BLACK;
    uint16_t ImageFormatJPG::firstPixColor = 0;
    bool ImageFormatJPG::pixelcheck(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t *bitmap)
    {
        firstPixColor = bitmap[0];
        return 0;
    }

}