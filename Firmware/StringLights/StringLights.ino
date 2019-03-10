/*
 * ESP8266 SPIFFS HTML Web Page with JPEG, PNG Image 
 *
 */

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FS.h>   
#include <ESP8266mDNS.h>
#include <NeoPixelBus.h>
#include <ArduinoJson.h>

#define SAVED_COLORS_COUNT  30
#define COLOR_NAME_LENGTH   13 // 12 chars + \0
#define EFFECT_SINGLE       0
#define EFFECT_MULTI        1
#define MULTI_COLOR_COUNT   10

//ESP AP Mode configuration
const char *ssid = "Tenda";
const char *password = "unstuckunstuck";

const int led = 13;
const int ledCount = 20;

#define colorSaturation 128
NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod > strip(ledCount);
RgbColor red(colorSaturation, 0, 0);
RgbColor green(0, colorSaturation, 0);
RgbColor blue(0, 0, colorSaturation);
RgbColor white(colorSaturation);
RgbColor black(0);
ESP8266WebServer server(80);

unsigned char savedColors[SAVED_COLORS_COUNT][3] ;
char savedNames[SAVED_COLORS_COUNT][COLOR_NAME_LENGTH];
unsigned char singleColor[3];
unsigned char multiColor[SAVED_COLORS_COUNT][3];
int multiColorLength;
int multiColorCount = 10;
int multiColorIndex;
char multiColorNames[MULTI_COLOR_COUNT][COLOR_NAME_LENGTH];
int currentEffect;
int isOn;

