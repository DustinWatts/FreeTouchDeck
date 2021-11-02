#include "FTButton.h"
#include "Audio.h"
#include "esp_attr.h"
#include "WString.h"
#include "ImageCache.h"
#include "System.h"
static const char *module = "FTButton";

namespace FreeTouchDeck
{
    //static NO_INT std::map<std::string,bool> LatchList;

    const char *enum_to_string(ButtonTypes type)
    {
        switch (type)
        {
            ENUM_TO_STRING_HELPER(ButtonTypes, STANDARD);
            ENUM_TO_STRING_HELPER(ButtonTypes, MENU);
            ENUM_TO_STRING_HELPER(ButtonTypes, LATCH);
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
    const char *FTButton::JsonLabelBackground = "backgroundcolor";
    const char *FTButton::JsonLabelTextColor = "textcolor";
    const char *FTButton::JsonLabelTextSize = "textsize";
    const char *FTButton::homeButtonTemplate = R"({ "label":"Home",  "logo":"home.jpg","actions": ["{MENU:home}"] })";
    const char *FTButton::backButtonTemplate = R"({"label": "Back","logo": "arrow_back.jpg","actions": ["{MENU:~BACK}"]})";
    FTButton FTButton::EmptyButton;
    FTButton *FTButton::BackButton = NULL;
    FTButton *FTButton::HomeButton = NULL;
    void FTButton::InitConstants()
    {
        LOC_LOGD(module, "Initializing buttons constants");
        BackButton = new FTButton(FTButton::backButtonTemplate, true);
        HomeButton = new FTButton(FTButton::homeButtonTemplate, true);
    }
    FTButton::FTButton()
    {
        ButtonType = ButtonTypes::NONE;
    }
    FTButton::FTButton(ButtonTypes buttonType, const char *label, const char *logo, const char *latchedLogo, uint32_t outline, uint8_t textSize, uint32_t textColor)
    {
        PrintMemInfo(__FUNCTION__, __LINE__);
        ButtonType = buttonType;
        Label = label;
        IsLabel=true;
        if (!ISNULLSTRING(logo))
        {
            _jsonLogo = logo;
            IsLabel = !ImageCache::GetImage(_jsonLogo)->valid;
            if (Label.empty())
            {
                Label = _jsonLogo;
            }
        }
        if (!ISNULLSTRING(latchedLogo))
        {
            _jsonLatchedLogo = latchedLogo;
        }
        Outline = outline;
        TextSize = textSize;
        TextColor = textColor;
        MenuBackgroundColor = generalconfig.backgroundColour;
        LOC_LOGD(module, "Button type is [%s], Label: [%s], Logo : [%s], Draw type is [%s]", enum_to_string(ButtonType), Label.c_str(), _jsonLogo.c_str(),IsLabel ? "LABEL" : "IMAGE");
        PrintMemInfo(__FUNCTION__, __LINE__);
    }
    bool FTButton::IsMenu()
    {
        if(ButtonType==ButtonTypes::MENU) return true;
        for(auto al : Sequences)
        {
            if(al.HasMenuAction())
            return true;
        }
        return false;
    }
    FTButton::FTButton(const char *jsonString, bool isShared)
    {
        IsShared = isShared;
        MenuBackgroundColor = generalconfig.backgroundColour;
        PrintMemInfo(__FUNCTION__, __LINE__);
        cJSON *doc = cJSON_Parse(jsonString);
        if (!doc)
        {
            const char *error = cJSON_GetErrorPtr();
            drawErrorMessage(true, module, "Unable to parse json string : %s", error);
        }
        else
        {
            LOC_LOGD(module, "Initializing button from json string");
            Init(doc);
        }
        cJSON_Delete(doc);
        PrintMemInfo(__FUNCTION__, __LINE__);
    }
    ImageWrapper *FTButton::LatchedLogo()
    {
        LOC_LOGV(module, "Latched Logo file name is %s", _jsonLatchedLogo.c_str());
        return ImageCache::GetImage(_jsonLatchedLogo);
    }
    ImageWrapper *FTButton::Logo()
    {
        ImageWrapper *image = ImageCache::GetImage(_jsonLogo);
        ;
        if (image->valid)
        {
            LOC_LOGV(module, "Logo file name is [%s]", _jsonLogo.c_str());
        }
        return image;
    }
    bool FTButton::Latch(FTAction *action)
    {
        LOC_LOGV(module, "Button is Executing Action %s", action->toString());
        if (ButtonType == ButtonTypes::LATCH)
        {
            String state = action->GetParameter(3).c_str();
            if (ISNULLSTRING(state.c_str()))
            {
                LOC_LOGE(module, "Invalid latch parameter. Should be ON,OFF or TOGGLE");
                return false;
            }
            if (state == "ON")
            {
                if (!Latched)
                {
                    Latched = true;
                    Invalidate();
                }
            }
            else if (state == "OFF")
            {
                if (!Latched)
                {
                    Latched = false;
                    Invalidate();
                }
            }
            else if (state == "TOGGLE")
            {
                Latched = !Latched;
                Invalidate();
            }
            else
            {
                LOC_LOGE(module, "Invalid latch parameter %s. Should be ON,OFF or TOGGLE", state.c_str());
            }
        }
        else
        {
            LOC_LOGW(module, "Cannot latch button. Button %s has type %s.", Label.c_str(), enum_to_string(ButtonType));
            return false;
        }
        return true;
    }

    void FTButton::SetCoordinates(uint16_t width, uint16_t height, uint16_t row, uint16_t col, uint8_t spacing)
    {
        uint16_t centerX = 0;
        uint16_t centerY = 0;
        Spacing = spacing;
        ButtonWidth = width;
        ButtonHeight = height;

        X = col * width + col * spacing;
        Y = row * height + row * spacing;

        centerX = X + (ButtonWidth / 2);
        centerY = Y + (ButtonHeight / 2);
        if (width != ButtonWidth || ButtonHeight != height || centerX != CenterX || centerY != CenterY || Spacing != spacing)
        {
            LOC_LOGD(module, "Button size %dx%d, row %d, col %d  ", width, height, row, col);
            CenterX = centerX;
            CenterY = centerY;
        }
        AdjustedWidth = ButtonWidth;   // - (2 * Spacing);
        AdjustedHeight = ButtonHeight; // - (2 * Spacing);
        TextAdjustedWidth = AdjustedWidth - (2 * Spacing);
    }

    FTButton::~FTButton()
    {
    }
    void FTButton::Invalidate()
    {
        NeedsDraw = true;
        NeedsDrawImage = !IsLabel;
    }
    ImageWrapper *FTButton::GetActiveImage()
    {
        if (Latched && LatchedLogo()->valid)
        {
            return LatchedLogo();
        }
        return Logo();
    }
    void FTButton::DrawShape(bool force)
    {
        bool transparent = false;
        int32_t radius = 4;
        uint8_t textSize = TextSize;
        uint16_t BGColor = TFT_BLACK;
        if (!NeedsDraw && !force)
            return;

        NeedsDraw = false;
        const char *buttonLabel = "";
        LOC_LOGV(module, "Getting active image for button");
        auto image = GetActiveImage();
        if (IsLabel)
        {
            SetDefaultFont();
            tft.setTextSize(TextSize);
            buttonLabel = Label.c_str();
            int16_t textWidth = tft.textWidth(buttonLabel, 0);
            while (textWidth > TextAdjustedWidth)
            {
                LOC_LOGD(module, "Text width is %d pixels and button text width is %d pixels. Trying a smaller font to fit the text", textWidth, TextAdjustedWidth);
                // Get the largest font that will fit in the butotn size
                if (textSize > 1)
                {
                    textSize--; //if text size multiplier is greater then 1, reduce it first
                    tft.setTextSize(textSize);
                }
                else
                {
                    // check if a smaller font is available
                    if (!SetSmallerFont())
                    {
                        break;
                    }
                    textWidth = tft.textWidth(buttonLabel, 0);
                }
            }
            LOC_LOGD(module, "Label draw of button [%s] [%d pixels]", buttonLabel, ButtonWidth);
            BGColor = convertRGB888ToRGB565(IsMenu()?generalconfig.functionButtonColour:BackgroundColor);
        }
        else
        {
            LOC_LOGD(module, "Image draw of button %s", image->LogoName.c_str());
            uint16_t ImagePixelColor = image->GetPixelColor();
            if (ImagePixelColor == TFT_BLACK)
            {
                LOC_LOGV(module, "Corner pixel is black, drawing transparent with default background color");
                transparent = true;
                BGColor = convertRGB888ToRGB565(IsMenu()?generalconfig.functionButtonColour:BackgroundColor);
            }
            else
            {
                // Draw the button with found image's background color
                BGColor = convertRGB888ToRGB565(ImagePixelColor);
            }
        }

        LOC_LOGD(module, "Drawing button at [%d,%d] size: %dx%d,  outline : 0x%04X, BG Color: 0x%04X, Text color: 0x%04X, Text size: %d, logo: %s", CenterX, CenterY, AdjustedWidth, AdjustedHeight, Outline, BGColor, TextColor, TextSize, _jsonLogo.c_str());
        PrintMemInfo(__FUNCTION__, __LINE__);

        uint8_t r = min(ButtonWidth, ButtonHeight) / 4; // Corner radius

        if (!bleKeyboard.isConnected() && HasKeyboardActions())
        {
            BGColor = TFT_DARKGREY;
        }
        if (BGColor != MenuBackgroundColor)
        {
            tft.fillRoundRect(X, Y, ButtonWidth, ButtonHeight, r, BGColor);
        }
        tft.drawRoundRect(X, Y, ButtonWidth, ButtonHeight, r, Outline);
        if (IsPressed)
        {
            tft.drawRoundRect(X + 2, Y + 2, ButtonWidth - 4, ButtonHeight - 4, r, Outline);
        }
        else
        {
            tft.drawRoundRect(X + 2, Y + 2, ButtonWidth - 4, ButtonHeight - 4, r, BGColor);
        }

        if (ButtonType == ButtonTypes::LATCH && !LatchedLogo()->valid)
        {
            // Draw a rounded rectangle in the button corner
            uint32_t roundRectWidth = ButtonWidth / 4;
            uint32_t roundRectHeight = ButtonHeight / 4;
            uint32_t cornerX = CenterX - (AdjustedWidth / 2) + roundRectWidth / 2;
            uint32_t cornerY = CenterY - (AdjustedHeight / 2) + roundRectHeight / 2;
            if (Latched)
            {
                LOC_LOGD(module, "LATCH Marker for %s",buttonLabel);
                tft.fillRoundRect(cornerX, cornerY, roundRectWidth, roundRectHeight, radius, generalconfig.latchedColour);
            }
            else
            {
                LOC_LOGD(module, "UNLATCH Marker for %s",buttonLabel);
                tft.fillRoundRect(cornerX, cornerY, roundRectWidth, roundRectHeight, radius, BGColor);
            }
        }

        if (IsLabel)
        {
            tft.setTextColor(convertRGB888ToRGB565(TextColor), BGColor);
            tft.setTextSize(TextSize);
            uint8_t tempdatum = tft.getTextDatum();
            tft.setTextDatum(MC_DATUM);
            uint16_t tempPadding = tft.getTextPadding();
            tft.setTextPadding(0);
            tft.drawString(buttonLabel, CenterX, CenterY);
            tft.setTextDatum(tempdatum);
            tft.setTextPadding(tempPadding);
        }
    }
    void FTButton::DrawImage(bool force)
    {
        bool transparent = false;
        uint16_t BGColor = TFT_BLACK;
        if (!NeedsDrawImage && !force)
            return;
        NeedsDrawImage = false;
        if (IsLabel)
        {
            return;
        }

        LOC_LOGV(module, "Getting active image for button");
        auto image = GetActiveImage();
        if (!image->valid)
        {
            LOC_LOGD(module, "Skipping invalid image draw");
            return;
        }
        LOC_LOGV(module, "Image draw of button %s", image->LogoName.c_str());
        uint16_t ImagePixelColor = image->GetPixelColor();
        if (ImagePixelColor == TFT_BLACK)
        {
            LOC_LOGV(module, "Corner pixel is black, drawing transparent with default background color");
            transparent = true;
            BGColor = convertRGB888ToRGB565(BackgroundColor);
        }
        else
        {
            // Draw the button with found image's background color
            BGColor = convertRGB888ToRGB565(ImagePixelColor);
        }
        image->Draw(CenterX, CenterY, transparent);
    }
    void FTButton::Draw(bool force)
    {
        DrawShape(force);
        DrawImage(force);
    }
    uint16_t FTButton::Width()
    {
        if (!GetActiveImage()->valid)
        {
            return 0;
        }

        return GetActiveImage()->w;
    }
    uint16_t FTButton::Height()
    {
        if (!GetActiveImage()->valid)
        {
            return 0;
        }
        return GetActiveImage()->h;
    }
    bool FTButton::HasKeyboardActions()
    {
        for (ActionsSequences &sequence : Sequences)
        {
            if (sequence.HasKeyboardAction())
                return true;
        }
        return false;
    }
    void FTButton::ExecuteActions()
    {
        LOC_LOGD(module, "%s Button Press detected with %d actions", enum_to_string(ButtonType), Sequences.size());
        for (ActionsSequences &sequence : Sequences)
        {
            sequence.Execute();
        }
    }
    void FTButton::UnPress()
    {
        if (!IsPressed)
            return;
        LOC_LOGD(module, "Cancelling press for button %s", Label.c_str());
        IsPressed = false;
        FTButton::Invalidate();
        return;
    }
    void FTButton::Press()
    {
        if (IsPressed)
        {
            LOC_LOGV(module, "Button already pressed. Ignoring");
            return;
        }
        LOC_LOGD(module, "Button %s is pressed", Label.c_str());
        HandleAudio(Sounds::BEEP);
        IsPressed = true;
        FTButton::Invalidate();
    }
    void FTButton::Release()
    {
        if (IsPressed)
        {
            LOC_LOGD(module, "Releasing button %s", Label.c_str());
            // Beep

            IsPressed = false;
            if (!bleKeyboard.isConnected() && HasKeyboardActions())
            {
                LOC_LOGW(module, "Ignoring action for button when Bluetooth is not connected");
                HandleAudio(Sounds::ERROR);
            }
            else
            {
                if (ButtonType == ButtonTypes::LATCH)
                {
                    Latched = !Latched;
                    LOC_LOGD(module, "Toggling LATCH to %s", Latched ? "ACTIVE" : "INACTIVE");
                }
                ExecuteActions();
            }
            FTButton::Invalidate();
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
        LOC_LOGD(module, "Adding button members to Json");
        if (!Label.empty())
        {
            cJSON_AddStringToObject(button, FTButton::JsonLabelLabel, Label.c_str());
        }
        if (!_jsonLogo.empty())
        {
            cJSON_AddStringToObject(button, FTButton::JsonLabelLogo, _jsonLogo.c_str());
        }
        if (ButtonType == ButtonTypes::LATCH && !_jsonLatchedLogo.empty())
        {
            cJSON_AddStringToObject(button, FTButton::JsonLabelLatchedLogo, _jsonLatchedLogo.c_str());
        }
        if (ButtonType != ButtonTypes::STANDARD)
        {
            cJSON_AddStringToObject(button, FTButton::JsonLabelType, enum_to_string(ButtonType));
        }

        if (Outline != generalconfig.DefaultOutline)
        {
            cJSON_AddStringToObject(button, FTButton::JsonLabelOutline, convertRGB888oHTMLRGB888(Outline));
        }
        uint32_t baseColor=IsMenu()?generalconfig.functionButtonColour:MenuBackgroundColor;
        if (BackgroundColor != baseColor)
        {
            cJSON_AddStringToObject(button, FTButton::JsonLabelBackground, convertRGB888oHTMLRGB888(BackgroundColor));
        }
        if (TextColor != generalconfig.DefaultTextColor)
        {
            cJSON_AddStringToObject(button, FTButton::JsonLabelTextColor, convertRGB888oHTMLRGB888(TextColor));
        }
        if (TextSize != generalconfig.DefaultTextSize)
        {
            cJSON_AddNumberToObject(button, FTButton::JsonLabelTextSize, TextSize);
        }
        LOC_LOGD(module, "Adding actions to Json");
        if (Sequences.size() > 0)
        {
            cJSON *actionsJson = cJSON_CreateArray();
            for (auto sequence : Sequences)
            {
                if (!ISNULLSTRING(sequence.ConfigSequence))
                {
                    cJSON_AddItemToArray(actionsJson, cJSON_CreateString(sequence.ConfigSequence));
                }
            }
            cJSON_AddItemToObject(button, FTButton::JsonLabelActions, actionsJson);
        }
        if (generalconfig.LogLevel >= LogLevels::VERBOSE)
        {
            char *buttonString = cJSON_Print(button);
            if (buttonString)
            {
                LOC_LOGD(module, "Button json structure : \n%s", buttonString);
                FREE_AND_NULL(buttonString);
            }
            else
            {
                LOC_LOGE(module, "Unable to format JSON for output!");
            }
        }
        return button;
    }
    bool FTButton::contains(uint16_t x, uint16_t y)
    {
        //        _button.initButton(&tft, CenterX, CenterY, adjustedWidth, adjustedHeight, Outline, BGColor, convertRGB888ToRGB565(TextColor), (char *)buttonLabel, TextSize);
        return ((x >= X) && (x < (X + AdjustedWidth)) && (y >= Y) && (y < (Y + AdjustedHeight)));
    }
    void FTButton::Init(cJSON *button)
    {
        char *buttonType = NULL;
        PrintMemInfo(__FUNCTION__, __LINE__);
        GetValueOrDefault(button, FTButton::JsonLabelType, &buttonType, enum_to_string(ButtonTypes::STANDARD));
        ButtonType = parse_button_types(buttonType);
        FREE_AND_NULL(buttonType);
        GetValueOrDefault(button, FTButton::JsonLabelLabel, Label, NULL);
        GetValueOrDefault(button, FTButton::JsonLabelLogo, _jsonLogo, NULL);
        IsLabel = !ImageCache::GetImage(_jsonLogo)->valid;
        if (Label.empty())
        {
            Label = _jsonLogo;
        }
        LOC_LOGD(module, "Button type is [%s], Label: [%s], Logo : [%s], draw type: [%s]", enum_to_string(ButtonType), Label.c_str(), _jsonLogo.c_str(), IsLabel ? "LABEL" : "IMAGE");
        if (ButtonType == ButtonTypes::LATCH)
        {
            GetValueOrDefault(button, FTButton::JsonLabelLatchedLogo, _jsonLatchedLogo, NULL);
            LOC_LOGD(module, "Latched logo: %s", _jsonLatchedLogo.c_str());
        }
        GetColorOrDefault(button, FTButton::JsonLabelOutline, &Outline, generalconfig.DefaultOutline);
        GetColorOrDefault(button, FTButton::JsonLabelTextColor, &TextColor, generalconfig.DefaultTextColor);
        GetValueOrDefault(button, FTButton::JsonLabelTextSize, &TextSize, generalconfig.DefaultTextSize);

        cJSON *jsonActions = cJSON_GetObjectItem(button, FTButton::JsonLabelActions);
        if (!jsonActions)
        {
            LOC_LOGW(module, "Button %s does not have any action!", Label.c_str());
        }
        else
        {
            if (!cJSON_IsArray(jsonActions))
            {
                LOC_LOGE(module, "Actions object for button %s should be an array but it is not.", Label.c_str());
            }
            cJSON *actionJson = NULL;
            cJSON_ArrayForEach(actionJson, jsonActions)
            {
                ActionsSequences sequence;
                if (sequence.Parse(actionJson))
                {
                    Sequences.push_back(sequence);
                }
            }
        }
        // re-determine color at the end, since we need to know if there are menu actions 
        ButtonType=IsMenu()?ButtonTypes::MENU:ButtonType;
        GetColorOrDefault(button, FTButton::JsonLabelBackground, &BackgroundColor, IsMenu()?generalconfig.functionButtonColour:MenuBackgroundColor);

        PrintMemInfo(__FUNCTION__, __LINE__);
    }
    FTButton::FTButton(cJSON *button)
    {
        Init(button);
    }
    FTButton::FTButton(cJSON *button, uint32_t backgroundColor, uint32_t outline, uint32_t textColor)
    {
        BackgroundColor = backgroundColor;
        Outline = outline;
        TextColor = textColor;
        MenuBackgroundColor = backgroundColor;
        Init(button);
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
