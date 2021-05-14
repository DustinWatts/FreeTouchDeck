#include "FTButton.h"


namespace FreeTouchDeck 
{

    FTButton::FTButton(TFT_eSPI &gfx,String label, uint8_t index, 
                ButtonTypes buttonType, const char * logoParm, const char * latchLogoParm, 
                uint16_t outline, uint8_t textSize, uint16_t textColor) : GFX(gfx)
    {
        Index = index;

        ButtonType = buttonType;
        TextColor = textColor;
        Label = strdup(label.isEmpty()?"":label.c_str());


        if(logoParm && strlen(logoParm)>0)
        {
            Logo=GetImage(logoParm);
        }
        
        if(!Logo || !Logo->valid)
        {
            Serial.printf("[ERROR]: Invalid logo %s\n",logoParm);
        }
        if(latchLogoParm && strlen(latchLogoParm)>0)
        {
            LatchedLogo=GetImage(latchLogoParm);
        }
        Outline = outline;
        TextSize = textSize;
    }

    FTButton::~FTButton()
    {
        if (Label != NULL)
            free(Label);
    }
    void FTButton::Invalidate()
    {
        NeedsDraw=true;
    }
    void FTButton::Draw(int16_t x, int16_t y,uint16_t margin, bool force)
    {
        if(!NeedsDraw && !force)  return;

        NeedsDraw=false;
        //Serial.printf("Drawing button at [%d,%d] with margin %d\n", x, y, margin );
        GFX.setFreeFont(LABEL_FONT);
        BMPImage * image = ((Latched && LatchedLogo->valid)? LatchedLogo : Logo);
        if(!image)
        {
            Serial.println("[ERROR]: No image found");
            return;
        }
        bool transparent=false;
        uint16_t BGColor = GFX.color565(image->R, image->G, image->B);
        initButton(&GFX, x, y, image->w+margin, image->h+margin, Outline, BGColor, TextColor, Label, TextSize);
        
        if(Latched && !LatchedLogo->valid)
        {
            GFX.fillRoundRect(x,y,18,18,4,generalconfig.latchedColour);
            transparent=true;
        }
        drawButton();
        
        image->Draw(&GFX,x, y, transparent);
    }
    uint16_t FTButton::Width()
    {
        return max(Logo->w,LatchedLogo->w);
    }
    uint16_t FTButton::Height()
    {
        return max(Logo->h,LatchedLogo->h);
    } 

    void FTButton::Press()
    {
        if(IsPressed) 
        {
            return;
        }
        // Beep
        HandleAudio(Sounds::BEEP);
        //Serial.printf("%sButton Press detected with %d actions\n", ButtonType == LATCH?"LATCH ":"", actions.size());
        for(FTAction * action : actions)
        {
            if(!QueueAction(action))
            {
                Serial.println("Button action could not be executed");
            }
        }
        if(ButtonType == LATCH)
        {
            Latched=!Latched;
            //Serial.printf("Toggling LATCH to %s\n",Latched?"ACTIVE":"INACTIVE");
        }                    
        IsPressed=true;
        NeedsDraw=true;
    }
    void FTButton::Release()
    {
        if(IsPressed) 
        {
            Serial.println("Releasing button");

            IsPressed = false;
            NeedsDraw = true;
        }
    }

};
