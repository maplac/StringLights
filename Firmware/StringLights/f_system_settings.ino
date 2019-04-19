
int loadSystemSettings(std::unique_ptr<char[]> &charBuffer, DynamicJsonBuffer &jsonBuffer){
  File file = SPIFFS.open("/index_settings.js", "r");
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

  if(!file.seek(13)){
    Serial.println("index_settings.js seek failed");
    file.close();
    return -1;
  }
  size -= 14;
  
  file.readBytes(charBuffer.get(), size);
  file.close();

  charBuffer.get()[size] = 0;
  JsonObject& json = jsonBuffer.parseObject(charBuffer.get());
  if (!json.success()) {
    Serial.println("Failed to parse index_settings.js file");
    return -1;
  }

  String wifi_ssid = json["wifi_ssid"];
  wifi_ssid.toCharArray(ssid, MAX_WIFI_CHAR_LENGTH);
  ledCount = json["led_count"];

  for(int i = 0; i < 4; ++i){
    lastIpAddress[i] = json["last_ip_address"][i];
  }

  // file content is: {"wifi_passwd":"thePassword"}
  file = SPIFFS.open("/passwd.js", "r");
  if (!file) {
      Serial.println("Opening passwd.js failed.");
      return -1;
  }
  size = file.size();
  if (size > MAX_SETTINGS_FILE_SIZE) {
    Serial.println("passwd.js is too large");
    file.close();
    return -1;
  }
 
  file.readBytes(charBuffer.get(), size);
  file.close();

  charBuffer.get()[size] = 0;
  JsonObject& json2 = jsonBuffer.parseObject(charBuffer.get());
  if (!json2.success()) {
    Serial.println("Failed to parse passwd.js file");
    return -1;
  }

  String wifi_passwd = json2["wifi_passwd"];
  wifi_passwd.toCharArray(password, MAX_WIFI_CHAR_LENGTH);

  return 0;
}

//=============================================================================================
void handleIndex(){
  Serial.println("Handling Index");
  if(server.hasArg("type")){
    String type = server.arg("type");
    if(type == "cmd"){
      if(server.hasArg("cmd")){
        if(server.arg("cmd") == "on"){
          digitalWrite(led,1);
        }else if(server.arg("cmd") == "off"){
          digitalWrite(led,0);
        }else{
          server.send(400,"text/html", "unknown cmd");
          return;
        }
      }else{
        server.send(400,"text/html", "cmd is missing");
      }
    } else if (type == "settings"){
        if(server.hasArg("led_count") && server.hasArg("wifi_ssid")){
          ledCount = server.arg("led_count").toInt();
          if(ledCount > 1000){
            ledCount = 1000;
          }
          String wifi_ssid = server.arg("wifi_ssid");
          Serial.println("new wifi SSID: " + wifi_ssid);
          wifi_ssid.toCharArray(ssid, MAX_WIFI_CHAR_LENGTH);
          
          if (!saveSystemSettings) {
            server.send(400,"text/html", "new setting cannot be saved");
            return;
          }

          if(server.hasArg("wifi_passwd")){
            String wifi_passwd = server.arg("wifi_passwd");
            Serial.println("new wifi password: " + wifi_passwd);
            wifi_passwd.toCharArray(password, MAX_WIFI_CHAR_LENGTH);
            
            if (!savePassword()) {
              server.send(400,"text/html", "new setting cannot be saved");
              return;
            }
          }
          
        } else {
          server.send(400,"text/html", "led_count or wifi_ssid is missing");
          return;
        }
    } else {
      server.send(400,"text/html", "unknown type");
      return;
    }
  }else{
    server.send(400,"text/html", "type is missing");
    return;
  }
  server.send(200,"text/html", "OK");
}

//=============================================================================================
bool saveSystemSettings(){
  File file = SPIFFS.open("/index_settings.js", "w");
  if(!file){
    return false;
  }
  file.print("settingsstr='{\"led_count\":");file.print(ledCount);
  file.print(",\"wifi_ssid\":\"");file.print(String(ssid));
  file.print("\",\"last_ip_address\":[");
  file.print(String(lastIpAddress[0]));file.print(",");
  file.print(String(lastIpAddress[1]));file.print(",");
  file.print(String(lastIpAddress[2]));file.print(",");
  file.print(String(lastIpAddress[3]));file.print("]");
  file.print("}'");
  file.close();
  return true;
}

//=============================================================================================
bool savePassword(){
  File file = SPIFFS.open("/passwd.js", "w");
  if(!file){
    return false;
  }
  file.print("'{\"wifi_passwd\":\"");
  file.print(String(password));
  file.print("\"}'");
  file.close();
  return true;
}

