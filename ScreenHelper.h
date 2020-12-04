/* ------------ Read two bytes of date from a file  ---------------- 
Purpose: This function reads chuncks of 2 bytes of data from a
         file and returns the data.
Input  : reference operator &f
Output : uint16_t
Note   : litte-endian
*/

uint16_t read16(fs::File &f)
{
  uint16_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read(); // MSB
  return result;
}

/* ------------ Read four bytes of date from a file  ---------------- 
Purpose: This function reads chuncks of 4 bytes of data from a
         file and returns the data.
Input  : reference operator &f
Output : uint32_t
Note   : litte-endian
*/

uint32_t read32(fs::File &f)
{
  uint32_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read();
  ((uint8_t *)&result)[2] = f.read();
  ((uint8_t *)&result)[3] = f.read(); // MSB
  return result;
}

/* ------------ Convert HTML colour code to RGB888  ---------------- 
Purpose: This functions accepts a HTML including the # colour code 
         (eg. #FF00FF)  and returns it in RGB888 format.
Input  : char *html (including #)
Output : unsigned long
Note   : none
*/

unsigned long convertHTMLtoRGB888(char *html)
{
  char *hex = html + 1; // remove the #
  unsigned long rgb = strtoul(hex, NULL, 16);
  return rgb;
}

/* ------------ Convert RGB888 colour code to RGB565  ---------------- 
Purpose: This function converts RGB888 to RGB565. 
Input  : unsigned long rgb
Output : unsigned int
Note   : none
*/

unsigned int convertRGB888ToRGB565(unsigned long rgb)
{
  return (((rgb & 0xf80000) >> 8) | ((rgb & 0xfc00) >> 5) | ((rgb & 0xf8) >> 3));
}

/* ------------ Drawing a transparent BMP  ---------------- 
Purpose: This function draws a transparent BMP on the TFT screen according
         to the given x and y coordinates. 
Input  : *filename, int16_t x, int16_t y
Output : none
Note   : A completely black pixel is transparent e.g. (0x0000) not drawn.
*/

void drawBmpTransparent(const char *filename, int16_t x, int16_t y)
{

  if ((x >= tft.width()) || (y >= tft.height()))
    return;

  fs::File bmpFS;

  bmpFS = FILESYSTEM.open(filename, "r");

  if (!bmpFS)
  {
    Serial.println("[WARNING]: Bitmap not found: ");
    Serial.println(filename);
    filename = "/logos/question.bmp";
    bmpFS = FILESYSTEM.open(filename, "r");
  }

  uint32_t seekOffset;
  uint16_t w, h, row;
  uint8_t r, g, b;

  if (read16(bmpFS) == 0x4D42)
  {
    read32(bmpFS);
    read32(bmpFS);
    seekOffset = read32(bmpFS);
    read32(bmpFS);
    w = read32(bmpFS);
    h = read32(bmpFS);

    if ((read16(bmpFS) == 1) && (read16(bmpFS) == 24) && (read32(bmpFS) == 0))
    {
      y += h - 1;

      bool oldSwapBytes = tft.getSwapBytes();
      tft.setSwapBytes(true);
      bmpFS.seek(seekOffset);

      uint16_t padding = (4 - ((w * 3) & 3)) & 3;
      uint8_t lineBuffer[w * 3 + padding];

      for (row = 0; row < h; row++)
      {

        bmpFS.read(lineBuffer, sizeof(lineBuffer));
        uint8_t *bptr = lineBuffer;
        uint16_t *tptr = (uint16_t *)lineBuffer;
        // Convert 24 to 16 bit colours
        for (uint16_t col = 0; col < w; col++)
        {
          b = *bptr++;
          g = *bptr++;
          r = *bptr++;
          *tptr++ = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
        }

        // Push the pixel row to screen, pushImage will crop the line if needed
        // y is decremented as the BMP image is drawn bottom up
        tft.pushImage(x, y--, w, 1, (uint16_t *)lineBuffer, TFT_BLACK);
      }
      tft.setSwapBytes(oldSwapBytes);
    }
    else
      Serial.println("BMP format not recognized.");
  }
  bmpFS.close();
}

/* ---------------- Drawing a BMP  ---------------- 
Purpose: This function draws a BMP on the TFT screen according
         to the given x and y coordinates. 
Input  : *filename, int16_t x, int16_t y
Output : none
Note   : In contradiction to drawBmpTransparent() this does draw black pixels.
*/

