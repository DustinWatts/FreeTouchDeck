/**
* @brief This function takes an int as an action and value. It uses 
         a switch statement to determine which type of action to do.
         e.g. write, print, press.
*
* @param action int 
* @param value int
* @param symbol char *
*
* @return none
*
* @note Case 11 is used for special functions, none bleKeyboard related.
*/

void bleKeyboardAction(int action, int value, char *symbol)
{

  Serial.println("[INFO]: BLE Keyboard action received");
  switch (action)
  {
  case 0:
    // No Action
    break;
  case 1: // Delay
    delay(value);
    break;
  case 2: // Send TAB ARROW etc
    switch (value)
    {
    case 1:
      bleKeyboard.write(KEY_UP_ARROW);
      break;
    case 2:
      bleKeyboard.write(KEY_DOWN_ARROW);
      break;
    case 3:
      bleKeyboard.write(KEY_LEFT_ARROW);
      break;
    case 4:
      bleKeyboard.write(KEY_RIGHT_ARROW);
      break;
    case 5:
      bleKeyboard.write(KEY_BACKSPACE);
      break;
    case 6:
      bleKeyboard.write(KEY_TAB);
      break;
    case 7:
      bleKeyboard.write(KEY_RETURN);
      break;
    case 8:
      bleKeyboard.write(KEY_PAGE_UP);
      break;
    case 9:
      bleKeyboard.write(KEY_PAGE_DOWN);
      break;
    default:
      //if nothing matches do nothing
      break;
    }
    break;
  case 3: // Send Media Key
    switch (value)
    {
    case 1:
      bleKeyboard.write(KEY_MEDIA_MUTE);
      break;
    case 2:
      bleKeyboard.write(KEY_MEDIA_VOLUME_DOWN);
      break;
    case 3:
      bleKeyboard.write(KEY_MEDIA_VOLUME_UP);
      break;
    case 4:
      bleKeyboard.write(KEY_MEDIA_PLAY_PAUSE);
      break;
    case 5:
      bleKeyboard.write(KEY_MEDIA_STOP);
      break;
    case 6:
      bleKeyboard.write(KEY_MEDIA_NEXT_TRACK);
      break;
    case 7:
      bleKeyboard.write(KEY_MEDIA_PREVIOUS_TRACK);
      break;
    default:
      //if nothing matches do nothing
      break;
    }
    break;
  case 4: // Send Character
    bleKeyboard.print(symbol);
    break;
  case 5: // Option Keys
    switch (value)
    {
    case 1:
      bleKeyboard.press(KEY_LEFT_CTRL);
      break;
    case 2:
      bleKeyboard.press(KEY_LEFT_SHIFT);
      break;
    case 3:
      bleKeyboard.press(KEY_LEFT_ALT);
      break;
    case 4:
      bleKeyboard.press(KEY_LEFT_GUI);
      break;
    case 5:
      bleKeyboard.press(KEY_RIGHT_CTRL);
      break;
    case 6:
      bleKeyboard.press(KEY_RIGHT_SHIFT);
      break;
    case 7:
      bleKeyboard.press(KEY_RIGHT_ALT);
      break;
    case 8:
      bleKeyboard.press(KEY_RIGHT_GUI);
      break;
    case 9:
      bleKeyboard.releaseAll();
      break;
    default:
      //if nothing matches do nothing
      break;
    }
    break;
  case 6: // Function Keys
    switch (value)
    {
    case 1:
      bleKeyboard.press(KEY_F1);
      break;
    case 2:
      bleKeyboard.press(KEY_F2);
      break;
    case 3:
      bleKeyboard.press(KEY_F3);
      break;
    case 4:
      bleKeyboard.press(KEY_F4);
      break;
    case 5:
      bleKeyboard.press(KEY_F5);
      break;
    case 6:
      bleKeyboard.press(KEY_F6);
      break;
    case 7:
      bleKeyboard.press(KEY_F7);
      break;
    case 8:
      bleKeyboard.press(KEY_F8);
      break;
    case 9:
      bleKeyboard.press(KEY_F9);
      break;
    case 10:
      bleKeyboard.press(KEY_F10);
      break;
    case 11:
      bleKeyboard.press(KEY_F11);
      break;
    case 12:
      bleKeyboard.press(KEY_F12);
      break;
    case 13:
      bleKeyboard.press(KEY_F13);
      break;
    case 14:
      bleKeyboard.press(KEY_F14);
      break;
    case 15:
      bleKeyboard.press(KEY_F15);
      break;
    case 16:
      bleKeyboard.press(KEY_F16);
      break;
    case 17:
      bleKeyboard.press(KEY_F17);
      break;
    case 18:
      bleKeyboard.press(KEY_F18);
      break;
    case 19:
      bleKeyboard.press(KEY_F19);
      break;
    case 20:
      bleKeyboard.press(KEY_F20);
      break;
    case 21:
      bleKeyboard.press(KEY_F21);
      break;
    case 22:
      bleKeyboard.press(KEY_F22);
      break;
    case 23:
      bleKeyboard.press(KEY_F23);
      break;
    case 24:
      bleKeyboard.press(KEY_F24);
      break;
    default:
      //if nothing matches do nothing
      break;
    }
    break;
  case 7: // Send Number
    bleKeyboard.print(value);
    break;
  case 8: // Send Special Character
    bleKeyboard.print(symbol);
    break;
  case 9: // Combos
    switch (value)
    {
    case 1:
      bleKeyboard.press(KEY_LEFT_CTRL);
      bleKeyboard.press(KEY_LEFT_SHIFT);
      break;
    case 2:
      bleKeyboard.press(KEY_LEFT_ALT);
      bleKeyboard.press(KEY_LEFT_SHIFT);
      break;
    case 3:
      bleKeyboard.press(KEY_LEFT_GUI);
      bleKeyboard.press(KEY_LEFT_SHIFT);
      break;
    case 4:
      bleKeyboard.press(KEY_LEFT_CTRL);
      bleKeyboard.press(KEY_LEFT_GUI);
      break;
    case 5:
      bleKeyboard.press(KEY_LEFT_ALT);
      bleKeyboard.press(KEY_LEFT_GUI);
      break;
    case 6:
      bleKeyboard.press(KEY_LEFT_CTRL);
      bleKeyboard.press(KEY_LEFT_ALT);
      break;
    case 7:
      bleKeyboard.press(KEY_LEFT_CTRL);
      bleKeyboard.press(KEY_LEFT_ALT);
      bleKeyboard.press(KEY_LEFT_GUI);
      break;
    case 8:
      bleKeyboard.press(KEY_RIGHT_CTRL);
      bleKeyboard.press(KEY_RIGHT_SHIFT);
      break;
    case 9:
      bleKeyboard.press(KEY_RIGHT_ALT);
      bleKeyboard.press(KEY_RIGHT_SHIFT);
      break;
    case 10:
      bleKeyboard.press(KEY_RIGHT_GUI);
      bleKeyboard.press(KEY_RIGHT_SHIFT);
      break;
    case 11:
      bleKeyboard.press(KEY_RIGHT_CTRL);
      bleKeyboard.press(KEY_RIGHT_GUI);
      break;
    case 12:
      bleKeyboard.press(KEY_RIGHT_ALT);
      bleKeyboard.press(KEY_RIGHT_GUI);
      break;
    case 13:
      bleKeyboard.press(KEY_RIGHT_CTRL);
      bleKeyboard.press(KEY_RIGHT_ALT);
      break;
    case 14:
      bleKeyboard.press(KEY_RIGHT_CTRL);
      bleKeyboard.press(KEY_RIGHT_ALT);
      bleKeyboard.press(KEY_RIGHT_GUI);
      break;
    }
    break;
  case 10: // future feature
    break;
  case 11: // Special functions
    switch (value)
    {
    case 1:        // Enter config mode
      
      if(configmode()){
        pageNum = 7; // By setting pageNum to 7
        drawKeypad(); // and calling drawKeypad() a new keypad is drawn with pageNum 7
      }else{
        pageNum = 9; // By setting pageNum to 9
        drawKeypad(); // and calling drawKeypad() we are drawing an error message that we are not able to connect to WiFi
      }
      break;
    case 2: // Display Brightness Down
      if (ledBrightness > 25)
      {
        ledBrightness = ledBrightness - 25;
        ledcWrite(0, ledBrightness);
      }
      break;
    case 3: // Display Brightness Up
      if (ledBrightness < 230)
      {
        ledBrightness = ledBrightness + 25;
        ledcWrite(0, ledBrightness);
      }
      break;
    case 4: // Sleep Enabled
      if (wificonfig.sleepenable)
      {
        wificonfig.sleepenable = false;
        Serial.println("[INFO]: Sleep disabled.");
      }
      else
      {
        wificonfig.sleepenable = true;
        Interval = wificonfig.sleeptimer * 60000;
        Serial.println("[INFO]: Sleep enabled.");
        Serial.print("[INFO]: Timer set to: ");
        Serial.println(wificonfig.sleeptimer);
      }
      break;
    }
    break;
  default:
    //If nothing matches do nothing
    break;
  }
}
