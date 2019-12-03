
int loadCurrentSettings(){
  if (readJson("/current_settings.js", 0)) {
    Serial.print("Reading JSON from file \"/current_settings.js\" failed. "); Serial.println(errorMessage);
    return -1;
  }

  currentEffect = jsonDoc["effect"];
  isOn = jsonDoc["isOn"];
  
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