void drawBmp(const char *filename, int16_t x, int16_t y)
{

  if ((x >= tft.width()) || (y >= tft.height()))
    return;

  fs::File bmpFS;

  bmpFS = FILESYSTEM.open(filename, "r");

  if (!bmpFS)
  {

    Serial.print("File not found:");
    Serial.println(filename);
    return;
  }

  uint32_t seekOffset;
  uint16_t w, h, row;
  uint8_t r, g, b;

  if (read16(bmpFS) == 0x4D42)
  {
    read32(bmpFS);
    read32(bmpFS);
    seekOffset = read32(bmpFS);
    read32(bmpFS);
    w = read32(bmpFS);
    h = read32(bmpFS);

    if ((read16(bmpFS) == 1) && (read16(bmpFS) == 24) && (read32(bmpFS) == 0))
    {
      y += h - 1;

      bool oldSwapBytes = tft.getSwapBytes();
      tft.setSwapBytes(true);
      bmpFS.seek(seekOffset);

      uint16_t padding = (4 - ((w * 3) & 3)) & 3;
      uint8_t lineBuffer[w * 3 + padding];

      for (row = 0; row < h; row++)
      {

        bmpFS.read(lineBuffer, sizeof(lineBuffer));
        uint8_t *bptr = lineBuffer;
        uint16_t *tptr = (uint16_t *)lineBuffer;
        // Convert 24 to 16 bit colours
        for (uint16_t col = 0; col < w; col++)
        {
          b = *bptr++;
          g = *bptr++;
          r = *bptr++;
          *tptr++ = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
        }

        // Push the pixel row to screen, pushImage will crop the line if needed
        // y is decremented as the BMP image is drawn bottom up
        tft.pushImage(x, y--, w, 1, (uint16_t *)lineBuffer);
      }
      tft.setSwapBytes(oldSwapBytes);
    }
    else
      Serial.println("[WARNING]: BMP format not recognized.");
  }
  bmpFS.close();
}

/* --------------- Read bytes from file  ---------------- 
Purpose: This function reads a number of bytes from the given
         file at the given position.
Input  : File *p_file, int position, byte nBytes
Output : int32_t
Note   : none
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

/* ---------- Read the colour of the first pixel  ---------------- 
Purpose: This function reads the RGB565 colour of the first pixel for a
         given the logo number. The pagenumber is global.
Input  : const char *filename
Output : uint16_t
Note   : Uses readNbytesInt
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

/* ---------- Get the colour of the first pixel in an image  ---------------- 
Purpose: This function returns the RGB565 colour of the first pixel for a
         given the logo number. The pagenumber is global.
Input  : int logonumber
Output : uint16_t
Note   : Uses getBMPColor to read the actual image data.
*/

uint16_t getImageBG(int logonumber)
{

  if (pageNum == 0)
  {
    if (logonumber == 0)
    {
      return getBMPColor(screen0.logo0);
    }
    else if (logonumber == 1)
    {
      return getBMPColor(screen0.logo1);
    }
    else if (logonumber == 2)
    {
      return getBMPColor(screen0.logo2);
    }
    else if (logonumber == 3)
    {
      return getBMPColor(screen0.logo3);
    }
    else if (logonumber == 4)
    {
      return getBMPColor(screen0.logo4);
    }
    else
    {
      return 0x0000;
    }
  }
  else if (pageNum == 1)
  {
    if (logonumber == 0)
    {
      return getBMPColor(screen1.logo0);
    }
    else if (logonumber == 1)
    {
      return getBMPColor(screen1.logo1);
    }
    else if (logonumber == 2)
    {
      return getBMPColor(screen1.logo2);
    }
    else if (logonumber == 3)
    {
      return getBMPColor(screen1.logo3);
    }
    else if (logonumber == 4)
    {
      return getBMPColor(screen1.logo4);
    }
    else
    {
      return 0x0000;
    }
  }
  else if (pageNum == 2)
  {
    if (logonumber == 0)
    {
      return getBMPColor(screen2.logo0);
    }
    else if (logonumber == 1)
    {
      return getBMPColor(screen2.logo1);
    }
    else if (logonumber == 2)
    {
      return getBMPColor(screen2.logo2);
    }
    else if (logonumber == 3)
    {
      return getBMPColor(screen2.logo3);
    }
    else if (logonumber == 4)
    {
      return getBMPColor(screen2.logo4);
    }
    else
    {
      return 0x0000;
    }
  }
  else if (pageNum == 3)
  {
    if (logonumber == 0)
    {
      return getBMPColor(screen3.logo0);
    }
    else if (logonumber == 1)
    {
      return getBMPColor(screen3.logo1);
    }
    else if (logonumber == 2)
    {
      return getBMPColor(screen3.logo2);
    }
    else if (logonumber == 3)
    {
      return getBMPColor(screen3.logo3);
    }
    else if (logonumber == 4)
    {
      return getBMPColor(screen3.logo4);
    }
    else
    {
      return 0x0000;
    }
  }
  else if (pageNum == 4)
  {
    if (logonumber == 0)
    {
      return getBMPColor(screen4.logo0);
    }
    else if (logonumber == 1)
    {
      return getBMPColor(screen4.logo1);
    }
    else if (logonumber == 2)
    {
      return getBMPColor(screen4.logo2);
    }
    else if (logonumber == 3)
    {
      return getBMPColor(screen4.logo3);
    }
    else if (logonumber == 4)
    {
      return getBMPColor(screen4.logo4);
    }
    else
    {
      return 0x0000;
    }
  }
  else if (pageNum == 5)
  {
    if (logonumber == 0)
    {
      return getBMPColor(screen5.logo0);
    }
    else if (logonumber == 1)
    {
      return getBMPColor(screen5.logo1);
    }
    else if (logonumber == 2)
    {
      return getBMPColor(screen5.logo2);
    }
    else if (logonumber == 3)
    {
      return getBMPColor(screen5.logo3);
    }
    else if (logonumber == 4)
    {
      return getBMPColor(screen5.logo4);
    }
    else
    {
      return 0x0000;
    }
  }
  else if (pageNum == 6)
  {
    return 0x0000;
  }
  else
  {
    return 0x0000;
  }
}
