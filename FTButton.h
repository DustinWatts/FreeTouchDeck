#pragma once

#include "globals.hpp"
#include "FTAction.h"
#include "UserConfig.h"
#include "BMPImage.h"
namespace FreeTouchDeck
{
    enum class ButtonTypes
    {
        STANDARD,
        LATCH,
        MENU

    };
    const char *enum_to_string(ButtonTypes type);
    

    class FTButton : public TFT_eSPI_Button
    {
    protected:
        char *Label = NULL;
        bool Latched = false;
        uint8_t Index = 0;
        ButtonTypes ButtonType;
        bool NeedsDraw = true;
        bool IsPressed = false;

    private:
        BMPImage *LatchedLogo;
        BMPImage *Logo;
        uint16_t Outline = 0;
        uint8_t TextSize = 0;
        uint16_t TextColor = 0;

    public:
        std::list<FTAction *> actions;
        FTButton(const char * label, uint8_t index,
                 ButtonTypes buttonType, const char *logoParm, const char *latchLogoParm,
                 uint16_t outline, uint8_t textSize, uint16_t textColor);
        ~FTButton();
        uint16_t Width();
        uint16_t Height();
        void Draw(int16_t x, int16_t y, uint16_t margin, bool force = false);
        void Invalidate();
        void Press();
        void Release();
    };

}