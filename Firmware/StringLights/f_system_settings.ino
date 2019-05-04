
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
  wifi_ssid.toCharArray(wifiSettings.ssid, MAX_WIFI_CHAR_LENGTH);
  ledCount = json["led_count"];
  wifiSettings.staticActive = json["static_active"];

  for(int i = 0; i < 4; ++i){
    wifiSettings.lastIp[i] = json["last_ip_address"][i];
    wifiSettings.staticIp[i] = json["static_ip"][i];
    wifiSettings.subnet[i] = json["subnet"][i];
    wifiSettings.gateway[i] = json["gateway"][i];
    wifiSettings.dns[i] = json["dns"][i];
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
  wifi_passwd.toCharArray(wifiSettings.password, MAX_WIFI_CHAR_LENGTH);

  return 0;
}

//=============================================================================================
void handleIndex(){
  digitalWrite(gpioLedProcessing, 1);
  Serial.println("Handling Index");
  bool error = false;
  
  if(server.hasArg("type")){
    String type = server.arg("type");
    if(type == "cmd"){
      if(server.hasArg("cmd")){
        if(server.arg("cmd") == "on"){
          isOn = true;
          //digitalWrite(gpioLedHotSpot,1);
        }else if(server.arg("cmd") == "off"){
          //digitalWrite(gpioLedHotSpot,0);
          isOn = false;
        }else{
          error = true;
          server.send(400,"text/html", "unknown cmd");
        }
        // todo save settings
      }else{
        error = true;
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
        wifi_ssid.toCharArray(wifiSettings.ssid, MAX_WIFI_CHAR_LENGTH);
        
        if (!saveSystemSettings()) {
          error = true;
          server.send(400,"text/html", "new setting cannot be saved");
        }

        if(server.hasArg("wifi_passwd")){
          String wifi_passwd = server.arg("wifi_passwd");
          Serial.println("new wifi password: " + wifi_passwd);
          wifi_passwd.toCharArray(wifiSettings.password, MAX_WIFI_CHAR_LENGTH);
          
          if (!savePassword()) {
            error = true;
            server.send(400,"text/html", "new setting cannot be saved");
          }
        }
      } else {
        error = true;
        server.send(400,"text/html", "led_count or wifi_ssid is missing");
      }
    } else if (type == "settings_static"){
      if(server.hasArg("static_active") && server.hasArg("ip") && server.hasArg("subnet") && server.hasArg("gateway") && server.hasArg("dns")){
        String static_active = server.arg("static_active");
        String ip = server.arg("ip");
        String subnet = server.arg("subnet");
        String gateway = server.arg("gateway");
        String dns = server.arg("dns");
        if(ip.length() == 12 && subnet.length() == 12 && gateway.length() == 12 && dns.length() == 12){
          if(static_active == "true"){
            wifiSettings.staticActive = true;
          } else {
            wifiSettings.staticActive = false;
          }
          for(int i = 0; i < 4; i ++){
            wifiSettings.staticIp[i] = stringToNum(ip.charAt(i * 3), ip.charAt((i * 3) + 1), ip.charAt((i * 3) + 2));
            wifiSettings.subnet[i] = stringToNum(subnet.charAt(i * 3), subnet.charAt((i * 3) + 1), subnet.charAt((i * 3) + 2));
            wifiSettings.gateway[i] = stringToNum(gateway.charAt(i * 3), gateway.charAt((i * 3) + 1), gateway.charAt((i * 3) + 2));
            wifiSettings.dns[i] = stringToNum(dns.charAt(i * 3), dns.charAt((i * 3) + 1), dns.charAt((i * 3) + 2));
          }
          if (!saveSystemSettings()) {
            error = true;
            server.send(400,"text/html", "new setting cannot be saved");
          }
        } else {
          error = true;
          server.send(400,"text/html", "ip, subnet, gateway or DNS doesn't have length 9");
        }
      } else {
        error = true;
        server.send(400,"text/html", "static_active, ip, subnet, gateway or DNS is missing");
      }
    } else {
      error = true;
      server.send(400,"text/html", "unknown type");
    }
  }else{
    error = true;
    server.send(400,"text/html", "type is missing");
  }

  if (!error)
    server.send(200,"text/html", "OK");
    
  digitalWrite(gpioLedProcessing, 0);
}

//=============================================================================================
bool saveSystemSettings(){
  File file = SPIFFS.open("/index_settings.js", "w");
  if(!file){
    return false;
  }
  file.print("settingsstr='{\"led_count\":");file.print(ledCount);
  file.print(",\"wifi_ssid\":\"");file.print(String(wifiSettings.ssid));
  file.print("\",\"last_ip_address\":[");
  file.print(String(wifiSettings.lastIp[0]));file.print(",");
  file.print(String(wifiSettings.lastIp[1]));file.print(",");
  file.print(String(wifiSettings.lastIp[2]));file.print(",");
  file.print(String(wifiSettings.lastIp[3]));file.print("]");
  file.print(",\"static_active\":");
  if(wifiSettings.staticActive){
    file.print("true");
  } else {
    file.print("false");
  }
  file.print(",\"static_ip\":[");
  file.print(String(wifiSettings.staticIp[0]));file.print(",");
  file.print(String(wifiSettings.staticIp[1]));file.print(",");
  file.print(String(wifiSettings.staticIp[2]));file.print(",");
  file.print(String(wifiSettings.staticIp[3]));file.print("]");
  file.print(",\"subnet\":[");
  file.print(String(wifiSettings.subnet[0]));file.print(",");
  file.print(String(wifiSettings.subnet[1]));file.print(",");
  file.print(String(wifiSettings.subnet[2]));file.print(",");
  file.print(String(wifiSettings.subnet[3]));file.print("]");
  file.print(",\"gateway\":[");
  file.print(String(wifiSettings.gateway[0]));file.print(",");
  file.print(String(wifiSettings.gateway[1]));file.print(",");
  file.print(String(wifiSettings.gateway[2]));file.print(",");
  file.print(String(wifiSettings.gateway[3]));file.print("]");
  file.print(",\"dns\":[");
  file.print(String(wifiSettings.dns[0]));file.print(",");
  file.print(String(wifiSettings.dns[1]));file.print(",");
  file.print(String(wifiSettings.dns[2]));file.print(",");
  file.print(String(wifiSettings.dns[3]));file.print("]");
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
  file.print(String(wifiSettings.password));
  file.print("\"}'");
  file.close();
  return true;
}
