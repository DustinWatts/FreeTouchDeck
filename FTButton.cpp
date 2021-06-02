#include "FTButton.h"

static const char *module = "FTButton";
namespace FreeTouchDeck
{

    const char *enum_to_string(ButtonTypes type)
    {
        switch (type)
        {
            ENUM_TO_STRING_HELPER(ButtonTypes, STANDARD);
            ENUM_TO_STRING_HELPER(ButtonTypes, LATCH);
            ENUM_TO_STRING_HELPER(ButtonTypes, MENU);
        default:
            return "Unknown button type";
        }
    }
    const char *FTButton::JsonLabelLatchedLogo = "latchedlogo";
    const char *FTButton::JsonLabelLogo = "logo";
    const char *FTButton::JsonLabelType = "type";
    const char *FTButton::JsonLabelLabel = "label";
    const char *FTButton::JsonLabelActions = "actions";
    const char *FTButton::JsonLabelOutline = "outline";
    const char *FTButton::JsonLabelBackground = "background";
    const char *FTButton::JsonLabelTextColor = "textcolor";
    const char *FTButton::JsonLabelTextSize = "textsize";

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
        if (!ISNULLSTRING(_jsonLogo))
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
        ESP_LOGD(module,"Button is Executing Action %s", action->toString());
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
            ESP_LOGW(module, "Cannot latch button. Button %s has type %s.", STRING_OR_DEFAULT(Label, _jsonLogo), enum_to_string(ButtonType));
            return false;
        }
        return true;
    }

    void FTButton::SetCoordinates(uint16_t width, uint16_t height, uint16_t row, uint16_t col, uint8_t spacing)
    {
        ESP_LOGD(module, "Button size %dx%d, row %d, col %d  ", width, height, row, col);
        ButtonWidth = width;
        ButtonHeight = height;
        Spacing = spacing;
        CenterX = (col * 2 + 1) * ButtonWidth / 2 + (col + 1) * spacing;
        CenterY = (row * 2 + 1) * ButtonHeight / 2 + (row + 1) * spacing;
    }
    FTButton::FTButton(const char *label, uint8_t index, cJSON *button, uint16_t outline, uint8_t textSize, uint16_t textColor) : Outline(outline), TextSize(textSize), TextColor(textColor)
    {
        char menuName[31] = {0};
        ESP_LOGD(module, "Button Constructor for menu button");
        if (button && cJSON_IsString(button))
        {
            _jsonLogo = strdup(cJSON_GetStringValue(button));
        }
        else
        {
            ESP_LOGW(module, "Menu button does not have a logo!");
        }
        cJSON *jsonActionValue = NULL;
        ButtonType = ButtonTypes::MENU;
        BackgroundColor = generalconfig.menuButtonColour;
        snprintf(menuName, sizeof(menuName), "menu%d", index + 1);
        Label = ps_strdup(STRING_OR_DEFAULT(label, ""));
        actions.push_back(new FTAction(ActionTypes::MENU, menuName));
    }
    FTButton::FTButton(const char *label, uint8_t index, cJSON *document, cJSON *button, uint16_t outline, uint8_t textSize, uint16_t textColor) : Outline(outline), TextSize(textSize), TextColor(textColor)
    {
        char logoName[31] = {0};
        cJSON *jsonActionValue = NULL;
        cJSON *_jsonLatch = NULL;
        ESP_LOGD(module, "Button Constructor for action button");
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
        Label = ps_strdup(STRING_OR_DEFAULT(label, ""));
        cJSON *jsonLatchedLogo = cJSON_GetObjectItem(button, "latchlogo");
        if (jsonLatchedLogo && cJSON_IsString(jsonLatchedLogo))
        {
            _jsonLatchedLogo = strdup(cJSON_GetStringValue(jsonLatchedLogo));
        }
        _jsonLatch = cJSON_GetObjectItem(button, "latch");
        ButtonType = (_jsonLatch && cJSON_IsBool(_jsonLatch) && cJSON_IsTrue(_jsonLatch)) ? ButtonTypes::LATCH : ButtonTypes::STANDARD;
        BackgroundColor = (ButtonType == ButtonTypes::MENU ? generalconfig.menuButtonColour : generalconfig.backgroundColour);
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
                        if (action->Type == ActionTypes::FUNCTIONKEYS)
                        {
                            BackgroundColor = generalconfig.functionButtonColour;
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
        FREE_AND_NULL(Label);
        FREE_AND_NULL(_jsonLogo);
        FREE_AND_NULL(_jsonLatchedLogo);
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
    void FTButton::Draw(bool force)
    {
        bool transparent = false;
        int32_t radius = 4;
        uint16_t BGColor = TFT_BLACK;
        uint16_t adjustedWidth = ButtonWidth - (2 * Spacing);
        uint16_t adjustedHeight = ButtonHeight - (2 * Spacing);

        if (!NeedsDraw && !force)
            return;

        NeedsDraw = false;
        _button.initButton(&tft, CenterX, CenterY, adjustedWidth, adjustedHeight, Outline, BackgroundColor, TextColor, (char *)(IsLabelDraw() ? Label : ""), TextSize);
        _button.drawButton();
        bool LatchNeedsRoundRect = !LatchedLogo();
        ImageWrapper *image = GetActiveImage();

        if (!image || !image->valid)
        {
            ESP_LOGE(module, "No image found, or image invalid!");
        }
        else
        {
            BGColor= tft.color565(image->R, image->G, image->B);
        }
        ESP_LOGV(module, "Found image structure, bitmap is %s", image->LogoName);
        ESP_LOGD(module, "Drawing button at [%d,%d] size: %dx%d,  outline : 0x%04X, BG Color: 0x%04X, Text color: 0x%04X, Text size: %d", CenterX, CenterY, adjustedWidth, adjustedHeight,  Outline, BGColor, TextColor, TextSize);
        PrintMemInfo();
        tft.setFreeFont(LABEL_FONT);

        if (ButtonType == ButtonTypes::LATCH && LatchNeedsRoundRect)
        {
            uint32_t roundRectWidth = ButtonWidth / 4;
            uint32_t roundRectHeight = ButtonHeight / 4;
            uint32_t cornerX = CenterX - (adjustedWidth / 2) + roundRectWidth / 2;
            uint32_t cornerY = CenterY - (adjustedHeight / 2) + roundRectHeight / 2;
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
                _button.drawButton();
            }
        }
        if (image && image->valid)    
        {
            image->Draw(CenterX, CenterY, transparent);
        }
        
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
        bool needsRelease=false;
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
            needsRelease=action->NeedsRelease?true:needsRelease;
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
        if(needsRelease)
        {
            QueueAction(&FTAction::releaseAllAction);
        }
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
    cJSON *FTButton::ToJSON()
    {
        cJSON *button = cJSON_CreateObject();
        if (!button)
        {
            drawErrorMessage(true, module, "Memory allocation failed when rendering JSON button");
            return NULL;
        }
        ESP_LOGD(module, "Adding button members to Json");
        if (!ISNULLSTRING(Label))
        {
            cJSON_AddStringToObject(button, FTButton::JsonLabelLabel, STRING_OR_DEFAULT(Label, ""));
        }
        if (!ISNULLSTRING(_jsonLogo))
        {
            cJSON_AddStringToObject(button, FTButton::JsonLabelLogo, _jsonLogo ? _jsonLogo : "");
        }
        if (!ISNULLSTRING(_jsonLatchedLogo))
        {
            cJSON_AddStringToObject(button, FTButton::JsonLabelLatchedLogo, _jsonLatchedLogo ? _jsonLatchedLogo : "");
        }
        cJSON_AddStringToObject(button, FTButton::JsonLabelType, enum_to_string(ButtonType));

        if (Outline != generalconfig.DefaultOutline)
        {
            cJSON_AddNumberToObject(button, FTButton::JsonLabelOutline, Outline);
        }
        if (BackgroundColor != generalconfig.backgroundColour)
        {
            cJSON_AddNumberToObject(button, FTButton::JsonLabelBackground, BackgroundColor);
        }
        if (TextColor != generalconfig.DefaultTextColor)
        {
            cJSON_AddNumberToObject(button, FTButton::JsonLabelTextColor, TextColor);
        }
        if (TextSize != generalconfig.DefaultTextSize)
        {
            cJSON_AddNumberToObject(button, FTButton::JsonLabelTextSize, TextSize);
        }
        ESP_LOGD(module, "Adding actions to Json");
        if (actions.size() > 0)
        {
            cJSON *actionsJson = cJSON_CreateArray();
            for (auto action : actions)
            {
                if (action->Type != ActionTypes::NONE)
                {
                    cJSON_AddItemToArray(actionsJson, action->ToJson());
                }
            }
            cJSON_AddItemToObject(button, FTButton::JsonLabelActions, actionsJson);
        }
#if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_VERBOSE
        char *buttonString = cJSON_Print(button);
        if (buttonString)
        {
            ESP_LOGD(module, "Button json structure : \n%s", buttonString);
            FREE_AND_NULL(buttonString);
        }
        else
        {
            ESP_LOGE(module, "Unable to format JSON for output!");
        }
#endif
        return button;
    }
    bool FTButton::contains(uint16_t x, uint16_t y)
    {
        return _button.contains(x,y);
    }
    FTButton::FTButton(cJSON *button)
    {
        char *buttonType = NULL;
        GetValueOrDefault(button, FTButton::JsonLabelLabel, &Label, "");
        ESP_LOGD(module, "Label: %s", Label);
        GetValueOrDefault(button, FTButton::JsonLabelLogo, &_jsonLogo, "");
        ESP_LOGD(module, "Logo : %s", _jsonLogo);
        GetValueOrDefault(button, FTButton::JsonLabelLatchedLogo, &_jsonLatchedLogo, "");
        ESP_LOGD(module, "Latched logo: %s", _jsonLatchedLogo);
        GetValueOrDefault(button, FTButton::JsonLabelType, &buttonType, enum_to_string(ButtonTypes::STANDARD));
        ButtonType = parse_button_types(buttonType);
        FREE_AND_NULL(buttonType);
        ESP_LOGD(module, "Button type is %s", enum_to_string(ButtonType));

        GetValueOrDefault(button, FTButton::JsonLabelOutline, &Outline, generalconfig.DefaultOutline);
        GetValueOrDefault(button, FTButton::JsonLabelBackground, &BackgroundColor, generalconfig.backgroundColour);
        GetValueOrDefault(button, FTButton::JsonLabelTextColor, &TextColor, generalconfig.DefaultTextColor);
        GetValueOrDefault(button, FTButton::JsonLabelTextSize, &TextSize, generalconfig.DefaultTextSize);

        cJSON *jsonActions = cJSON_GetObjectItem(button, FTButton::JsonLabelActions);
        if (!jsonActions)
        {
            ESP_LOGE(module, "Button %s does not have any action!", Logo()->LogoName);
        }
        else
        {
            if(!cJSON_IsArray(jsonActions))
            {
                ESP_LOGE(module, "Actions object for button %s should be an array but it is not.", Label);
            }
            cJSON *actionJson = NULL;
            cJSON_ArrayForEach(actionJson, jsonActions)
            {
                auto action = new FTAction(actionJson);
                if (action->Type != ActionTypes::NONE)
                {
                    actions.push_back(action);
                }
            }
        }
    }
    ButtonTypes &operator++(ButtonTypes &state, int)
    {
        int i = static_cast<int>(state) + 1;
        i = i >= (int)ButtonTypes::ENDLIST ? (int)ButtonTypes::NONE : i;
        state = static_cast<ButtonTypes>(i);
        return state;
    }
    ButtonTypes parse_button_types(const char *buttonType)
    {
        ButtonTypes Result = ButtonTypes::NONE;
        do
        {
            Result++; // Start after NONE
        } while (strcmp(buttonType, enum_to_string(Result)) != 0 && Result != ButtonTypes::NONE);
        return Result;
    }
};
