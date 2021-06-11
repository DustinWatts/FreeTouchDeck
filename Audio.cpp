#include "globals.hpp"
#include "ConfigLoad.h"
#include "Audio.h"
namespace FreeTouchDeck
{

    void AudioChipTune(Sounds sound)
    {
        switch (sound)
        {
        case Sounds::GOING_TO_SLEEP:
            if (generalconfig.beep)
            {
                ledcAttachPin(speakerPin, 2);
                ledcWriteTone(2, 1200);
                delay(150);
                ledcDetachPin(speakerPin);
                ledcWrite(2, 0);

                ledcAttachPin(speakerPin, 2);
                ledcWriteTone(2, 800);
                delay(150);
                ledcDetachPin(speakerPin);
                ledcWrite(2, 0);

                ledcAttachPin(speakerPin, 2);
                ledcWriteTone(2, 600);
                delay(150);
                ledcDetachPin(speakerPin);
                ledcWrite(2, 0);
            }
            break;
        case Sounds::BEEP:
            if (generalconfig.beep)
            {
                ledcAttachPin(speakerPin, 2);
                ledcWriteTone(2, 600);
                delay(50);
                ledcDetachPin(speakerPin);
                ledcWrite(2, 0);
            }
            break;
        case Sounds::STARTUP:
            if (generalconfig.beep)
            {
                ledcAttachPin(speakerPin, 2);
                ledcWriteTone(2, 600);
                delay(150);
                ledcDetachPin(speakerPin);
                ledcWrite(2, 0);

                ledcAttachPin(speakerPin, 2);
                ledcWriteTone(2, 800);
                delay(150);
                ledcDetachPin(speakerPin);
                ledcWrite(2, 0);

                ledcAttachPin(speakerPin, 2);
                ledcWriteTone(2, 1200);
                delay(150);
                ledcDetachPin(speakerPin);
                ledcWrite(2, 0);
            }
            break;
        default:
            break;
        }
    }
    void HandleAudio(Sounds sound)
    {
        if (speakerPin >= 0)
        {
            static bool isInit = false;

            if (!isInit)
            {
                // Setup PWM channel for Piezo speaker
                ledcSetup(2, 500, 8);
                isInit = true;
            }
            AudioChipTune(sound);
        }
        // todo:  add support for i2s audio
    }
    
}