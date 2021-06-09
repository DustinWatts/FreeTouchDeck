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
  const char *hex = html ; // remove the #
  if(ISNULLSTRING(html))
  {
    LOC_LOGE(module,"Invalid color.  Null string passed");
    return 0;
  }
  if(html[0]=='#')
  {
    hex+=1;
  }
  return (unsigned long)strtoul(hex, NULL, 16);
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
* @brief This function converts RGB888 to RGB565.
*
* @param rgb unsigned long
*
* @return unsigned int
*
* @note none
*/
unsigned int convertHTMLRGB888ToRGB565(const char * html)
{
  unsigned long rgb=convertHTMLtoRGB888(html);
  return convertRGB888ToRGB565(rgb);
}


unsigned long convertRGB656ToRGB888(unsigned long rgb565)
{
  return (unsigned long) (rgb565 & 0xF800 << 8) | (rgb565 & 0x7E0 << 5) | (rgb565 & 0x1F << 3);
}
const char * convertRGB656ToHTMLRGB888(unsigned long rgb565)
{
  static char buffer[8]={0};
  snprintf(buffer,sizeof(buffer),"#%06x",convertRGB656ToRGB888(rgb565));
  return buffer;
}
const char * convertRGB888oHTMLRGB888(unsigned long rgb888)
{
  static char buffer[8]={0};
  snprintf(buffer,sizeof(buffer),"#%06x",rgb888);
  return buffer;
}