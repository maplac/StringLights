void handleButton1() {
  unsigned long timeNow = millis();

   if (button1.debouncing) {
    if ((timeNow - button1.lastDebounceTime) > debounceDelay) {
      button1.debouncing = false;
      int butState = digitalRead(gpioBut1);
      // button pressed
      if ( (button1.state == 1) && (butState == 0)) {
        //Serial.println("But1 down");
        button1.lastPressTime = timeNow;
        button1.longPressEnabled = true;
      }
      // button released
      if ( (button1.state == 0) && (butState == 1)) {
        if ((timeNow - button1.lastPressTime) <= longPressTime) {
          Serial.println("But1 short press");
          if (isOn) {
            isOn = 0;
          } else {
            isOn = 1;
          }
          saveCurrentSettings();
          applySettings();
        }
      }
      button1.state = butState;
    }
  }
  if (button1.state == 0) {
    if ((timeNow - button1.lastPressTime) > longPressTime && button1.longPressEnabled) {
      button1.longPressEnabled = false;
      Serial.println("But1 long press");
    }
  }
}

void handleButton2() {
  unsigned long timeNow = millis();
  if (button2.debouncing) {
    if ((timeNow - button2.lastDebounceTime) > debounceDelay) {
      button2.debouncing = false;
      int butState = digitalRead(gpioBut2);
      // button pressed
      if ( (button2.state == 1) && (butState == 0)) {
        //Serial.println("But2 down");
        button2.lastPressTime = timeNow;
        button2.longPressEnabled = true;
      }
      // button released
      if ( (button2.state == 0) && (butState == 1)) {
        if ((timeNow - button2.lastPressTime) <= longPressTime) {
          Serial.println("But2 short press");
          // do nothing if the LEDs are turned off
          if (isOn == 1) {
            if (currentEffect == EFFECT_MULTI) {
              // find the next nonblack slot
              for(int i = 0; i < MULTI_COLOR_COUNT; ++i) {
                // next index
                multiColorIndex = (multiColorIndex + 1) % MULTI_COLOR_COUNT;
                // load slot colors from file
                if (loadMultiColorSlot() < 0) {
                  multiColorIndex = (multiColorIndex + MULTI_COLOR_COUNT - 1) % MULTI_COLOR_COUNT;
                  Serial.println("loading multi color slot failed");
                  break;
                }
                // check if the slot is nonblack
                bool isBlack = true;
                for(int i = 0; i < multiColorLength; ++i) {
                  if ((multiColor[i][0] != 0) || (multiColor[i][1] != 0) || (multiColor[i][2] != 0)) {
                    isBlack = false;
                    break;
                  }
                }
                if (!isBlack) {
                  if(!saveMultiColor()) {
                    Serial.println("saving multi color settings failed");
                  }
                  break;
                }
              }
            } else {
              int newIndex = singleColorIndex;
              for(int i = 0; i < SINGLE_COLOR_COUNT; ++i){
                newIndex = (newIndex + 1) % SINGLE_COLOR_COUNT;
                if ((singleColor[newIndex][0] != 0) || (singleColor[newIndex][1] != 0) || (singleColor[newIndex][2] != 0)) {
                  break;
                }
              }
              singleColorIndex = newIndex;
              if(!saveSingleColor()) {
                Serial.println("saving single color failed");
              }
            }
            applySettings();
          }
        }
      }
      button2.state = butState;
    }
  }
  if (button2.state == 0) {
    if ((timeNow - button2.lastPressTime) > longPressTime && button2.longPressEnabled) {
      button2.longPressEnabled = false;
      Serial.println("But2 long press");
      // do nothing if the LEDs are turned off
      if (isOn == 1) {
        if (currentEffect == EFFECT_MULTI) {
          currentEffect = EFFECT_SINGLE;
        } else {
          currentEffect = EFFECT_MULTI;
        }
        saveCurrentSettings();
        applySettings();
      }
    }
  }
}
