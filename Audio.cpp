#include "globals.hpp"
#include "ConfigLoad.h"
#include "Audio.h"
namespace FreeTouchDeck
{
    void ChipTune(double freq, uint16_t duration)
    {
        if (generalconfig.beep)
        {
            ledcAttachPin(speakerPin, 2);
            ledcWriteTone(2, freq);
            delay(duration);
            ledcDetachPin(speakerPin);
            ledcWrite(2, 0);
        }
    }
    void AudioChipTune(Sounds sound)
    {
        switch (sound)
        {
        case Sounds::GOING_TO_SLEEP:
            ChipTune(1200,150);
            ChipTune(800,150);
            ChipTune(600,150);
            break;
        case Sounds::BEEP:
            ChipTune(600,50);
            break;
        case Sounds::STARTUP:
            ChipTune(600,150);
            ChipTune(800,150);
            ChipTune(1200,150);
            break;
        case Sounds::ERROR:
            ChipTune(500,50);
            ChipTune(300,250);
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