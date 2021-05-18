#include "FTButton.h"

static const char *module = "FTButton";
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
    BMPImage *FTButton::LatchedLogo()
    {
        if (_jsonLogo && cJSON_IsString(_jsonLogo))
        {
            return GetImage(cJSON_GetStringValue(_jsonLogo));
        }
        return GetImage("question.bmp");
    }
    BMPImage *FTButton::Logo()
    {
        if (_jsonLogo && cJSON_IsString(_jsonLogo))
        {
            return GetImage(cJSON_GetStringValue(_jsonLogo));
        }
        return GetImage("question.bmp");
    }
    FTButton::FTButton(uint8_t index, cJSON *button, uint16_t outline, uint8_t textSize, uint16_t textColor)
    {
        char menuName[31] = {0};

        _jsonLogo = button;
        cJSON *jsonActionValue = NULL;
        ButtonType = ButtonTypes::MENU;
        snprintf(menuName, sizeof(menuName), "menu%d", index + 1);
        actions.push_back(new FTAction(ActionTypes::MENU, menuName));
    }
    FTButton::FTButton(uint8_t index, cJSON *document, cJSON *button, uint16_t outline, uint8_t textSize, uint16_t textColor)
    {
        char logoName[31] = {0};
        cJSON *jsonActionValue = NULL;
        cJSON *_jsonLatch = NULL;
        snprintf(logoName, sizeof(logoName), "logo%d", index);
        _jsonLogo = cJSON_GetObjectItem(document, logoName);
        strncpy(Label,logoName,sizeof(Label));
        _jsonLatchedLogo = cJSON_GetObjectItem(button, "latchlogo");
        _jsonLatch = cJSON_GetObjectItem(button, "latch");
        ButtonType = (_jsonLatch && cJSON_IsBool(_jsonLatch) && cJSON_IsTrue(_jsonLatch)) ? ButtonTypes::LATCH : ButtonTypes::STANDARD;
        cJSON *jsonActions = cJSON_GetObjectItem(button, "actionarray");
        if (!jsonActions)
        {
            ESP_LOGE(module,"Button %s does not have any action!",Logo()->LogoName);
        }
        else
        {
            uint8_t valuePos = 0;
            ESP_LOGD(module,"Button %s has %d actions",Logo()->LogoName,cJSON_GetArraySize(jsonActions));
            cJSON_ArrayForEach(jsonActionValue, cJSON_GetObjectItem(button, "valuearray"))
            {
                cJSON *jsonAction = cJSON_GetArrayItem(jsonActions, valuePos++);
                if (!jsonAction)
                {
                    ESP_LOGE(module, "Current value does not have a matching action!");
                }
                else if (FTAction::GetType(jsonAction) != ActionTypes::NONE)
                {
                    ESP_LOGD(module,"Adding action to button %s, type %s",Logo()->LogoName,enum_to_string(FTAction::GetType(jsonAction)));
                    // only push valid actions
                    auto action = new FTAction(jsonAction, jsonActionValue);
                    if (!action)
                    {
                        ESP_LOGE(module, "Could not allocate memory for action");
                    }
                    actions.push_back(action);
                }
                else
                {
                    ESP_LOGD(module,"Ignoring action type NONE for button %s",Logo()->LogoName);
                }
            }
        }
    }

    FTButton::~FTButton()
    {
        if (Label != NULL)
            free(Label);
    }
    void FTButton::Invalidate()
    {
        NeedsDraw = true;
    }
    void FTButton::Draw(int16_t x, int16_t y, uint16_t margin, bool force)
    {
        if (!NeedsDraw && !force)
            return;

        NeedsDraw = false;
        ESP_LOGD(module, "Drawing button at [%d,%d] with margin %d\n", x, y, margin);
        tft.setFreeFont(LABEL_FONT);
        BMPImage *image = ((Latched && LatchedLogo()->valid) ? LatchedLogo() : Logo());
        if (!image)
        {
            ESP_LOGE(module, "No image found");
            return;
        }
        ESP_LOGD(module,"Found image structure, bitmap is %s",image->LogoName);
        bool transparent = false;
        
        uint16_t BGColor = tft.color565(image->R, image->G, image->B);
        ESP_LOGD(module,"Background color is 0X%x",BGColor);
        initButton(&tft, x, y, image->w + margin, image->h + margin, Outline, BGColor, TextColor, Label, TextSize);

        if (Latched && !LatchedLogo()->valid)
        {
            ESP_LOGD(module,"Latched without a latched logo.  Drawing round rectangle");
            tft.fillRoundRect(x, y, 18, 18, 4, generalconfig.latchedColour);
            transparent = true;
        }
        ESP_LOGD(module,"Drawing button");
        drawButton();
        image->Draw(x, y, transparent);
    }
    uint16_t FTButton::Width()
    {
        return max(Logo()->w, LatchedLogo()->w);
    }
    uint16_t FTButton::Height()
    {
        return max(Logo()->h, LatchedLogo()->h);
    }

    void FTButton::Press()
    {
        if (IsPressed)
        {
            ESP_LOGV(module, "Button already pressed. Ignoring");
            return;
        }
        // Beep
        HandleAudio(Sounds::BEEP);
        ESP_LOGD(module, "%s Button Press detected with %d actions", enum_to_string(ButtonType), actions.size());
        for (FTAction *action : actions)
        {
            ESP_LOGD(module,"Queuing action type %s, value %s, num value %d",enum_to_string(action->Type), action->symbol?action->symbol:"",action->value);
            if (!QueueAction(action))
            {
                ESP_LOGW(module, "Button action type %s could not be queued for execution.", enum_to_string(action->Type));
            }
        }
        if (ButtonType == ButtonTypes::LATCH)
        {
            Latched = !Latched;
            ESP_LOGD(module, "Toggling LATCH to %s", Latched ? "ACTIVE" : "INACTIVE");
        }
        IsPressed = true;
        NeedsDraw = true;
    }
    void FTButton::Release()
    {
        if (IsPressed)
        {
            ESP_LOGD(module, "Releasing button with logo %s",Logo()->LogoName);
            IsPressed = false;
            NeedsDraw = true;
        }
    }

};
