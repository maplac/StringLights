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
          if (currentEffect == EFFECT_MULTI) {
            multiColorIndex = (multiColorIndex + 1) % MULTI_COLOR_COUNT;
            
            String fileName;
            fileName = "/multi_color_settings_" + String(multiColorIndex) + ".js";
            char nameArr[32];
            fileName.toCharArray(nameArr, fileName.length() + 1);
            //Serial.println(nameArr);
            if (readJson(nameArr, 0)) {
              Serial.print("Reading JSON from file \"/multi_color_settings_#.js\" failed. "); Serial.println(errorMessage);
            } else {
              multiColorLength = jsonDoc["mc"].size();
              if (multiColorLength > MAX_LED_COUNT) {
                Serial.println("Multicolor length reduced to ");Serial.println(MAX_LED_COUNT);
                multiColorLength = MAX_LED_COUNT;
              }
              multiColorAssignment = jsonDoc["mcassign"];
              for(int i = 0; i < multiColorLength; ++i){
                multiColor[i][0] = jsonDoc["mc"][i][0];
                multiColor[i][1] = jsonDoc["mc"][i][1];
                multiColor[i][2] = jsonDoc["mc"][i][2];
              }
              File file = SPIFFS.open("/multi_color_settings.js", "w");
              if (!file) {
                  server.send(400,"text/html", "multi_color_settings.js file open failed");
              }else{
                file.print("mcbasestr='{\"mcindex\":");
                file.print(multiColorIndex);
                file.print(",\"mccount\":");
                file.print(MULTI_COLOR_COUNT);
                file.print(",\"mcnames\":[");
                for(int i = 0; i < MULTI_COLOR_COUNT; ++i){
                  if(i > 0){
                    file.print(",");
                  }
                  file.print("\"");
                  file.print(String(multiColorNames[i]));
                  file.print("\"");
                }
                file.print("]}'");
                file.close();
              }
            }
          } else {
            // TODO change single color
          }
          applySettings();
        }
      }
      button2.state = butState;
    }
  }
  if (button2.state == 0) {
    if ((timeNow - button2.lastPressTime) > longPressTime && button2.longPressEnabled) {
      button2.longPressEnabled = false;
      Serial.println("But2 long press");
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
