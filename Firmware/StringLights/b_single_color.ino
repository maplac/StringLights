
int loadSingleColor(std::unique_ptr<char[]> &charBuffer, DynamicJsonBuffer &jsonBuffer){
  File file = SPIFFS.open("/single_color_settings.js", "r");
  if (!file) {
      Serial.println("Opening single_color_settings.js failed.");
      return -1;
  }
  size_t size = file.size();
  if (size > MAX_SETTINGS_FILE_SIZE) {
    Serial.println("single_color_settings is too large");
    file.close();
    return -1;
  }
  
  if(!file.seek(7)){
    Serial.println("seek failed");
    file.close();
    return -1;
  }
  size -= 8;
  
  file.readBytes(charBuffer.get(), size);
  file.close();
  
  charBuffer.get()[size] = 0;
  JsonObject& json = jsonBuffer.parseObject(charBuffer.get());
  if (!json.success()) {
    Serial.println("Failed to parse single_color_settings.js file");
    return -1;
  }
  singleColor[0] = json["sc"][0];
  singleColor[1] = json["sc"][1];
  singleColor[2] = json["sc"][2];

  return 0;
}

//=============================================================================================
void handleSingleColor(){
  Serial.println("Handling SingleColor");
  if(server.hasArg("type")){
    if(server.arg("type") == "color"){
      if(server.hasArg("r") && server.hasArg("g") && server.hasArg("b")){
      int r = server.arg("r").toInt();
      int g = server.arg("g").toInt();
      int b = server.arg("b").toInt();
      Serial.print("single color: ");  Serial.print(r);
      Serial.print(", ");  Serial.print(g);
      Serial.print(", ");  Serial.println(b);

      File file = SPIFFS.open("/single_color_settings.js", "w");
      if (!file) {
          Serial.println("Opening failed.");
          server.send(400,"text/html", "single_color_settings.js file open failed");
          return;
      }
      file.print("scstr='{\"sc\":[");
      file.print(server.arg("r"));file.print(",");
      file.print(server.arg("g"));file.print(",");
      file.print(server.arg("b"));file.print("]}'");
      file.close();

      currentEffect = EFFECT_SINGLE;
      
      singleColor[0] = (unsigned char) (r & 0xFF);
      singleColor[1] = (unsigned char) (g & 0xFF);
      singleColor[2] = (unsigned char) (b & 0xFF);
     
      for(int i = 0; i < ledCount; ++i){
        strip.SetPixelColor(i, RgbColor(r,g,b));
       }
      strip.Show();

      if(!saveCurrentSettings()){
        server.send(400,"text/html", "current_settings.js file open failed");
        return;
      }
      
      }else{
        server.send(400,"text/html", "r/g/b/ is missing");
      }
    }else{
      server.send(400,"text/html", "unknown type");  
    }
  }else{
    server.send(400,"text/html", "type is missing");
  }
  server.send(200,"text/html", "OK");
}
