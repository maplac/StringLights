
int loadSystemSettings(){
  if (readJson("/index_settings.js", 13)) {
    Serial.print("Reading JSON from file \"/index_settings.js\" failed. "); Serial.println(errorMessage);
    return -1;
  }

  String wifi_ssid = jsonDoc["wifi_ssid"];
  wifi_ssid.toCharArray(wifiSettings.ssid, MAX_WIFI_CHAR_LENGTH);
  ledCount = jsonDoc["led_count"];
  wifiSettings.staticActive = jsonDoc["static_active"];

  for(int i = 0; i < 4; ++i){
    wifiSettings.currentIp[i] = jsonDoc["current_ip_address"][i];
    wifiSettings.lastIp[i] = jsonDoc["last_ip_address"][i];
    wifiSettings.staticIp[i] = jsonDoc["static_ip"][i];
    wifiSettings.subnet[i] = jsonDoc["subnet"][i];
    wifiSettings.gateway[i] = jsonDoc["gateway"][i];
    wifiSettings.dns[i] = jsonDoc["dns"][i];
  }

  // file content is: {"wifi_passwd":"thePassword"}
   if (readJson("/passwd.js", 0)) {
    Serial.print("Reading JSON from file \"/passwd.js\" failed. "); Serial.println(errorMessage);
    return -1;
  }

  String wifi_passwd = jsonDoc["wifi_passwd"];
  wifi_passwd.toCharArray(wifiSettings.password, MAX_WIFI_CHAR_LENGTH);

  return 0;
}

//=============================================================================================
void handleIndex(){
  setLedColor(2, BLUE);
  Serial.println("Handling Index");
  bool error = false;
  
  if(server.hasArg("type")){
    String type = server.arg("type");
    if(type == "cmd"){
      if(server.hasArg("cmd")){
        if(server.arg("cmd") == "on"){
          isOn = 1;
        }else if(server.arg("cmd") == "off"){
          isOn = 0;
        }else{
          error = true;
          server.send(400,"text/html", "unknown cmd");
        }
        if (!error) {
          if (saveCurrentSettings()) {
            applySettings();
          } else {
            error = true;
            server.send(400,"text/html", "setting cannot be saved");
          }
        }
      }else{
        error = true;
        server.send(400,"text/html", "cmd is missing");
      }
    } else if (type == "settings"){
      if(server.hasArg("led_count") && server.hasArg("wifi_ssid")){
        ledCount = server.arg("led_count").toInt();
        if(ledCount > MAX_LED_COUNT){
          ledCount = MAX_LED_COUNT;
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
    
  setLedColor(2, NONE);
}

//=============================================================================================
bool saveSystemSettings(){
  File file = SPIFFS.open("/index_settings.js", "w");
  if(!file){
    return false;
  }
  file.print("settingsstr='{\"led_count\":");file.print(ledCount);
  file.print(",\"wifi_ssid\":\"");file.print(String(wifiSettings.ssid));
  file.print("\",\"current_ip_address\":[");
  file.print(String(wifiSettings.currentIp[0]));file.print(",");
  file.print(String(wifiSettings.currentIp[1]));file.print(",");
  file.print(String(wifiSettings.currentIp[2]));file.print(",");
  file.print(String(wifiSettings.currentIp[3]));file.print("]");
  file.print(",\"last_ip_address\":[");
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
  file.print("{\"wifi_passwd\":\"");
  file.print(String(wifiSettings.password));
  file.print("\"}");
  file.close();
  return true;
}
