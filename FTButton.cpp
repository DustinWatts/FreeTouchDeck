#include "FTButton.h"
#include "Audio.h"
#include "esp_attr.h"
#include "WString.h"
#include "ImageCache.h"
static const char *module = "FTButton";
namespace FreeTouchDeck
{
    //static NO_INT std::map<std::string,bool> LatchList;

    const char *enum_to_string(ButtonTypes type)
    {
        switch (type)
        {
            ENUM_TO_STRING_HELPER(ButtonTypes, STANDARD);
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
    FTButton *FTButton::BackButton = NULL;
    FTButton *FTButton::HomeButton = NULL;
    void FTButton::InitConstants()
    {
        BackButton = new FTButton(FTButton::backButtonTemplate, true);
        HomeButton = new FTButton(FTButton::homeButtonTemplate, true);
    }
    FTButton::FTButton()
    {
        ButtonType = ButtonTypes::NONE;
    }
    FTButton::FTButton(ButtonTypes buttonType, const char *label, const char *logo, const char *latchedLogo, uint32_t outline, uint8_t textSize, uint32_t textColor)
    {
        ButtonType = buttonType;
        Label = ps_strdup(STRING_OR_DEFAULT(label, ""));
        if (!ISNULLSTRING(logo))
        {
            _jsonLogo = ps_strdup(STRING_OR_DEFAULT(logo, ""));
        }
        if (!ISNULLSTRING(latchedLogo))
        {
            _jsonLatchedLogo = ps_strdup(STRING_OR_DEFAULT(latchedLogo, ""));
        }

        Outline = outline;
        TextSize = textSize;
        TextColor = textColor;
        LOC_LOGD(module, "Button type is [%s], Label: [%s], Logo : [%s]", enum_to_string(ButtonType), STRING_OR_DEFAULT(Label, ""), STRING_OR_DEFAULT(_jsonLogo, ""));
    }
    FTButton::FTButton(const char *jsonString, bool isShared)
    {
        IsShared = isShared;
        MenuBackgroundColor = generalconfig.backgroundColour;
        cJSON *doc = cJSON_Parse(jsonString);
        if (!doc)
        {
            const char *error = cJSON_GetErrorPtr();
            drawErrorMessage(true, module, "Unable to parse json string : %s", error);
        }
        else
        {
            Init(doc);
        }
        cJSON_Delete(doc);
    }
    ImageWrapper *FTButton::LatchedLogo()
    {
        ImageWrapper *image = NULL;
        if (!ISNULLSTRING(_jsonLatchedLogo))
        {
            LOC_LOGV(module, "Latched Logo file name is %s", _jsonLatchedLogo);
            image = ImageCache::GetImage(_jsonLatchedLogo);
            if (image && !image->valid)
            {
                LOC_LOGD(module, "Latched Logo file %s is invalid.", _jsonLatchedLogo);
                image = NULL;
            }
        }
        return image;
    }
    bool FTButton::IsLabelDraw()
    {
        ImageWrapper *image = GetActiveImage();
        
        if (!image || !image->valid)
        {
            return true;
        }
        return false;
    }
    ImageWrapper *FTButton::Logo()
    {
        ImageWrapper *image = NULL;

        if (!ISNULLSTRING(_jsonLogo))
        {
            LOC_LOGD(module, "Logo file name is [%s]", _jsonLogo);
            image = ImageCache::GetImage(_jsonLogo);
            if (!image || !image->valid)
            {
                LOC_LOGD(module, "Logo file name [%s] was not found or is invalid.", STRING_OR_DEFAULT(_jsonLogo, ""));
            }
        }
        else
        {
            LOC_LOGV(module, "No image logo for button");
        }
        return image;
    }
    bool FTButton::Latch(FTAction *action)
    {
        LOC_LOGD(module, "Button is Executing Action %s", action->toString());
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
            LOC_LOGW(module, "Cannot latch button. Button %s has type %s.", STRING_OR_DEFAULT(Label, STRING_OR_DEFAULT(_jsonLogo, "")), enum_to_string(ButtonType));
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
        if (!ISNULLSTRING(Label))
            std::cout << "Deleting button " << Label << "\n";
        FREE_AND_NULL(Label);
        if (!ISNULLSTRING(_jsonLogo))
            std::cout << "deleting logo " << _jsonLogo << "\n";
        FREE_AND_NULL(_jsonLogo);
        FREE_AND_NULL(_jsonLatchedLogo);
    }
    void FTButton::Invalidate()
    {
        NeedsDraw = true;
        NeedsDrawImage = true;
    }
    ImageWrapper *FTButton::GetActiveImage()
    {
        ImageWrapper *image = NULL;
        if (Latched)
        {
            image = LatchedLogo();
        }
        if (!image || !image->valid)
        {
            LOC_LOGV(module, "Getting button's logo");
            image = Logo();
        }
        if (!image || !image->valid)
        {
            if(strlen(Label) == 0 || (Label[0] == '?' && strlen(_jsonLogo)>0 ))
            {
                String LogoName = _jsonLogo;
                int dotPos =LogoName.lastIndexOf(".")-1;
                dotPos=dotPos>0?dotPos:LogoName.length()-1;
                LogoName.substring(0,dotPos);
                FREE_AND_NULL(_jsonLogo);
                _jsonLogo = ps_strdup(LogoName.c_str());
                LOC_LOGW(module,"Defaulting Label to Logo icon name %s",_jsonLogo);
            }
        }        
        return image;
    }
    void IRAM_ATTR FTButton::DrawShape(bool force)
    {
        bool transparent = false;
        int32_t radius = 4;
        uint8_t textSize = TextSize;
        uint16_t BGColor = TFT_BLACK;
        bool labelDraw = false;
        if (!NeedsDraw && !force)
            return;

        NeedsDraw = false;
        const char *buttonLabel = "";
        LOC_LOGV(module, "Getting active image for button");
        ImageWrapper *image = GetActiveImage();
        if (!image || !image->valid)
        {
            SetDefaultFont();
            tft.setTextSize(TextSize);
            buttonLabel = STRING_OR_DEFAULT(Label, "");
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
            BGColor = convertRGB888ToRGB565(BackgroundColor);
            labelDraw = true;
        }
        else
        {
            LOC_LOGD(module, "Image draw of button %s", STRING_OR_DEFAULT(image->LogoName, ""));
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
        }

        LOC_LOGV(module, "Drawing button at [%d,%d] size: %dx%d,  outline : 0x%04X, BG Color: 0x%04X, Text color: 0x%04X, Text size: %d, logo: %s", CenterX, CenterY, AdjustedWidth, AdjustedHeight, Outline, BGColor, TextColor, TextSize, STRING_OR_DEFAULT(_jsonLogo, ""));
        PrintMemInfo();

        uint8_t r = min(ButtonWidth, ButtonHeight) / 4; // Corner radius
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
        if (ButtonType == ButtonTypes::LATCH && !LatchedLogo())
        {
            // Draw a rounded rectangle in the button corner
            uint32_t roundRectWidth = ButtonWidth / 4;
            uint32_t roundRectHeight = ButtonHeight / 4;
            uint32_t cornerX = CenterX - (AdjustedWidth / 2) + roundRectWidth / 2;
            uint32_t cornerY = CenterY - (AdjustedHeight / 2) + roundRectHeight / 2;
            if (Latched)
            {
                LOC_LOGD(module, "Latched without a latched logo.  Drawing round rectangle");
                tft.fillRoundRect(cornerX, cornerY, roundRectWidth, roundRectHeight, radius, generalconfig.latchedColour);
            }
            else
            {
                LOC_LOGD(module, "Latched without a latched logo.  Drawing round rectangle");
                tft.fillRoundRect(cornerX, cornerY, roundRectWidth, roundRectHeight, radius, BGColor);
            }
        }

        if (labelDraw)
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
        LOC_LOGV(module, "Getting active image for button");
        ImageWrapper *image = GetActiveImage();
        if (!image || !image->valid)
        {
            LOC_LOGD(module,"Skipping invalid image draw");
            return;
        }
        LOC_LOGV(module, "Image draw of button %s", STRING_OR_DEFAULT(image->LogoName, ""));
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
        if (!GetActiveImage() || !GetActiveImage()->valid)
        {
            return 0;
        }

        return GetActiveImage()->w;
    }
    uint16_t FTButton::Height()
    {
        if (!GetActiveImage() || !GetActiveImage()->valid)
        {
            return 0;
        }
        return GetActiveImage()->h;
    }
    void FTButton::ExecuteActions()
    {
        LOC_LOGD(module, "%s Button Press detected with %d actions", enum_to_string(ButtonType), Sequences.size());
        for (auto sequence : Sequences)
        {
            sequence.Execute();
        }
    }
    void FTButton::UnPress()
    {
        if (!IsPressed) return;
        LOC_LOGD(module, "Cancelling press for button %s",
        STRING_OR_DEFAULT(Label, 
            STRING_OR_DEFAULT((GetActiveImage() && GetActiveImage()->valid?GetActiveImage()->LogoName:""), "")));
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
        HandleAudio(Sounds::BEEP);
        IsPressed = true;
        FTButton::Invalidate();
    }
    void FTButton::Release()
    {
        if (IsPressed)
        {
            LOC_LOGD(module, "Releasing button %s", STRING_OR_DEFAULT(Label, STRING_OR_DEFAULT(Logo()->LogoName, "")));
            // Beep

            if (ButtonType == ButtonTypes::LATCH)
            {
                Latched = !Latched;
                LOC_LOGD(module, "Toggling LATCH to %s", Latched ? "ACTIVE" : "INACTIVE");
            }            
            IsPressed = false;
            FTButton::Invalidate();
            ExecuteActions();
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
        if (!ISNULLSTRING(Label))
        {
            cJSON_AddStringToObject(button, FTButton::JsonLabelLabel, STRING_OR_DEFAULT(Label, ""));
        }
        if (!ISNULLSTRING(_jsonLogo))
        {
            cJSON_AddStringToObject(button, FTButton::JsonLabelLogo, _jsonLogo);
        }
        if (ButtonType == ButtonTypes::LATCH && !ISNULLSTRING(_jsonLatchedLogo))
        {
            cJSON_AddStringToObject(button, FTButton::JsonLabelLatchedLogo, _jsonLatchedLogo);
        }
        if (ButtonType != ButtonTypes::STANDARD)
        {
            cJSON_AddStringToObject(button, FTButton::JsonLabelType, enum_to_string(ButtonType));
        }

        if (Outline != generalconfig.DefaultOutline)
        {
            cJSON_AddStringToObject(button, FTButton::JsonLabelOutline, convertRGB888oHTMLRGB888(Outline));
        }
        if (BackgroundColor != MenuBackgroundColor)
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
        GetValueOrDefault(button, FTButton::JsonLabelType, &buttonType, enum_to_string(ButtonTypes::STANDARD));
        ButtonType = parse_button_types(buttonType);
        FREE_AND_NULL(buttonType);
        GetValueOrDefault(button, FTButton::JsonLabelLabel, &Label, "?");
        GetValueOrDefault(button, FTButton::JsonLabelLogo, &_jsonLogo, NULL);
        LOC_LOGD(module, "Button type is [%s], Label: [%s], Logo : [%s]", enum_to_string(ButtonType), STRING_OR_DEFAULT(Label, ""), STRING_OR_DEFAULT(_jsonLogo, ""));
        if (ButtonType == ButtonTypes::LATCH)
        {
            GetValueOrDefault(button, FTButton::JsonLabelLatchedLogo, &_jsonLatchedLogo, NULL);
            LOC_LOGD(module, "Latched logo: %s", STRING_OR_DEFAULT(_jsonLatchedLogo, ""));
        }
        GetColorOrDefault(button, FTButton::JsonLabelOutline, &Outline, generalconfig.DefaultOutline);
        GetColorOrDefault(button, FTButton::JsonLabelBackground, &BackgroundColor, MenuBackgroundColor);
        GetColorOrDefault(button, FTButton::JsonLabelTextColor, &TextColor, generalconfig.DefaultTextColor);
        GetValueOrDefault(button, FTButton::JsonLabelTextSize, &TextSize, generalconfig.DefaultTextSize);

        cJSON *jsonActions = cJSON_GetObjectItem(button, FTButton::JsonLabelActions);
        if (!jsonActions)
        {
            LOC_LOGW(module, "Button %s does not have any action!", STRING_OR_DEFAULT(Label, STRING_OR_DEFAULT(Logo()->LogoName, "")));
        }
        else
        {
            if (!cJSON_IsArray(jsonActions))
            {
                LOC_LOGE(module, "Actions object for button %s should be an array but it is not.", Label);
            }
            cJSON *actionJson = NULL;
            cJSON_ArrayForEach(actionJson, jsonActions)
            {
                ActionsSequences sequence;
                if (!sequence.Parse(actionJson))
                {
                    LOC_LOGE(module, "Could not parse action %s", cJSON_IsString(actionJson) ? STRING_OR_DEFAULT(cJSON_GetStringValue(actionJson), "") : "");
                }
                else
                {
                    Sequences.push_back(sequence);
                }
            }
        }
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
