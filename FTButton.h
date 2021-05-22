#pragma once

#include "globals.hpp"
#include "FTAction.h"
#include "UserConfig.h"
#include "ImageWrapper.h""
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
        bool Latched = false;
        ButtonTypes ButtonType;
        bool NeedsDraw = true;
        bool IsPressed = false;

    private:
        uint16_t Outline = 0;
        uint8_t TextSize = 0;
        uint16_t TextColor = 0;
        char * _jsonLogo=NULL;
        char * _jsonLatchedLogo=NULL;

    public:
        std::list<FTAction *> actions;
        FTButton(uint8_t index, cJSON * document, cJSON * button, uint16_t outline, uint8_t textSize, uint16_t textColor);
        FTButton(uint8_t index,cJSON * button, uint16_t outline, uint8_t textSize, uint16_t textColor);
        ~FTButton();
        bool Latch(FTAction *action);
        ImageWrapper *LatchedLogo();
        ImageWrapper *GetActiveImage();
        char Label[21] = {0};        
        bool IsLabelDraw();
        ImageWrapper *Logo();
        uint16_t Width();
        uint16_t Height();
        void Draw(int16_t x, int16_t y,uint16_t width,uint16_t height, uint16_t margin, bool force);
        void Invalidate();
        void Press();
        void Release();

    };

}