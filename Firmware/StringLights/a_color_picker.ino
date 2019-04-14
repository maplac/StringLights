
int loadColorPicker(std::unique_ptr<char[]> &charBuffer, DynamicJsonBuffer &jsonBuffer){
  File file = SPIFFS.open("/color_picker_settings.js", "r");
  if (!file) {
      Serial.println("Opening color_picker_settings.js failed.");
      return -1;
  }
  size_t size = file.size();
  if (size > MAX_SETTINGS_FILE_SIZE) {
    Serial.println("color_picker_settings is too large");
    file.close();
    return -1;
  }
  
  if(!file.seek(7)){
    Serial.println("seek failed");
    file.close();
    return -1;
  }
  size -= 8;
  
  

  // We don't use String here because ArduinoJson library requires the input
  // buffer to be mutable. If you don't use ArduinoJson, you may as well
  // use configFile.readString instead.
  file.readBytes(charBuffer.get(), size);
  file.close();
  
  charBuffer.get()[size] = 0;

  /*for(int i = 0; i < size; i++){
    Serial.write(buf.get()[i]);
  }
  Serial.println("");*/
  
  //char str[] = "{\"q\":[1,2,3],\"C\":[[11,22,33],[111,222,333]],\"names\":[\"jedna\",\"dva\",\"-\"]}";
  //StaticJsonBuffer<1000> jsonBuffer;
 

  JsonObject& json = jsonBuffer.parseObject(charBuffer.get());

  if (!json.success()) {
    Serial.println("Failed to parse color_picker_settings.js file");
    return -1;
  }

  for(int i = 0; i < SAVED_COLORS_COUNT; ++i){
    savedColors[i][0] = json["C"][i][0];
    savedColors[i][1] = json["C"][i][1];
    savedColors[i][2] = json["C"][i][2];
    String newName = json["names"][i];
    newName.toCharArray(savedNames[i], COLOR_NAME_LENGTH);
    //savedNames[i][newName.length()] = '\0';
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
  Serial.println("Handling ColorPicker");

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
              server.send(400,"text/html", "saving to file failed");
            }
            
          }else{
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
              server.send(400,"text/html", "saving to file failed");
            }
          }else{
            server.send(400,"text/html", "r/g/b/ is missing");
          }
        }else{
          server.send(400,"text/html", "id out of range");
        }
      }else{
         server.send(400,"text/html", "unknown type");
      }
    }else{
      server.send(400,"text/html", "id is missing");
    }
  }else{
    server.send(400,"text/html", "type is missing");
  }
  server.send(200,"text/html", "OK");
}
