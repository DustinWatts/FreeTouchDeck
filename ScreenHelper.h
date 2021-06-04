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
  return (((rgb & 0xf80000) >> 8) | ((rgb & 0xfc00) >> 5) | ((rgb & 0xf8) >> 3));
}
