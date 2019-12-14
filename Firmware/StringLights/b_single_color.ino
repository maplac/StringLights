
int loadSingleColor(){
  if (readJson("/single_color_settings.js", 7)) {
    Serial.print("Reading JSON from file \"/single_color_settings.js\" failed. "); Serial.println(errorMessage);
    return -1;
  }

  singleColorIndex = jsonDoc["index"];
  int colorLength = jsonDoc["sc"].size();

  if (colorLength > SINGLE_COLOR_COUNT) {
    Serial.println("Single color length reduced to ");Serial.println(SINGLE_COLOR_COUNT);
    colorLength = SINGLE_COLOR_COUNT;
  }

  if (singleColorIndex > SINGLE_COLOR_COUNT) {
    Serial.println("Single color index is out of range: ");Serial.println(singleColorIndex);
    singleColorIndex = 0;
  }

  for(int i = 0; i < colorLength; ++i){
    singleColor[i][0] = jsonDoc["sc"][i][0];
    singleColor[i][1] = jsonDoc["sc"][i][1];
    singleColor[i][2] = jsonDoc["sc"][i][2];
  }
  return 0;
}

bool saveSingleColor() {
  File file = SPIFFS.open("/single_color_settings.js", "w");
  if (file) {
    file.print("scstr='{\"sc\":[");
    for(int i = 0; i < SINGLE_COLOR_COUNT; ++i){
      if(i > 0){
        file.print(",");
      }
      file.print("[");
      file.print(singleColor[i][0]);file.print(",");
      file.print(singleColor[i][1]);file.print(",");
      file.print(singleColor[i][2]);file.print("]");
    }
    file.print("],\"index\":");file.print(singleColorIndex);
    file.print("}'");
    file.close();
  
    return true;
  } else {
    return false;
  }
}

//=============================================================================================
void handleSingleColor(){
  setLedColor(2, BLUE);
  Serial.println("Handling SingleColor");
  bool error = false;
  
  if(server.hasArg("type")){
    if(server.arg("type") == "color"){
      if(server.hasArg("index")){
        int index = server.arg("index").toInt();
        if(index >= 0 && index < SINGLE_COLOR_COUNT) {
          if(server.hasArg("r") && server.hasArg("g") && server.hasArg("b")){
            int r = server.arg("r").toInt();
            int g = server.arg("g").toInt();
            int b = server.arg("b").toInt();
            
            Serial.print("single color: [");Serial.print(r);
            Serial.print(", "); Serial.print(g);
            Serial.print(", "); Serial.print(b);
            Serial.print("], index = "); Serial.println(index);

            singleColorIndex = index;
            singleColor[singleColorIndex][0] = (unsigned char) (r & 0xFF);
            singleColor[singleColorIndex][1] = (unsigned char) (g & 0xFF);
            singleColor[singleColorIndex][2] = (unsigned char) (b & 0xFF);

            // save single color settings
            bool saveSettingsFailed = false;
            if(!saveSingleColor()) {
              saveSettingsFailed = true;
            }

            // save current settings if required
            bool saveCurrentSettingsRequired = false;
            if (currentEffect != EFFECT_SINGLE) {
              currentEffect = EFFECT_SINGLE;
              saveCurrentSettingsRequired = true;
            }
            if (isOn != 1) {
              isOn = 1;
              saveCurrentSettingsRequired = true;
            }
            if (saveCurrentSettingsRequired) {
              if(!saveCurrentSettings()){
                saveSettingsFailed = true;
              }
            }

            // if the saving failed
            if (saveSettingsFailed) {
              error = true;
              server.send(400,"text/html", "saving single_color_settings.js or current_settings.js failed");
            }
          }else{
            error = true;
            server.send(400,"text/html", "r/g/b/ is missing");
          }
        } else {
          error = true;
          server.send(400,"text/html", "index out of range");
        }
      } else {
        error = true;
        server.send(400,"text/html", "index is missing");
      }
    }else{
      error = true;
      server.send(400,"text/html", "unknown type");  
    }
  }else{
    error = true;
    server.send(400,"text/html", "type is missing");
  }

  if (!error) {
    server.send(200,"text/html", "OK");
    for(int i = 0; i < ledCount; ++i){
      strip->SetPixelColor(i, RgbColor(singleColor[singleColorIndex][0], singleColor[singleColorIndex][1], singleColor[singleColorIndex][2]));
    }
    strip->Show();
  }
  
  setLedColor(2, NONE);
}
