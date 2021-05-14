#include "BMPImage.h"
#include <SPIFFS.h>
#include <FS.h>
#include "UserConfig.h"
namespace FreeTouchDeck
{
    
    BMPImage::BMPImage() 
    {
        valid = false;
    }
    BMPImage::BMPImage(const char *imageName) 
    {
        if (imageName != NULL && strlen(imageName) > 0 && SetLogoPath(imageName))
        {
            LogoName=strdup(imageName);
            GetBMPDetails();
            if (!valid)
            {
                free(FileName);
                SetLogoPath("question.bmp");
                GetBMPDetails();
            }
        }
        else
        {
            LogoName=strdup("Unknown");
            valid = false;
        }
    }
    BMPImage::~BMPImage()
    {
        if (FileName)
        {
            free(FileName);
        }
        if(LogoName)
        {
            free(LogoName);
        }
        if(ImageData)
        {
            free(ImageData);
        }
    }
    bool BMPImage::SetLogoPath(const char *imageName)
    {
        const char *logoPathTemplate = "/logos/%s";
        size_t len = strlen(logoPathTemplate) + strlen(imageName);
        FileName = (char *)malloc(len);
        memset(FileName, 0x00, len);
        snprintf(FileName, len, logoPathTemplate, imageName);
        return FileName;
    }
    void BMPImage::GetBMPDetails()
    {
        // Open File
        fs::File bmpImage = SPIFFS.open(FileName, FILE_READ);
        valid = true;
        if (!bmpImage)
        {
            Serial.print("[ERROR] Invalid image file ");
            Serial.println(FileName);
            valid = false;
        }
        if (valid && read16(bmpImage) == 0x4D42)
        {
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
                Serial.printf("[ERROR]: GetBMPDetails: Image %s is not 24 bpp\r\n", FileName);
            }
            if (Compression != 0)
            {
                Serial.printf("[ERROR]: GetBMPDetails: Image %s is compressed\r\n", FileName);
                valid = false;
            }
            if (Planes != 1)
            {
                Serial.printf("[ERROR]: GetBMPDetails: Image %s has %d planes.  Expected 1. \r\n", FileName, Planes);
                valid = false;
            }
            if (valid)
            {
                padding = (4 - ((w * 3) & 3)) & 3;
                bmpImage.seek(Offset); //skip bitmap header

                byte R, G, B;

                B = bmpImage.read();
                G = bmpImage.read();
                R = bmpImage.read();
            }
        }
        else
        {
            Serial.printf("Invalid BMP file %s\r", FileName);
            valid = false;
        }
#if defined (ESP32) && defined (CONFIG_SPIRAM_SUPPORT)
        // if (psramFound())
        // {
        //     Serial.println("Loading image into PSRAM");
        //     ImageData= (uint8_t *)heap_caps_malloc(bmpImage.size(), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
        //     if(!ImageData)
        //     {
        //         Serial.printf("[ERROR]: Unable to allocate %d bytes of PSRAM for the image file ", bmpImage.size());
        //         return;
        //     }
        //     Serial.println("Seeking to start of file");
        //     bmpImage.seek(0);
        //     Serial.println("Reading bytes into the buffer");
        //     size_t read_size = bmpImage.readBytes((char *)ImageData, bmpImage.size());
        //     Serial.printf("Loaded %d/%d bytes from image file\n",read_size, bmpImage.size() );
        // }

#else   
        ImageData=NULL;
#endif              
        Serial.println("Closing file.");
        bmpImage.close();
        Serial.println("Done");
    }

    void BMPImage::Draw(TFT_eSPI* GFX, int16_t x, int16_t y, bool transparent)
    {

        Serial.printf("Drawing file %s\n", LogoName);
        if ((x >= GFX->width()) || (y >= GFX->height()))
        {
            Serial.printf("Coordinates [%d,%d] overflow screen size\r", x, y);
            return;
        }
        if (!valid)
        {
            // won't draw an invalid image
            Serial.println("Not drawing an invalid image");
            return;
        }
        Serial.println("Getting background color");
        uint16_t BGColor = GFX->color565(R, G, B);
        bool Transparent = (BGColor == 0);        
        fs::File bmpFS;
        uint8_t * position=NULL;
        Serial.println("Checking if draw is from memory  or File");
        if(!ImageData){
            Serial.println("From File");
            fs::File bmpFS = FILESYSTEM.open(FileName, "r");
            if (!bmpFS)
            {
                Serial.print("File not found:");
                Serial.println(FileName);
                return;
            }
            bmpFS.seek(Offset);
        }
        else 
        {
            Serial.println("From PSRAM");
            position=ImageData+Offset;
        }

        uint16_t row;
        uint8_t r, g, b;

        //y += h - 1;
        bool oldSwapBytes = GFX->getSwapBytes();
        GFX->setSwapBytes(true);
        
        uint8_t lineBuffer[w * 3 + padding];
        uint16_t lx = x - w / 2;
        uint16_t ly = y + h / 2 - 1;
        uint8_t *bptr = NULL;
        uint16_t *tptr = NULL;
        for (row = 0; row < h; row++)
        {
            if(!ImageData)
            {
                Serial.println("Loading image line");
                bmpFS.read(lineBuffer, sizeof(lineBuffer));
                bptr = lineBuffer;
                tptr = (uint16_t *)lineBuffer;
                position=lineBuffer;
            }
            else
            {
                Serial.println("Setting position in buffer");
                bptr=position;
                tptr = (uint16_t *)position;
            }

            // Convert 24 to 16 bit colours
            for (uint16_t col = 0; col < w; col++)
            {
                b = *bptr++;
                g = *bptr++;
                r = *bptr++;
                *tptr++ = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
            }

            Serial.println("Pushing line to screen");
            if (Transparent || transparent)
            {
                // Push the pixel row to screen, pushImage will crop the line if needed
                // y is decremented as the BMP image is drawn bottom up
                GFX->pushImage(lx, ly--, w, 1, (uint16_t *)position, BGColor);
            }
            else
            {
                // Push the pixel row to screen, pushImage will crop the line if needed
                // y is decremented as the BMP image is drawn bottom up
                GFX->pushImage(lx, ly--, w, 1, (uint16_t *)position);
            }
            if(ImageData)
            {
                Serial.println("Advancing buffer");
                position+=sizeof(lineBuffer);
            }
        }
        GFX->setSwapBytes(oldSwapBytes);
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
    BMPImage * GetImage(const char *imageName)
    {
        BMPImage * image=NULL;
        if(!imageName || strlen(imageName)==0)
        {
            return NULL;
        }
        for(int i=0;i<ImageList.size();i++)
        {
            if(strcmp(ImageList[i]->LogoName,imageName)==0)
            {
                Serial.printf("Found image cache entry for %s\n",imageName);
                image=ImageList[i];
                break;
            }
        }
        if(!image)
        {
            Serial.printf("Image cache entry not found for %s. Adding.\n",imageName);
            image=new BMPImage(imageName);
            ImageList.push_back(image);
        }
        return image;

    }
    std::vector<BMPImage *> ImageList;
}