
int loadSingleColor(){
  if (readJson("/single_color_settings.js", 7)) {
    Serial.print("Reading JSON from file \"/single_color_settings.js\" failed. "); Serial.println(errorMessage);
    return -1;
  }
  
  singleColor[0] = jsonDoc["sc"][0];
  singleColor[1] = jsonDoc["sc"][1];
  singleColor[2] = jsonDoc["sc"][2];

  return 0;
}

//=============================================================================================
void handleSingleColor(){
  setLedColor(2, BLUE);
  Serial.println("Handling SingleColor");
  bool error = false;
  
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
      isOn = 1;
      
      singleColor[0] = (unsigned char) (r & 0xFF);
      singleColor[1] = (unsigned char) (g & 0xFF);
      singleColor[2] = (unsigned char) (b & 0xFF);
     
      for(int i = 0; i < ledCount; ++i){
        strip->SetPixelColor(i, RgbColor(singleColor[0], singleColor[1], singleColor[2]));
      }

      if(!saveCurrentSettings()){
        error = true;
        server.send(400,"text/html", "current_settings.js file open failed");
      }
      
      }else{
        error = true;
        server.send(400,"text/html", "r/g/b/ is missing");
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
    strip->Show();
  }
  
  setLedColor(2, NONE);
}
