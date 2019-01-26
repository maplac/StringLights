/*
 * ESP8266 SPIFFS HTML Web Page with JPEG, PNG Image 
 *
 */

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FS.h>   //Include File System Headers
#include <ESP8266mDNS.h>
#include <NeoPixelBus.h>
#include <ArduinoJson.h>

#define SAVED_COLORS_COUNT  30
#define COLOR_NAME_LENGTH   12

//ESP AP Mode configuration
const char *ssid = "Tenda";
const char *password = "unstuckunstuck";

const int led = 13;
const int ledCount = 4;

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

void loadSettings(){
  Serial.println("Loading settings");

  /*savedColors[0][0] = 10;
  savedColors[0][1] = 20;
  savedColors[0][2] = 30;
  savedColors[1][0] = 110;
  savedColors[1][1] = 120;
  savedColors[1][2] = 130;
  for(int i=0; i<SAVED_COLORS_COUNT;i++){
    strcpy(savedNames[i], "-");  
  }
  strcpy(savedNames[0], "zero");
  strcpy(savedNames[1], "one");
  
  saveColorPickerSettings();*/
    
  File file = SPIFFS.open("/color_picker_settings.js", "r");
  if (!file) {
      Serial.println("Opening color_picker_settings.js failed.");
      return;
  }
  size_t size = file.size();
  if (size > 1024) {
    Serial.println("color_picker_settings is too large");
    return;
  }
  
  if(!file.seek(7)){
    Serial.println("seek failed");
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
  /*
  const char* name = json["names"][0];
  unsigned char r = json["C"][0][0];
  unsigned char g = json["C"][0][1];
  unsigned char b = json["C"][0][2];
  Serial.print("name: "); Serial.println(name);
  Serial.print(r);Serial.print(", ");
  Serial.print(g);Serial.print(", ");
  Serial.println(b);
 */
}

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

void handleRoot(){
  Serial.println("Handling root.");
  server.sendHeader("Location", "/index.html",true);   //Redirect to our html web page
  server.send(302, "text/plane","");
}

void handleWebRequests(){
  Serial.println("Handling WebRequests.");
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
          server.send(400,"text/html", "file open failed");
          return;
      }
      file.print("scstr='{\"sc\":[");
      file.print(server.arg("r"));file.print(",");
      file.print(server.arg("g"));file.print(",");
      file.print(server.arg("b"));file.print("]}'");
      file.close();

      File f = SPIFFS.open("/single_color_settings.js", "r");
      if (!f) {
          Serial.println("Opening for read failed.");
          return;
      }
     
      for(int i = 0; i < ledCount; ++i){
        strip.SetPixelColor(i, RgbColor(r,g,b));
       }
      strip.Show();
      
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

void setup() {
  delay(1000);
  Serial.begin(115200);
  Serial.println();

  pinMode(led, OUTPUT);
  digitalWrite(led, LOW);

  //Initialize File System
  SPIFFS.begin();
  Serial.println("File System Initialized");

  loadSettings();
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
  server.on("/single-color", HTTP_POST, handleSingleColor);
  server.on("/color-picker", HTTP_POST, handleColorPicker);
  //server.on("/color_picker_settings.js", HTTP_GET, handleColorPickerSettings);
  //server.on("/single_color_settings.js", HTTP_GET, handleSingleColorSettings);
  server.onNotFound(handleWebRequests);
  server.begin();
  Serial.println("HTTP server started");

  strip.Begin();
  strip.SetPixelColor(0, red);
  strip.SetPixelColor(1, green);
  strip.SetPixelColor(2, blue);
  strip.SetPixelColor(3, white);
  strip.Show();

  //MDNS.addService("http", "tcp", 80);
}

void loop() {
  //MDNS.update();
  server.handleClient();
}

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

/*
ulozeni, zmena jmena, nacteni/zmena (index), nacteni/zmena (single-color)
POST 


*/



 
