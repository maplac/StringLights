int loadMultiColor(std::unique_ptr<char[]> &charBuffer, DynamicJsonBuffer &jsonBuffer){
  File file = SPIFFS.open("/multi_color_settings.js", "r");
  if (!file) {
      Serial.println("Opening multi_color_settings.js failed.");
      return -1;
  }
  size_t size = file.size();
  if (size > MAX_SETTINGS_FILE_SIZE) {
    Serial.println("multi_color_settings is too large");
    file.close();
    return -1;
  }
  
  if(!file.seek(11)){// mcbasestr='
    Serial.println("seek failed");
    file.close();
    return -1;
  }
  size -= 12;
  
  file.readBytes(charBuffer.get(), size);
  file.close();

  charBuffer.get()[size] = 0;
  JsonObject& json = jsonBuffer.parseObject(charBuffer.get());
  if (!json.success()) {
    Serial.println("Failed to parse multi_color_settings.js file");
    return -1;
  }

  multiColorIndex = json["mcindex"];
 
  for(int i = 0; i < json["mcnames"].size(); ++i){
    String newName = json["mcnames"][i];
    newName.toCharArray(multiColorNames[i], COLOR_NAME_LENGTH);
  }

  //---- load active settings ----
  
  String fileName;
  fileName = "/multi_color_settings_" + String(multiColorIndex) + ".js";
  file = SPIFFS.open(fileName, "r");
  if (!file) {
      Serial.println("Opening multi_color_settings_x.js failed.");
      return -1;
  }
  size = file.size();
  if (size > MAX_SETTINGS_FILE_SIZE) {
    Serial.println("multi_color_settings_x is too large");
    file.close();
    return -1;
  }
  /*
  if(!file.seek(11)){// mcbasestr='
    Serial.println("multi_color_settings_x seek failed");
    file.close();
    return;
  }
  size -= 12;*/
  
  file.readBytes(charBuffer.get(), size);
  file.close();

  charBuffer.get()[size] = 0;
  JsonObject& json2 = jsonBuffer.parseObject(charBuffer.get());
  if (!json2.success()) {
    Serial.println("Failed to parse multi_color_settings_x.js file");
    return -1;
  }
  
  multiColorLength = json2["mc"].size();

  for(int i = 0; i < multiColorLength; ++i){
    multiColor[i][0] = json2["mc"][i][0];
    multiColor[i][1] = json2["mc"][i][1];
    multiColor[i][2] = json2["mc"][i][2];
  }
  return 0;
}

//=============================================================================================
void handleMultiColor(){
  Serial.println("Handling MultiColor");
  if(server.hasArg("type") && server.hasArg("index")){
    int index = server.arg("index").toInt();
    if( index >= 0 || index < MULTI_COLOR_COUNT){
      Serial.print("index: ");Serial.println(index);
      if(server.arg("type") == "color"){
        
        if(server.hasArg("r") && server.hasArg("g") && server.hasArg("b")){
          String r = server.arg("r");
          String g = server.arg("g");
          String b = server.arg("b");
          if ( (r.length() % 3) == 0 && (g.length() % 3) == 0 && (b.length() % 3) == 0){
            if(r.length() == g.length() && g.length() == b.length()){
              if((r.length() / 3) <= ledCount || r.length() < 0){
                Serial.print("multi color: ");  Serial.print(r);
                Serial.print(", ");  Serial.print(g);
                Serial.print(", ");  Serial.println(b);
    
                //multiColorLength = r.length() / 3;
                //int count = 0;
                multiColorLength = 0;
                for(int i = 0; i < r.length(); i += 3){
                  multiColor[multiColorLength][0] = stringToNum(r.charAt(i), r.charAt(i + 1), r.charAt(i + 2));
                  multiColor[multiColorLength][1] = stringToNum(g.charAt(i), g.charAt(i + 1), g.charAt(i + 2));
                  multiColor[multiColorLength][2] = stringToNum(b.charAt(i), b.charAt(i + 1), b.charAt(i + 2));
                  multiColorLength++;
                }
                
                String fileName = "/multi_color_settings_" + String(index) + ".js";
                File file = SPIFFS.open(fileName, "w");
                if (!file) {
                    Serial.println("Opening failed.");
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
                  file.print("]}");
                  file.close();
                }

                // if index changed
                if(multiColorIndex != index){
                  multiColorIndex = index;
                  file = SPIFFS.open("/multi_color_settings.js", "w");
                  if (!file) {
                      Serial.println("Opening failed.");
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
                
                for(int i = 0; i < ledCount; ++i){
                  int index = i % multiColorLength;
                  strip.SetPixelColor(i, RgbColor(multiColor[index][0], multiColor[index][1], multiColor[index][2]));
                }
                strip.Show();
                
                if(!saveCurrentSettings()){
                  server.send(400,"text/html", "current_settings.js file open failed");
                  return;
                }
              }else{
                server.send(400,"text/html", "r/g/b/ length is out of range");
              }
            }else{
              server.send(400,"text/html", "r/g/b/ lengths do not match");
            }
          }else{
            server.send(400,"text/html", "r/g/b/ length is not divisible by 3");
          }
        }else{
          server.send(400,"text/html", "r/g/b/ is missing");
        }
      }else if(server.arg("type") == "rename"){
        String newName = server.arg("name");
        Serial.println("new name: " + newName);
        newName.toCharArray(multiColorNames[index], COLOR_NAME_LENGTH);
        File file = SPIFFS.open("/multi_color_settings.js", "w");
        if (!file) {
            Serial.println("Opening failed.");
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
        server.send(400,"text/html", "unknown type");
      }
    }else{
      server.send(400,"text/html", "index out of range");
    }
  }else{
    server.send(400,"text/html", "type is missing");
  }
  server.send(200,"text/html", "OK");
}


