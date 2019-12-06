int loadMultiColor(){
  if (readJson("/multi_color_settings.js", 11)) {
    Serial.print("Reading JSON from file \"/multi_color_settings.js\" failed. "); Serial.println(errorMessage);
    return -1;
  }

  multiColorIndex = jsonDoc["mcindex"];
  for(int i = 0; i < jsonDoc["mcnames"].size(); ++i){
    String newName = jsonDoc["mcnames"][i];
    newName.toCharArray(multiColorNames[i], COLOR_NAME_LENGTH);
  }

  //---- load active settings ----
  
  String fileName;
  fileName = "/multi_color_settings_" + String(multiColorIndex) + ".js";
  char nameArr[32];
  fileName.toCharArray(nameArr, fileName.length() + 1);
  //Serial.println(nameArr);
  if (readJson(nameArr, 0)) {
    Serial.print("Reading JSON from file \"/multi_color_settings_#.js\" failed. "); Serial.println(errorMessage);
    return -1;
  }
  
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
  return 0;
}

//=============================================================================================
void handleMultiColor(){
  setLedColor(2, BLUE);
  Serial.println("Handling MultiColor");
  bool error = false;
  
  if(server.hasArg("type") && server.hasArg("index")){
    int index = server.arg("index").toInt();
    if( index >= 0 || index < MULTI_COLOR_COUNT){
      Serial.print("index: ");Serial.println(index);
      if(server.arg("type") == "color"){
        if(server.hasArg("assignment")) {
          if(server.hasArg("r") && server.hasArg("g") && server.hasArg("b")){
            String r = server.arg("r");
            String g = server.arg("g");
            String b = server.arg("b");
            int assignment = server.arg("assignment").toInt();
            if ( (r.length() % 3) == 0 && (g.length() % 3) == 0 && (b.length() % 3) == 0){
              if(r.length() == g.length() && g.length() == b.length()){
                if((r.length() / 3) <= ledCount && r.length() > 0){
                  Serial.print("multi color: ");  Serial.print(r);
                  Serial.print(", ");  Serial.print(g);
                  Serial.print(", ");  Serial.println(b);
                  Serial.print("multi assignment: "); Serial.println(assignment);
      
                  //multiColorLength = r.length() / 3;
                  //int count = 0;
                  multiColorLength = 0;
                  for(int i = 0; i < r.length(); i += 3){
                    multiColor[multiColorLength][0] = stringToNum(r.charAt(i), r.charAt(i + 1), r.charAt(i + 2));
                    multiColor[multiColorLength][1] = stringToNum(g.charAt(i), g.charAt(i + 1), g.charAt(i + 2));
                    multiColor[multiColorLength][2] = stringToNum(b.charAt(i), b.charAt(i + 1), b.charAt(i + 2));
                    multiColorLength++;
                  }
                  multiColorAssignment = assignment;
                  
                  String fileName = "/multi_color_settings_" + String(index) + ".js";
                  File file = SPIFFS.open(fileName, "w");
                  if (!file) {
                      error = true;
                      server.send(400,"text/html", "multi_color_settings_x.js file open failed");
                  }else{
                    file.print("{\"mc\":[");
                    for(int i = 0; i < multiColorLength; ++i){
                      if(i > 0){
                        file.print(",");
                      }
                      file.print("[");
                      file.print(multiColor[i][0]);file.print(",");
                      file.print(multiColor[i][1]);file.print(",");
                      file.print(multiColor[i][2]);file.print("]");
                    }
                    file.print("],\"mcassign\":");
                    file.print(multiColorAssignment);
                    file.print("}");
                    file.close();
                  }
  
                  // if index changed
                  if(multiColorIndex != index){
                    multiColorIndex = index;
                    file = SPIFFS.open("/multi_color_settings.js", "w");
                    if (!file) {
                        error = true;
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
                  currentEffect = EFFECT_MULTI;
                  isOn = 1;
                  if (multiColorAssignment == MULTICOLOR_ASSIGNMENT_SPREAD) {
                    int denominator = ceil(ledCount/multiColorLength);
                    for(int i = 0; i < ledCount; ++i){
                      int index = floor(i/denominator);
                      if (index >= multiColorLength) // should not happen
                        index = multiColorLength - 1;
                      strip->SetPixelColor(i, RgbColor(multiColor[index][0], multiColor[index][1], multiColor[index][2]));
                    }
                  } else {
                    for(int i = 0; i < ledCount; ++i){
                      int index = i % multiColorLength;
                      strip->SetPixelColor(i, RgbColor(multiColor[index][0], multiColor[index][1], multiColor[index][2]));
                    }
                  }
                  
                  if(!saveCurrentSettings()){
                    error = true;
                    server.send(400,"text/html", "current_settings.js file open failed");
                  }
                }else{
                  error = true;
                  server.send(400,"text/html", "r/g/b/ length is out of range");
                }
              }else{
                error = true;
                server.send(400,"text/html", "r/g/b/ lengths do not match");
              }
            }else{
              error = true;
              server.send(400,"text/html", "r/g/b/ length is not divisible by 3");
            }
          }else{
            error = true;
            server.send(400,"text/html", "r/g/b/ is missing");
          }
        } else {
          error = true;
          server.send(400,"text/html", "assignment is missing");
        }
      } else if(server.arg("type") == "rename"){
        String newName = server.arg("name");
        Serial.println("new name: " + newName);
        newName.toCharArray(multiColorNames[index], COLOR_NAME_LENGTH);
        File file = SPIFFS.open("/multi_color_settings.js", "w");
        if (!file) {
            error = true;
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
      }else{
        error = true;
        server.send(400,"text/html", "unknown type");
      }
    }else{
      error = true;
      server.send(400,"text/html", "index out of range");
    }
  }else{
    error = true;
    server.send(400,"text/html", "type is missing");
  }
  
  if (!error) {
    server.send(200,"text/html", "OK");
    strip->Show();
  }
    
  setLedColor(2, NONE);
}
