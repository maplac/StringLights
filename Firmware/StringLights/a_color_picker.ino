
int loadColorPicker(){
  if (readJson("/color_picker_settings.js", 7)) {
    Serial.print("Reading JSON from file \"/color_picker_settings.js\" failed. "); Serial.println(errorMessage);
    return -1;
  }

  for(int i = 0; i < SAVED_COLORS_COUNT; ++i){
    savedColors[i][0] = jsonDoc["C"][i][0];
    savedColors[i][1] = jsonDoc["C"][i][1];
    savedColors[i][2] = jsonDoc["C"][i][2];
    String newName = jsonDoc["names"][i];
    newName.toCharArray(savedNames[i], COLOR_NAME_LENGTH);
    savedNames[i][newName.length()] = '\0';
  }

  return 0;
}

//=============================================================================================
bool saveColorPickerSettings(){
  File file = SPIFFS.open("/color_picker_settings.js", "w");
  if (!file) {
      Serial.println("Opening color_picker_settings.js failed.");
      return false;
  }
  file.print("cpstr='{\"c\":[0,0,0],\"C\":[");
  for(int i = 0; i < SAVED_COLORS_COUNT; i++){
    file.print("[");
    file.print(String(savedColors[i][0]));file.print(",");
    file.print(String(savedColors[i][1]));file.print(",");
    file.print(String(savedColors[i][2]));file.print("]");
    if( i < (SAVED_COLORS_COUNT - 1)){
      file.print(",");
    }
  }
  file.print("],\"names\":[");
  for(int i = 0; i < SAVED_COLORS_COUNT; i++){
    file.print("\"");
    file.print(String(savedNames[i]));
    file.print("\"");
    if( i < (SAVED_COLORS_COUNT - 1)){
      file.print(",");
    }
  }
  file.print("]}'");  
  file.close();
  return true;
}

//=============================================================================================
void handleColorPicker(){
  setLedColor(2, BLUE);
  Serial.println("Handling ColorPicker");
  bool error = false;

  if(server.hasArg("type")){
    if(server.hasArg("id")){
      int id = server.arg("id").toInt();
      Serial.println("id: " + server.arg("id"));
      if(id < SAVED_COLORS_COUNT){
        if(server.arg("type") == "rename"){
          if(server.hasArg("name")){
            String newName = server.arg("name");
            Serial.println("new name: " + newName);
            newName.toCharArray(savedNames[id], COLOR_NAME_LENGTH);
            //savedNames[id][newName.length()] = '\0';
            if(!saveColorPickerSettings()){
              error = true;
              server.send(400,"text/html", "saving to file failed");
            }
            
          }else{
            error = true;
            server.send(400,"text/html", "name is missing");
          }
        }else if(server.arg("type") == "save"){
          if(server.hasArg("r") && server.hasArg("g") && server.hasArg("b")){
            int r = server.arg("r").toInt();
            int g = server.arg("g").toInt();
            int b = server.arg("b").toInt();
            Serial.print("new color: ");  Serial.print(r);
            Serial.print(", ");  Serial.print(g);
            Serial.print(", ");  Serial.println(b);
            
            savedColors[id][0] = (unsigned char) r;
            savedColors[id][1] = (unsigned char) g;
            savedColors[id][2] = (unsigned char) b;

            if(!saveColorPickerSettings()){
              error = true;
              server.send(400,"text/html", "saving to file failed");
            }
          }else{
            error = true;
            server.send(400,"text/html", "r/g/b/ is missing");
          }
        }else{
          error = true;
          server.send(400,"text/html", "id out of range");
        }
      }else{
        error = true;
         server.send(400,"text/html", "unknown type");
      }
    }else{
      error = true;
      server.send(400,"text/html", "id is missing");
    }
  }else{
    error = true;
    server.send(400,"text/html", "type is missing");
  }
  if (!error)
    server.send(200,"text/html", "OK");
    
  setLedColor(2, NONE);
}