//=============================================================================================
void loadSettings(){
  Serial.println("Loading settings");

  //---------------------------------------------------------------------
  File file = SPIFFS.open("/color_picker_settings.js", "r");
  if (!file) {
      Serial.println("Opening color_picker_settings.js failed.");
      return;
  }
  size_t size = file.size();
  if (size > 1024) {
    Serial.println("color_picker_settings is too large");
    file.close();
    return;
  }
  
  if(!file.seek(7)){
    Serial.println("seek failed");
    file.close();
    return;
  }
  size -= 8;
  
  // Allocate a buffer to store contents of the file.
  std::unique_ptr<char[]> buf(new char[size+1]);

  // We don't use String here because ArduinoJson library requires the input
  // buffer to be mutable. If you don't use ArduinoJson, you may as well
  // use configFile.readString instead.
  file.readBytes(buf.get(), size);
  file.close();
  
  buf.get()[size] = 0;

  /*for(int i = 0; i < size; i++){
    Serial.write(buf.get()[i]);
  }
  Serial.println("");*/
  
  //char str[] = "{\"q\":[1,2,3],\"C\":[[11,22,33],[111,222,333]],\"names\":[\"jedna\",\"dva\",\"-\"]}";
  //StaticJsonBuffer<1000> jsonBuffer;
  DynamicJsonBuffer jsonBuffer(1000);

  JsonObject& json = jsonBuffer.parseObject(buf.get());

  if (!json.success()) {
    Serial.println("Failed to parse color_picker_settings.js file");
    return;
  }

  for(int i = 0; i < SAVED_COLORS_COUNT; ++i){
    savedColors[i][0] = json["C"][i][0];
    savedColors[i][1] = json["C"][i][1];
    savedColors[i][2] = json["C"][i][2];
    String newName = json["names"][i];
    newName.toCharArray(savedNames[i], COLOR_NAME_LENGTH);
    //savedNames[i][newName.length()] = '\0';
  }

  //---------------------------------------------------------------------
  file = SPIFFS.open("/single_color_settings.js", "r");
  if (!file) {
      Serial.println("Opening single_color_settings.js failed.");
      return;
  }
  size = file.size();
  if (size > 1024) {
    Serial.println("single_color_settings is too large");
    file.close();
    return;
  }
  
  if(!file.seek(7)){
    Serial.println("seek failed");
    file.close();
    return;
  }
  size -= 8;
  
  //std::unique_ptr<char[]> buf(new char[size+1]);

  file.readBytes(buf.get(), size);
  file.close();
  
  buf.get()[size] = 0;
  JsonObject& json2 = jsonBuffer.parseObject(buf.get());
  if (!json2.success()) {
    Serial.println("Failed to parse single_color_settings.js file");
    return;
  }
  singleColor[0] = json2["sc"][0];
  singleColor[1] = json2["sc"][1];
  singleColor[2] = json2["sc"][2];

  //---------------------------------------------------------------------
  file = SPIFFS.open("/multi_color_settings.js", "r");
  if (!file) {
      Serial.println("Opening multi_color_settings.js failed.");
      return;
  }
  size = file.size();
  if (size > 1024) {
    Serial.println("multi_color_settings is too large");
    file.close();
    return;
  }
  
  if(!file.seek(11)){// mcbasestr='
    Serial.println("seek failed");
    file.close();
    return;
  }
  size -= 12;
  
  file.readBytes(buf.get(), size);
  file.close();

  buf.get()[size] = 0;
  JsonObject& json3 = jsonBuffer.parseObject(buf.get());
  if (!json3.success()) {
    Serial.println("Failed to parse multi_color_settings.js file");
    return;
  }

  multiColorIndex = json3["mcindex"];
 
  for(int i = 0; i < json3["mcnames"].size(); ++i){
    String newName = json3["mcnames"][i];
    newName.toCharArray(multiColorNames[i], COLOR_NAME_LENGTH);
  }

  //---------------------------------------------------------------------
  String fileName;
  fileName = "/multi_color_settings_" + String(multiColorIndex) + ".js";
  file = SPIFFS.open(fileName, "r");
  if (!file) {
      Serial.println("Opening multi_color_settings_x.js failed.");
      return;
  }
  size = file.size();
  if (size > 1024) {
    Serial.println("multi_color_settings_x is too large");
    file.close();
    return;
  }
  /*
  if(!file.seek(11)){// mcbasestr='
    Serial.println("multi_color_settings_x seek failed");
    file.close();
    return;
  }
  size -= 12;*/
  
  file.readBytes(buf.get(), size);
  file.close();

  buf.get()[size] = 0;
  JsonObject& json5 = jsonBuffer.parseObject(buf.get());
  if (!json5.success()) {
    Serial.println("Failed to parse multi_color_settings_x.js file");
    return;
  }
  
  multiColorLength = json5["mc"].size();

  for(int i = 0; i < multiColorLength; ++i){
    multiColor[i][0] = json5["mc"][i][0];
    multiColor[i][1] = json5["mc"][i][1];
    multiColor[i][2] = json5["mc"][i][2];
  }
  
  //---------------------------------------------------------------------
  file = SPIFFS.open("/current_settings.js", "r");
  if (!file) {
      Serial.println("Opening current_settings.js failed.");
      return;
  }
  size = file.size();
  if (size > 1024) {
    Serial.println("current_settings is too large");
    file.close();
    return;
  }
  
  file.readBytes(buf.get(), size);
  file.close();

  buf.get()[size] = 0;
  JsonObject& json4 = jsonBuffer.parseObject(buf.get());
  if (!json4.success()) {
    Serial.println("Failed to parse current_settings.js file");
    return;
  }

  currentEffect = json4["effect"];
  isOn = 1; // todo 
  //isOn = json4["isOn"];

  if(isOn){
    if(currentEffect == EFFECT_MULTI){
      for(int i = 0; i < ledCount; ++i){
        int index = i % multiColorLength;
        strip.SetPixelColor(i, RgbColor(multiColor[index][0], multiColor[index][1], multiColor[index][2]));
      }
      strip.Show();
    }else if(currentEffect == EFFECT_SINGLE){
      for(int i = 0; i < ledCount; ++i){
        strip.SetPixelColor(i, RgbColor(singleColor[0], singleColor[1], singleColor[2]));
      }
    }
  }else{
    for(int i = 0; i < ledCount; ++i){
      strip.SetPixelColor(i, RgbColor(0, 0, 0));
    }
  }
  strip.Show();
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
void handleRoot(){
  Serial.println("Handling root.");
  server.sendHeader("Location", "/index.html",true);   //Redirect to our html web page
  server.send(302, "text/plane","");
}

//=============================================================================================
void handleWebRequests(){
  //Serial.println("Handling WebRequests.");
  if(loadFromSpiffs(server.uri())) return;
  String message = "File Not Detected\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " NAME:"+server.argName(i) + "\n VALUE:" + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  Serial.println(message);
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
        }
      }else{
        server.send(400,"text/html", "cmd is missing");
      }
    }    
  }else{
    server.send(400,"text/html", "type is missing");
  }
  server.send(200,"text/html", "OK");
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
//=============================================================================================
inline unsigned char stringToNum(char a, char b, char c){
  return (a - 48) * 100 + (b - 48) * 10 + (c - 48);
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

//=============================================================================================
void setup() {
  delay(1000);
  Serial.begin(115200);
  Serial.println();

  pinMode(led, OUTPUT);
  digitalWrite(led, LOW);

  //Initialize File System
  SPIFFS.begin();
  Serial.println("File System Initialized");

  
  /*
  //Initialize AP Mode
  WiFi.softAP(ssid);  //Password not used
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("Web Server IP:");
  Serial.println(myIP);
  */
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.print("Connecting to ");
  Serial.println(ssid);
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

/*
  if (MDNS.begin("test")) {              // Start the mDNS responder for esp8266.local
    Serial.println("mDNS responder started");
  } else {
    Serial.println("Error setting up MDNS responder!");
  }
*/
  //Initialize Webserver
  server.on("/",handleRoot);
  server.on("/index", HTTP_POST, handleIndex);
  server.on("/color-picker", HTTP_POST, handleColorPicker);
  server.on("/single-color", HTTP_POST, handleSingleColor);
  server.on("/multi-color", HTTP_POST, handleMultiColor);
  //server.on("/color_picker_settings.js", HTTP_GET, handleColorPickerSettings);
  //server.on("/single_color_settings.js", HTTP_GET, handleSingleColorSettings);
  server.onNotFound(handleWebRequests);
  server.begin();
  Serial.println("HTTP server started");

  strip.Begin();
  
  loadSettings();
 
  /*
  for(int i = 0; i < ledCount; ++i){
    strip.SetPixelColor(i, RgbColor(singleColor[0],singleColor[1],singleColor[2]));
  }
  strip.Show();*/

  //MDNS.addService("http", "tcp", 80);

  Serial.println("Setup finished.");
}

//=============================================================================================
void loop() {
  //MDNS.update();
  server.handleClient();
}

//=============================================================================================
bool loadFromSpiffs(String path){
  String dataType = "text/plain";
  if(path.endsWith("/")) path += "index.htm";
  if(path.endsWith(".src")) path = path.substring(0, path.lastIndexOf("."));
  
  else if(path.endsWith(".html")) dataType = "text/html";
  else if(path.endsWith(".htm")) dataType = "text/html";
  else if(path.endsWith(".css")) dataType = "text/css";
  else if(path.endsWith(".js")) dataType = "application/javascript";
  else if(path.endsWith(".png")) dataType = "image/png";
  else if(path.endsWith(".gif")) dataType = "image/gif";
  else if(path.endsWith(".jpg")) dataType = "image/jpeg";
  else if(path.endsWith(".ico")) dataType = "image/x-icon";
  else if(path.endsWith(".xml")) dataType = "text/xml";
  else if(path.endsWith(".pdf")) dataType = "application/pdf";
  else if(path.endsWith(".zip")) dataType = "application/zip";

  Serial.print("Requested file: ");
  Serial.print(path);
  Serial.print(". ");
  
  File dataFile = SPIFFS.open(path.c_str(), "r");

  if (!dataFile) {
      Serial.println("Opening failed.");
  }else{
    Serial.println();
  }
  
  if (server.hasArg("download")) dataType = "application/octet-stream";
  if (server.streamFile(dataFile, dataType) != dataFile.size()) {
  }

  dataFile.close();
  return true;
}



 
