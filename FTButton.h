#pragma once

#include "globals.hpp"
#include "FTAction.h"
#include "UserConfig.h"
#include "ImageWrapper.h"
#include "ActionsSequence.h"
namespace FreeTouchDeck
{
    enum class ButtonTypes
    {
        NONE,
        STANDARD,
        LATCH,
        ENDLIST
    };
    const char *enum_to_string(ButtonTypes type);

    class FTButton 
    {
    protected:
        bool Latched = false;
        bool NeedsDraw = true;
        bool NeedsDrawImage = true;
        bool IsPressed = false;

    private:
        uint16_t CenterX = 0;
        uint16_t CenterY = 0;
        uint16_t X = 0;
        uint16_t Y = 0;
        uint16_t ButtonWidth = 0;
           
        uint8_t Spacing = 0;
        uint16_t ButtonHeight = 0;
        uint16_t AdjustedWidth;
        uint16_t AdjustedHeight;
        uint16_t TextAdjustedWidth;
        char *_jsonLogo = NULL;
        char *_jsonLatchedLogo = NULL;
        void ExecuteActions();

    public:
        bool contains(uint16_t x, uint16_t y);
        ButtonTypes ButtonType;
        uint32_t BackgroundColor = 0;
        uint32_t MenuBackgroundColor = 0;
        uint32_t Outline = 0;
        uint8_t TextSize = 0;
        uint32_t TextColor = 0;
        char *Label = NULL;
        static void InitConstants();

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


        std::list<ActionsSequences> Sequences;
        FTButton(cJSON *button);
        FTButton();
        FTButton(cJSON *button,uint32_t BackgroundColor,uint32_t Outline ,uint32_t TextColor);
        FTButton(ButtonTypes buttonType, const char * label, const char * logo,const char * latchedLogo, uint32_t outline, uint8_t textSize, uint32_t textColor);
        void Init(cJSON *button);
        FTButton(const char * tmpl, bool isShared=false);
        static FTButton * BackButton; 
        static FTButton * HomeButton;
        void SetCoordinates(uint16_t width, uint16_t height, uint16_t row, uint16_t col, uint8_t spacing);
        ~FTButton();
        bool Latch(FTAction *action);
        ImageWrapper *LatchedLogo();
        ImageWrapper *GetActiveImage();
        bool IsLabelDraw();
        ImageWrapper *Logo();
        uint16_t Width();
        uint16_t Height();
        void DrawShape(bool force);
        void DrawImage(bool force);
        void Draw(bool force);
        void Invalidate();
        void Press();
        void UnPress();
        void Release();
        cJSON *ToJSON();
        
    };
    static ButtonTypes &operator++(ButtonTypes &state, int);
    
    ButtonTypes parse_button_types(const char *buttonType);

}