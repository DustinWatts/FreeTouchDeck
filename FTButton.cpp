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
    ImageWrapper *FTButton::LatchedLogo()
    {
        ImageWrapper *image = NULL;
        if (_jsonLatchedLogo && strlen(_jsonLatchedLogo) > 0)
        {
            ESP_LOGV(module, "Latched Logo file name is %s", _jsonLatchedLogo);
            image = GetImage(_jsonLatchedLogo);
            if (image && !image->valid)
            {
                ESP_LOGD(module, "Latched Logo file %s is invalid.", _jsonLatchedLogo);
                image = NULL;
            }
        }
        return image;
    }
    bool FTButton::IsLabelDraw()
    {
        ImageWrapper *image = NULL;
        if (_jsonLogo && strlen(_jsonLogo) > 0)
        {
            image = GetImage(_jsonLogo);
        }
        else
        {
            ESP_LOGD(module, "Empty logo value");
        }
        if (!image || !image->valid)
        {
            return true;
        }
        return false;
    }
    ImageWrapper *FTButton::Logo()
    {
        ImageWrapper *image = NULL;

        if (_jsonLogo && strlen(_jsonLogo) > 0)
        {
            ESP_LOGV(module, "Logo file name is %s", _jsonLogo);
            image = GetImage(_jsonLogo);
        }
        else
        {
            ESP_LOGD(module, "empty logo file name");
        }

        if (!image || !image->valid)
        {
            ESP_LOGD(module, "Logo file name %s was not found. Defaulting to label.bmp", _jsonLogo ? _jsonLogo : "");
            image = GetImage("label.bmp");
        }
        return image;
    }
    bool FTButton::Latch(FTAction *action)
    {
        if (ButtonType == ButtonTypes::LATCH && action->IsLatch())
        {
            switch (action->Type)
            {
            case ActionTypes::SETLATCH:
                if (!Latched)
                {
                    Latched = true;
                    Invalidate();
                }
                break;
            case ActionTypes::CLEARLATCH:
                if (Latched)
                {
                    Latched = false;
                    Invalidate();
                }
                break;
            case ActionTypes::TOGGLELATCH:
                Latched = !Latched;
                Invalidate();
                break;
            default:
                break;
            }
        }
        else
        {
            ESP_LOGW(module, "Cannot latch button. Button %s has type %s.", Label, enum_to_string(ButtonType));
            return false;
        }
        return true;
    }
    FTButton::FTButton(uint8_t index, cJSON *button, uint16_t outline, uint8_t textSize, uint16_t textColor) : Outline(outline), TextSize(textSize), TextColor(textColor)
    {
        char menuName[31] = {0};

        if (button && cJSON_IsString(button))
        {
            _jsonLogo = strdup(cJSON_GetStringValue(button));
        }
        else
        {
            ESP_LOGW(module,"Menu button does not have a logo!");
        }
        cJSON *jsonActionValue = NULL;
        ButtonType = ButtonTypes::MENU;
        BackgroundColor=generalconfig.menuButtonColour;
        snprintf(menuName, sizeof(menuName), "menu%d", index + 1);
        actions.push_back(new FTAction(ActionTypes::MENU, menuName));
    }
    FTButton::FTButton(uint8_t index, cJSON *document, cJSON *button, uint16_t outline, uint8_t textSize, uint16_t textColor) : Outline(outline), TextSize(textSize), TextColor(textColor)
    {
        char logoName[31] = {0};
        cJSON *jsonActionValue = NULL;
        cJSON *_jsonLatch = NULL;
        snprintf(logoName, sizeof(logoName), "logo%d", index);
        cJSON *jsonLogo = cJSON_GetObjectItem(document, logoName);
        if (jsonLogo && cJSON_IsString(jsonLogo))
        {
            _jsonLogo = strdup(cJSON_GetStringValue(jsonLogo));
        }
        else 
        {
            ESP_LOGW(module, "No logo file was found for %s", jsonLogo);
        }

        strncpy(Label, logoName, sizeof(Label));
        cJSON *jsonLatchedLogo = cJSON_GetObjectItem(button, "latchlogo");
        if (jsonLatchedLogo && cJSON_IsString(jsonLatchedLogo))
        {
            _jsonLatchedLogo = strdup(cJSON_GetStringValue(jsonLatchedLogo));
        }
        _jsonLatch = cJSON_GetObjectItem(button, "latch");
        ButtonType = (_jsonLatch && cJSON_IsBool(_jsonLatch) && cJSON_IsTrue(_jsonLatch)) ? ButtonTypes::LATCH : ButtonTypes::STANDARD;
        BackgroundColor=(ButtonType==ButtonTypes::MENU?generalconfig.menuButtonColour: generalconfig.backgroundColour);
        cJSON *jsonActions = cJSON_GetObjectItem(button, "actionarray");
        if (!jsonActions)
        {
            ESP_LOGE(module, "Button %s does not have any action!", Logo()->LogoName);
        }
        else
        {
            uint8_t valuePos = 0;
            ESP_LOGD(module, "Button %s has %d actions", Logo()->LogoName, cJSON_GetArraySize(jsonActions));
            cJSON_ArrayForEach(jsonActionValue, cJSON_GetObjectItem(button, "valuearray"))
            {
                cJSON *jsonAction = cJSON_GetArrayItem(jsonActions, valuePos++);
                if (!jsonAction)
                {
                    ESP_LOGE(module, "Current value does not have a matching action!");
                }
                else if (FTAction::GetType(jsonAction) != ActionTypes::NONE)
                {
                    ESP_LOGD(module, "Adding action to button %s, type %s", Logo()->LogoName, enum_to_string(FTAction::GetType(jsonAction)));
                    auto action = new FTAction(jsonAction, jsonActionValue);
                    if (!action)
                    {
                        ESP_LOGE(module, "Could not allocate memory for action");
                    }
                    else 
                    {
                        if(action->Type == ActionTypes::FUNCTIONKEYS)
                        {
                            BackgroundColor=generalconfig.functionButtonColour;
                        }
                        actions.push_back(action);
                        ESP_LOGD(module, "DONE Adding action to button %s, type %s", Logo()->LogoName, enum_to_string(FTAction::GetType(jsonAction)));                        
                    }

                    // only push valid actions
                    PrintMemInfo();
                }
                else
                {
                    ESP_LOGD(module, "Ignoring action type NONE for button %s", Logo()->LogoName);
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
    ImageWrapper *FTButton::GetActiveImage()
    {
        ImageWrapper *image = NULL;
        if (Latched)
        {
            image = LatchedLogo();
        }
        if (!image)
        {
            image = Logo();
        }
        return image;
    }
    void FTButton::Draw(int16_t centerX, int16_t centerY, uint16_t width, uint16_t height, uint16_t margin, bool force)
    {
        bool transparent = false;
        int32_t radius = 4;
        uint16_t adjustedWidth = width - (2 * margin);
        uint16_t adjustedHeight = height - (2 * margin);

        if (!NeedsDraw && !force)
            return;

        NeedsDraw = false;
        bool LatchNeedsRoundRect = !LatchedLogo();
        ImageWrapper *image = GetActiveImage();

        if (!image || !image->valid)
        {
            ESP_LOGE(module, "No image found, or image invalid!");
            return;
        }
        ESP_LOGV(module, "Found image structure, bitmap is %s", image->LogoName);

        uint16_t BGColor = tft.color565(image->R, image->G, image->B);
        ESP_LOGD(module, "Drawing button at [%d,%d] size: %dx%d,  with margin %d, outline : 0x%04X, BG Color: 0x%04X, Text color: 0x%04X, Text size: %d", centerX, centerY, image->w + margin, image->h + margin, margin, Outline, BGColor, TextColor, TextSize);
        PrintMemInfo();
        tft.setFreeFont(LABEL_FONT);
        initButton(&tft, centerX, centerY, adjustedWidth, adjustedHeight, Outline, BackgroundColor, TextColor, (char *)(IsLabelDraw() ? Label : ""), TextSize);
        drawButton();
        if (ButtonType == ButtonTypes::LATCH && LatchNeedsRoundRect)
        {
            uint32_t roundRectWidth = width / 4;
            uint32_t roundRectHeight = height / 4;
            uint32_t cornerX = centerX - (adjustedWidth/2)+roundRectWidth/2;
            uint32_t cornerY = centerY - (adjustedHeight/2)+roundRectHeight/2;
            if (Latched)
            {
                ESP_LOGD(module, "Latched without a latched logo.  Drawing round rectangle");
                tft.fillRoundRect(cornerX, cornerY, roundRectWidth, roundRectHeight, radius, generalconfig.latchedColour);
                transparent = true;
            }
            else
            {
                ESP_LOGD(module, "Latched deactivated without a latched logo.  Erasing round rectangle");
                tft.fillRoundRect(cornerX, cornerY, roundRectWidth, roundRectHeight, radius, BackgroundColor);
                // draw button one more time
                drawButton();
            }
        }

        image->Draw(centerX, centerY, transparent);
    }
    uint16_t FTButton::Width()
    {
        return LatchedLogo() ? max(Logo()->w, LatchedLogo()->w) : Logo()->w;
    }
    uint16_t FTButton::Height()
    {
        return LatchedLogo() ? max(Logo()->h, LatchedLogo()->h) : Logo()->h;
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
            ESP_LOGD(module, "Queuing action %s", enum_to_string(action->Type), action->toString());
            if (!QueueAction(action))
            {
                ESP_LOGW(module, "Button action %s could not be queued for execution.", action->toString());
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
            ESP_LOGD(module, "Releasing button with logo %s", Logo()->LogoName);
            IsPressed = false;
            NeedsDraw = true;
        }
    }

};
