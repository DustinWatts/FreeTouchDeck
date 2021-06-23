#pragma once
namespace FreeTouchDeck
{

  enum class Sounds
  {
    GOING_TO_SLEEP,
    BEEP,
    STARTUP,
    ERROR
  };
  void HandleAudio(Sounds sound);
}