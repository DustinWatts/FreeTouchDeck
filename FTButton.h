#pragma once 
#include <list>
#include <TFT_eSPI.h> // The TFT_eSPI library
#include "FTAction.h"
#include "UserConfig.h"
#include "BMPImage.h"
namespace FreeTouchDeck {
enum ButtonTypesList {
  STANDARD,
  LATCH,
  MENU
} ;
typedef ButtonTypesList ButtonTypes;

class FTButton : public TFT_eSPI_Button
{
    protected:
        char *Label=NULL;
        bool Latched=false;
        uint8_t Index=0;
        ButtonTypes ButtonType;
        bool NeedsDraw=true;
        bool IsPressed=false;
    private:
        TFT_eSPI &GFX;
        BMPImage * LatchedLogo;
        BMPImage * Logo;
        uint16_t Outline=0;
        uint8_t TextSize=0;    
        uint16_t TextColor=0;
    public:
        std::list<FTAction *> actions;
        FTButton(TFT_eSPI &gfx,String label, uint8_t index, 
                ButtonTypes buttonType, const char * logoParm, const char * latchLogoParm, 
                uint16_t outline, uint8_t textSize, uint16_t textColor);
        ~FTButton();
        uint16_t Width();
        uint16_t Height();
        void Draw(int16_t x, int16_t y,uint16_t margin, bool force=false);
        void Invalidate();
        void Press();
        void Release();
};

}