
int loadCurrentSettings(std::unique_ptr<char[]> &charBuffer, DynamicJsonBuffer &jsonBuffer){
  File file = SPIFFS.open("/current_settings.js", "r");
  if (!file) {
      Serial.println("Opening current_settings.js failed.");
      return -1;
  }
  size_t size = file.size();
  if (size > MAX_SETTINGS_FILE_SIZE) {
    Serial.println("current_settings is too large");
    file.close();
    return -1;
  }
  
  file.readBytes(charBuffer.get(), size);
  file.close();

  charBuffer.get()[size] = 0;
  JsonObject& json = jsonBuffer.parseObject(charBuffer.get());
  if (!json.success()) {
    Serial.println("Failed to parse current_settings.js file");
    return -1;
  }

  currentEffect = json["effect"];
  isOn = json["isOn"];
  
  return 0;
}

//=============================================================================================
bool saveCurrentSettings(){
  File file = SPIFFS.open("/current_settings.js", "w");
  if (!file) {
      Serial.println("Opening current_settings.js failed.");
      return false;
  }
  file.print("{\"effect\":");file.print(String(currentEffect));
  file.print(",\"isOn\":");file.print(String(isOn));
  file.print("}");
  file.close();
  return true;
}
