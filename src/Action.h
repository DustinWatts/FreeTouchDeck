/**
* @brief This function takes an int as an "action" and "value". It uses
         a switch statement to determine which type of action to do.
         e.g. write, print, press. If an action requires a char, you
         can pass the pointer to that char through the parameter "symbol"
*
* @param action int
* @param value int
* @param symbol char *
*
* @return none
*
* @note Case 11 is used for special functions, none bleKeyboard related.
*/

void bleKeyboardAction(int action, int value, char *symbol) {

  Serial.println("[INFO]: BLE Keyboard action received");

  switch (action) {
  case 0:
    // No Action
    break;
  case 1: // Delay
    delay(value);
    break;
  case 2: // Send TAB ARROW etc
    switch (value) {
    case 1:
      bleCombo.write(KEY_UP_ARROW);
      break;
    case 2:
      bleCombo.write(KEY_DOWN_ARROW);
      break;
    case 3:
      bleCombo.write(KEY_LEFT_ARROW);
      break;
    case 4:
      bleCombo.write(KEY_RIGHT_ARROW);
      break;
    case 5:
      bleCombo.write(KEY_BACKSPACE);
      break;
    case 6:
      bleCombo.write(KEY_TAB);
      break;
    case 7:
      bleCombo.write(KEY_RETURN);
      break;
    case 8:
      bleCombo.write(KEY_PAGE_UP);
      break;
    case 9:
      bleCombo.write(KEY_PAGE_DOWN);
      break;
    case 10:
      bleCombo.write(KEY_DELETE);
      break;
    case 11:
      bleCombo.write(KEY_PRTSC);
      break;
    case 12:
      bleCombo.write(KEY_ESC);
      break;
    case 13:
      bleCombo.write(KEY_HOME);
      break;
    case 14:
      bleCombo.write(KEY_END);
      break;

    default:
      // if nothing matches do nothing
      break;
    }
    break;
  case 3: // Send Media Key
#if defined(USEUSBHID)

#else
    switch (value) {
    case 1:
      bleCombo.write(KEY_MEDIA_MUTE);
      break;
    case 2:
      bleCombo.write(KEY_MEDIA_VOLUME_DOWN);
      break;
    case 3:
      bleCombo.write(KEY_MEDIA_VOLUME_UP);
      break;
    case 4:
      bleCombo.write(KEY_MEDIA_PLAY_PAUSE);
      break;
    case 5:
      bleCombo.write(KEY_MEDIA_STOP);
      break;
    case 6:
      bleCombo.write(KEY_MEDIA_NEXT_TRACK);
      break;
    case 7:
      bleCombo.write(KEY_MEDIA_PREVIOUS_TRACK);
      break;
    default:
      // if nothing matches do nothing
      break;
    }
    break;
#endif    // if defined(USEUSBHID)
  case 4: // Send Character
    bleCombo.print(symbol);
    break;
  case 5: // Option Keys
    switch (value) {
    case 1:
      bleCombo.keyPress(KEY_LEFT_CTRL);
      break;
    case 2:
      bleCombo.keyPress(KEY_LEFT_SHIFT);
      break;
    case 3:
      bleCombo.keyPress(KEY_LEFT_ALT);
      break;
    case 4:
      bleCombo.keyPress(KEY_LEFT_GUI);
      break;
    case 5:
      bleCombo.keyPress(KEY_RIGHT_CTRL);
      break;
    case 6:
      bleCombo.keyPress(KEY_RIGHT_SHIFT);
      break;
    case 7:
      bleCombo.keyPress(KEY_RIGHT_ALT);
      break;
    case 8:
      bleCombo.keyPress(KEY_RIGHT_GUI);
      break;
    case 9:
      bleCombo.keyReleaseAll();
      break;
    default:
      // if nothing matches do nothing
      break;
    }
    break;
  case 6: // Function Keys
    switch (value) {
    case 1:
      bleCombo.keyPress(KEY_F1);
      break;
    case 2:
      bleCombo.keyPress(KEY_F2);
      break;
    case 3:
      bleCombo.keyPress(KEY_F3);
      break;
    case 4:
      bleCombo.keyPress(KEY_F4);
      break;
    case 5:
      bleCombo.keyPress(KEY_F5);
      break;
    case 6:
      bleCombo.keyPress(KEY_F6);
      break;
    case 7:
      bleCombo.keyPress(KEY_F7);
      break;
    case 8:
      bleCombo.keyPress(KEY_F8);
      break;
    case 9:
      bleCombo.keyPress(KEY_F9);
      break;
    case 10:
      bleCombo.keyPress(KEY_F10);
      break;
    case 11:
      bleCombo.keyPress(KEY_F11);
      break;
    case 12:
      bleCombo.keyPress(KEY_F12);
      break;
    case 13:
      bleCombo.keyPress(KEY_F13);
      break;
    case 14:
      bleCombo.keyPress(KEY_F14);
      break;
    case 15:
      bleCombo.keyPress(KEY_F15);
      break;
    case 16:
      bleCombo.keyPress(KEY_F16);
      break;
    case 17:
      bleCombo.keyPress(KEY_F17);
      break;
    case 18:
      bleCombo.keyPress(KEY_F18);
      break;
    case 19:
      bleCombo.keyPress(KEY_F19);
      break;
    case 20:
      bleCombo.keyPress(KEY_F20);
      break;
    case 21:
      bleCombo.keyPress(KEY_F21);
      break;
    case 22:
      bleCombo.keyPress(KEY_F22);
      break;
    case 23:
      bleCombo.keyPress(KEY_F23);
      break;
    case 24:
      bleCombo.keyPress(KEY_F24);
      break;
    default:
      // if nothing matches do nothing
      break;
    }
    break;
  case 7: // Send Number
    bleCombo.print(value);
    break;
  case 8: // Send Special Character
    bleCombo.print(symbol);
    break;
  case 9: // Combos
    switch (value) {
    case 1:
      bleCombo.keyPress(KEY_LEFT_CTRL);
      bleCombo.keyPress(KEY_LEFT_SHIFT);
      break;
    case 2:
      bleCombo.keyPress(KEY_LEFT_ALT);
      bleCombo.keyPress(KEY_LEFT_SHIFT);
      break;
    case 3:
      bleCombo.keyPress(KEY_LEFT_GUI);
      bleCombo.keyPress(KEY_LEFT_SHIFT);
      break;
    case 4:
      bleCombo.keyPress(KEY_LEFT_CTRL);
      bleCombo.keyPress(KEY_LEFT_GUI);
      break;
    case 5:
      bleCombo.keyPress(KEY_LEFT_ALT);
      bleCombo.keyPress(KEY_LEFT_GUI);
      break;
    case 6:
      bleCombo.keyPress(KEY_LEFT_CTRL);
      bleCombo.keyPress(KEY_LEFT_ALT);
      break;
    case 7:
      bleCombo.keyPress(KEY_LEFT_CTRL);
      bleCombo.keyPress(KEY_LEFT_ALT);
      bleCombo.keyPress(KEY_LEFT_GUI);
      break;
    case 8:
      bleCombo.keyPress(KEY_RIGHT_CTRL);
      bleCombo.keyPress(KEY_RIGHT_SHIFT);
      break;
    case 9:
      bleCombo.keyPress(KEY_RIGHT_ALT);
      bleCombo.keyPress(KEY_RIGHT_SHIFT);
      break;
    case 10:
      bleCombo.keyPress(KEY_RIGHT_GUI);
      bleCombo.keyPress(KEY_RIGHT_SHIFT);
      break;
    case 11:
      bleCombo.keyPress(KEY_RIGHT_CTRL);
      bleCombo.keyPress(KEY_RIGHT_GUI);
      break;
    case 12:
      bleCombo.keyPress(KEY_RIGHT_ALT);
      bleCombo.keyPress(KEY_RIGHT_GUI);
      break;
    case 13:
      bleCombo.keyPress(KEY_RIGHT_CTRL);
      bleCombo.keyPress(KEY_RIGHT_ALT);
      break;
    case 14:
      bleCombo.keyPress(KEY_RIGHT_CTRL);
      bleCombo.keyPress(KEY_RIGHT_ALT);
      bleCombo.keyPress(KEY_RIGHT_GUI);
      break;
    }
    break;
  case 10: // Helpers
    switch (value) {
    case 1:
      if (generalconfig.modifier1 != 0) {
        bleCombo.keyPress(generalconfig.modifier1);
      }
      if (generalconfig.modifier2 != 0) {
        bleCombo.keyPress(generalconfig.modifier2);
      }
      if (generalconfig.modifier3 != 0) {
        bleCombo.keyPress(generalconfig.modifier3);
      }
      bleCombo.keyPress(KEY_F1);
      bleCombo.keyReleaseAll();
      delay(generalconfig.helperdelay);
      break;
    case 2:
      if (generalconfig.modifier1 != 0) {
        bleCombo.keyPress(generalconfig.modifier1);
      }
      if (generalconfig.modifier2 != 0) {
        bleCombo.keyPress(generalconfig.modifier2);
      }
      if (generalconfig.modifier3 != 0) {
        bleCombo.keyPress(generalconfig.modifier3);
      }
      bleCombo.keyPress(KEY_F2);
      bleCombo.keyReleaseAll();
      delay(generalconfig.helperdelay);
      break;
    case 3:
      if (generalconfig.modifier1 != 0) {
        bleCombo.keyPress(generalconfig.modifier1);
      }
      if (generalconfig.modifier2 != 0) {
        bleCombo.keyPress(generalconfig.modifier2);
      }
      if (generalconfig.modifier3 != 0) {
        bleCombo.keyPress(generalconfig.modifier3);
      }
      bleCombo.keyPress(KEY_F3);
      bleCombo.keyReleaseAll();
      delay(generalconfig.helperdelay);
      break;
    case 4:
      if (generalconfig.modifier1 != 0) {
        bleCombo.keyPress(generalconfig.modifier1);
      }
      if (generalconfig.modifier2 != 0) {
        bleCombo.keyPress(generalconfig.modifier2);
      }
      if (generalconfig.modifier3 != 0) {
        bleCombo.keyPress(generalconfig.modifier3);
      }
      bleCombo.keyPress(KEY_F4);
      bleCombo.keyReleaseAll();
      delay(generalconfig.helperdelay);
      break;
    case 5:
      if (generalconfig.modifier1 != 0) {
        bleCombo.keyPress(generalconfig.modifier1);
      }
      if (generalconfig.modifier2 != 0) {
        bleCombo.keyPress(generalconfig.modifier2);
      }
      if (generalconfig.modifier3 != 0) {
        bleCombo.keyPress(generalconfig.modifier3);
      }
      bleCombo.keyPress(KEY_F5);
      bleCombo.keyReleaseAll();
      delay(generalconfig.helperdelay);
      break;
    case 6:
      if (generalconfig.modifier1 != 0) {
        bleCombo.keyPress(generalconfig.modifier1);
      }
      if (generalconfig.modifier2 != 0) {
        bleCombo.keyPress(generalconfig.modifier2);
      }
      if (generalconfig.modifier3 != 0) {
        bleCombo.keyPress(generalconfig.modifier3);
      }
      bleCombo.keyPress(KEY_F6);
      bleCombo.keyReleaseAll();
      delay(generalconfig.helperdelay);
      break;
    case 7:
      if (generalconfig.modifier1 != 0) {
        bleCombo.keyPress(generalconfig.modifier1);
      }
      if (generalconfig.modifier2 != 0) {
        bleCombo.keyPress(generalconfig.modifier2);
      }
      if (generalconfig.modifier3 != 0) {
        bleCombo.keyPress(generalconfig.modifier3);
      }
      bleCombo.keyPress(KEY_F7);
      bleCombo.keyReleaseAll();
      delay(generalconfig.helperdelay);
      break;
    case 8:
      if (generalconfig.modifier1 != 0) {
        bleCombo.keyPress(generalconfig.modifier1);
      }
      if (generalconfig.modifier2 != 0) {
        bleCombo.keyPress(generalconfig.modifier2);
      }
      if (generalconfig.modifier3 != 0) {
        bleCombo.keyPress(generalconfig.modifier3);
      }
      bleCombo.keyPress(KEY_F8);
      bleCombo.keyReleaseAll();
      delay(generalconfig.helperdelay);
      break;
    case 9:
      if (generalconfig.modifier1 != 0) {
        bleCombo.keyPress(generalconfig.modifier1);
      }
      if (generalconfig.modifier2 != 0) {
        bleCombo.keyPress(generalconfig.modifier2);
      }
      if (generalconfig.modifier3 != 0) {
        bleCombo.keyPress(generalconfig.modifier3);
      }
      bleCombo.keyPress(KEY_F9);
      bleCombo.keyReleaseAll();
      delay(generalconfig.helperdelay);
      break;
    case 10:
      if (generalconfig.modifier1 != 0) {
        bleCombo.keyPress(generalconfig.modifier1);
      }
      if (generalconfig.modifier2 != 0) {
        bleCombo.keyPress(generalconfig.modifier2);
      }
      if (generalconfig.modifier3 != 0) {
        bleCombo.keyPress(generalconfig.modifier3);
      }
      bleCombo.keyPress(KEY_F10);
      bleCombo.keyReleaseAll();
      delay(generalconfig.helperdelay);
      break;
    case 11:
      if (generalconfig.modifier1 != 0) {
        bleCombo.keyPress(generalconfig.modifier1);
      }
      if (generalconfig.modifier2 != 0) {
        bleCombo.keyPress(generalconfig.modifier2);
      }
      if (generalconfig.modifier3 != 0) {
        bleCombo.keyPress(generalconfig.modifier3);
      }
      bleCombo.keyPress(KEY_F11);
      bleCombo.keyReleaseAll();
      delay(generalconfig.helperdelay);
      break;
    }
    break;
  case 11: // Special functions
    switch (value) {
    case 1: // Enter config mode

      configmode();

      break;
    case 2: // Display Brightness Down
      if (ledBrightness > 25) {
        ledBrightness = ledBrightness - 25;
        ledcWrite(0, ledBrightness);
        savedStates.putInt("ledBrightness", ledBrightness);
      }
      break;
    case 3: // Display Brightness Up
      if (ledBrightness < 230) {
        ledBrightness = ledBrightness + 25;
        ledcWrite(0, ledBrightness);
        savedStates.putInt("ledBrightness", ledBrightness);
      }
      break;
    case 4: // Sleep Enabled
      if (generalconfig.sleepenable) {
        generalconfig.sleepenable = false;
        Serial.println("[INFO]: Sleep disabled.");
      } else {
        generalconfig.sleepenable = true;
        Interval = generalconfig.sleeptimer * 60000;
        Serial.println("[INFO]: Sleep enabled.");
        Serial.print("[INFO]: Timer set to: ");
        Serial.println(generalconfig.sleeptimer);
      }
      break;
    }
    break;
  case 12: // Numpad
    switch (value) {
    case 0:
      bleCombo.write(KEY_NUM_0);
      break;
    case 1:
      bleCombo.write(KEY_NUM_1);
      break;
    case 2:
      bleCombo.write(KEY_NUM_2);
      break;
    case 3:
      bleCombo.write(KEY_NUM_3);
      break;
    case 4:
      bleCombo.write(KEY_NUM_4);
      break;
    case 5:
      bleCombo.write(KEY_NUM_5);
      break;
    case 6:
      bleCombo.write(KEY_NUM_6);
      break;
    case 7:
      bleCombo.write(KEY_NUM_7);
      break;
    case 8:
      bleCombo.write(KEY_NUM_8);
      break;
    case 9:
      bleCombo.write(KEY_NUM_9);
      break;
    case 10:
      bleCombo.write(KEY_NUM_SLASH);
      break;
    case 11:
      bleCombo.write(KEY_NUM_ASTERISK);
      break;
    case 12:
      bleCombo.write(KEY_NUM_MINUS);
      break;
    case 13:
      bleCombo.write(KEY_NUM_PLUS);
      break;
    case 14:
      bleCombo.write(KEY_NUM_ENTER);
      break;
    case 15:
      bleCombo.write(KEY_NUM_PERIOD);
      break;
    }
    break;
  case 13: // Custom functions
    switch (value) {
    case 1:
      userAction1();
      break;
    case 2:
      userAction2();
      break;
    case 3:
      userAction3();
      break;
    case 4:
      userAction4();
      break;
    case 5:
      userAction5();
      break;
    case 6:
      userAction6();
      break;
    case 7:
      userAction7();
      break;
    }
    break;
  default:
    // If nothing matches do nothing
    break;
  }
}
