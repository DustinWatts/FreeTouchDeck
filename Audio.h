#pragma once
namespace FreeTouchDeck
{

  enum class Sounds
  {
    GOING_TO_SLEEP,
    BEEP,
    STARTUP
  };
  void HandleAudio(Sounds sound);
}