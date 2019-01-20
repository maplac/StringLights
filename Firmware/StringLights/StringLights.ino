/*
 * ESP8266 SPIFFS HTML Web Page with JPEG, PNG Image 
 *
 */

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FS.h>   //Include File System Headers
#include <ESP8266mDNS.h>
#include <NeoPixelBus.h>

const char* imagefile = "/lenna.png";
const char* htmlfile = "/index.html";

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

void handleLedGet(){
  Serial.println("Handling LED get.");

  if(server.hasArg("r") && server.hasArg("g") && server.hasArg("b")){
    int r = server.arg("r").toInt();
    int g = server.arg("g").toInt();
    int b = server.arg("b").toInt();
    Serial.print("r=");  Serial.print(r);
    Serial.print(", g=");  Serial.print(g);
    Serial.print(", b=");  Serial.println(b);
    for(int i = 0; i < ledCount; ++i){
      strip.SetPixelColor(i, RgbColor(r,g,b));
    }
    strip.Show();
  }
  
  
  digitalWrite(led,!digitalRead(led));      // Change the state of the LED
  //server.sendHeader("Location","/");        // Add a header to respond with a new location for the browser to go to the home page again
  //server.send(303);                         // Send it back to the browser with an HTTP status 303 (See Other) to redirect
  server.send(200,"text/html", "OK");
}

void handleIndex(){
  Serial.println("Handling Index");
  if(server.hasArg("type")){
    String type = server.arg("type");
    if(type == "cmd"){
      if(server.hasArg("cmd")){
        if(server.arg("cmd") == "on"){
          digitalWrite(led,1);
          server.send(200,"text/html", "OK");
        }else if(server.arg("cmd") == "off"){
          digitalWrite(led,0);
          server.send(200,"text/html", "OK");
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
      for(int i = 0; i < ledCount; ++i){
        strip.SetPixelColor(i, RgbColor(r,g,b));
       }
      strip.Show();
       // todo
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
      if(server.arg("type") == "rename"){
        if(server.hasArg("name")){
          String newName = server.arg("name");
          Serial.println("new name: " + newName);
          // todo
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
          // todo
        }else{
          server.send(400,"text/html", "r/g/b/ is missing");
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
  server.on("/LED", HTTP_GET, handleLedGet);
  server.on("/index", HTTP_POST, handleIndex);
  server.on("/single-color", HTTP_POST, handleSingleColor);
  server.on("/color-picker", HTTP_POST, handleColorPicker);// color picker
  server.onNotFound(handleWebRequests); //Set setver all paths are not found so we can handle as per URI
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
      Serial.print("Opening failed.");
  }else{
    Serial.println();
  }
  
  if (server.hasArg("download")) dataType = "application/octet-stream";
  if (server.streamFile(dataFile, dataType) != dataFile.size()) {
  }

  dataFile.close();
  return true;
}
