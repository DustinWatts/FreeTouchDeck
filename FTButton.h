#pragma once

#include "globals.hpp"
#include "FTAction.h"
#include "UserConfig.h"
#include "ImageWrapper.h"

namespace FreeTouchDeck
{
    enum class ButtonTypes
    {
        NONE,
        STANDARD,
        LATCH,
        MENU,
        ENDLIST
    };
    const char *enum_to_string(ButtonTypes type);

    class FTButton 
    {
    protected:
        bool Latched = false;
        bool NeedsDraw = true;
        bool IsPressed = false;

    private:
        uint16_t Outline = 0;
        uint8_t TextSize = 0;
        uint16_t TextColor = 0;
        uint16_t CenterX = 0;
        uint16_t CenterY = 0;
        uint16_t ButtonWidth = 0;
        uint8_t Spacing = 0;
        uint16_t ButtonHeight = 0;
        uint16_t BackgroundColor = 0;
        char *_jsonLogo = NULL;
        char *_jsonLatchedLogo = NULL;
        TFT_eSPI_Button _button;

    public:
        bool contains(uint16_t x, uint16_t y);
        ButtonTypes ButtonType;
        static const char *JsonLabelLogo;
        static const char *JsonLabelLatchedLogo;
        static const char *JsonLabelType;
        static const char *JsonLabelLabel;
        static const char *JsonLabelActions;
        static const char *JsonLabelOutline;
        static const char *JsonLabelBackground;
        static const char *JsonLabelTextColor;
        static const char *JsonLabelTextSize;
        static const char *backButtonTemplate;
        static const char *homeButtonTemplate;
        bool IsShared = false;

        char *Label = NULL;
        std::list<FTAction *> actions;
        FTButton(const char *label, uint8_t index, cJSON *document, cJSON *button, uint16_t outline, uint8_t textSize, uint16_t textColor);
        FTButton(const char *label, uint8_t index, cJSON *button, uint16_t outline, uint8_t textSize, uint16_t textColor);
        FTButton(cJSON *button);
        void Init(cJSON *button);
        FTButton(const char * tmpl, bool isShared=false);
        static FTButton BackButton; 
        static FTButton HomeButton;
        void SetCoordinates(uint16_t width, uint16_t height, uint16_t row, uint16_t col, uint8_t spacing);
        ~FTButton();
        bool Latch(FTAction *action);
        ImageWrapper *LatchedLogo();
        ImageWrapper *GetActiveImage();
        bool IsLabelDraw();
        ImageWrapper *Logo();
        uint16_t Width();
        uint16_t Height();
        void Draw(bool force);
        void Invalidate();
        void Press();
        void Release();
        cJSON *ToJSON();
        
    };
    static ButtonTypes &operator++(ButtonTypes &state, int);
    
    ButtonTypes parse_button_types(const char *buttonType);

}