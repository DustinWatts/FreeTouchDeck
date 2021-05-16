#include "FTButton.h"

static const char * module="FTButton";
namespace FreeTouchDeck 
{
    const char *enum_to_string(ButtonTypes type)
    {
        switch (type)
        {
        case ButtonTypes::STANDARD:
            return "STANDARD";
        case ButtonTypes::LATCH:
            return "LATCH";
        case ButtonTypes::MENU:
            return "MENU";
        default:
            return "Unknown button type";
        }
    }
    FTButton::FTButton(const char * label, uint8_t index, 
                ButtonTypes buttonType, const char * logoParm, const char * latchLogoParm, 
                uint16_t outline, uint8_t textSize, uint16_t textColor)
    {

        Index = index;
        ButtonType = buttonType;
        TextColor = textColor;
        Label = ps_strdup(label?label:"");

        ESP_LOGD(module,"Instantiating button #%d, type %s ", index, enum_to_string(buttonType));
        if(logoParm && strlen(logoParm)>0)
        {
            ESP_LOGD(module, "Attempting to get logo %s", logoParm);
            Logo=GetImage(logoParm);
        }
        
        if(!Logo || !Logo->valid)
        {
            ESP_LOGE(module, "Invalid logo %s",logoParm);
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
        ESP_LOGD(module, "Drawing button at [%d,%d] with margin %d\n", x, y, margin );
        tft.setFreeFont(LABEL_FONT);
        BMPImage * image = ((Latched && LatchedLogo->valid)? LatchedLogo : Logo);
        if(!image)
        {
            ESP_LOGE(module, "No image found");
            return;
        }
        bool transparent=false;
        uint16_t BGColor = tft.color565(image->R, image->G, image->B);
        initButton(&tft, x, y, image->w+margin, image->h+margin, Outline, BGColor, TextColor, Label, TextSize);
        
        if(Latched && !LatchedLogo->valid)
        {
            tft.fillRoundRect(x,y,18,18,4,generalconfig.latchedColour);
            transparent=true;
        }
        drawButton();
        
        image->Draw(x, y, transparent);
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
            ESP_LOGV(module,"Button already pressed. Ignoring");
            return;
        }
        // Beep
        HandleAudio(Sounds::BEEP);
        ESP_LOGD(module,"%s Button Press detected with %d actions",  enum_to_string(ButtonType), actions.size());
        for(FTAction * action : actions)
        {
            if(!QueueAction(action))
            {
                ESP_LOGW(module,"Button action type %s could not be queued for execution.",enum_to_string(action->Type));
            }
        }
        if(ButtonType == ButtonTypes::LATCH)
        {
            Latched=!Latched;
            ESP_LOGD(module,"Toggling LATCH to %s",Latched?"ACTIVE":"INACTIVE");
        }                    
        IsPressed=true;
        NeedsDraw=true;
    }
    void FTButton::Release()
    {
        if(IsPressed) 
        {
            ESP_LOGD(module,"Releasing button");
            IsPressed = false;
            NeedsDraw = true;
        }
    }

};
