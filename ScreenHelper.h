#include <FS.h>
#include <FSImpl.h>
#include <vfs_api.h>




/**
* @brief This functions accepts a HTML including the # colour code 
         (eg. #FF00FF)  and returns it in RGB888 format.
*
* @param *html char (including #)
*
* @return unsigned long
*
* @note none
*/
unsigned long convertHTMLtoRGB888(const char *html)
{
  const char *hex = html + 1; // remove the #
  unsigned long rgb = strtoul(hex, NULL, 16);
  return rgb;
}

/**
* @brief This function converts RGB888 to RGB565.
*
* @param rgb unsigned long
*
* @return unsigned int
*
* @note none
*/
unsigned int convertRGB888ToRGB565(unsigned long rgb)
{
  return (((rgb & 0xf80000) >> 8) | ((rgb & 0xfc00) >> 5) | ((rgb & 0xf8) >> 3));
}



/**
* @brief This function reads a number of bytes from the given
         file at the given position.
*
* @param *p_file File
* @param position int
* @param nBytes byte 
*
* @return int32_t
*
* @note none
*/
int32_t readNbytesInt(File *p_file, int position, byte nBytes)
{
  if (nBytes > 4)
    return 0;

  p_file->seek(position);

  int32_t weight = 1;
  int32_t result = 0;
  for (; nBytes; nBytes--)
  {
    result += weight * p_file->read();
    weight <<= 8;
  }
  return result;
}

/**
* @brief This function reads the RGB565 colour of the first pixel for a
         given the logo number. The pagenumber is global.
*
* @param *filename const char
*
* @return uint16_t
*
* @note Uses readNbytesInt
*/
uint16_t getBMPColor(const char *filename)
{

  // Open File
  File bmpImage;
  bmpImage = SPIFFS.open(filename, FILE_READ);

  int32_t dataStartingOffset = readNbytesInt(&bmpImage, 0x0A, 4);
  int16_t pixelsize = readNbytesInt(&bmpImage, 0x1C, 2);

  if (pixelsize != 24)
  {
    Serial.println("[WARNING]: getBMPColor: Image is not 24 bpp");
    return 0x0000;
  }

  bmpImage.seek(dataStartingOffset); //skip bitmap header

  byte R, G, B;

  B = bmpImage.read();
  G = bmpImage.read();
  R = bmpImage.read();

  bmpImage.close();

  return tft.color565(R, G, B);
}

