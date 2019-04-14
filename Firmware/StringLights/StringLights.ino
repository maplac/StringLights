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
#define MAX_SETTINGS_FILE_SIZE  2048

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

int loadSingleColor(std::unique_ptr<char[]> &charBuffer, DynamicJsonBuffer &jsonBuffer);
int loadColorPicker(std::unique_ptr<char[]> &charBuffer, DynamicJsonBuffer &jsonBuffer);
int loadMultiColor(std::unique_ptr<char[]> &charBuffer, DynamicJsonBuffer &jsonBuffer);
int loadCurrentSettings(std::unique_ptr<char[]> &charBuffer, DynamicJsonBuffer &jsonBuffer);

bool saveColorPickerSettings();
bool saveCurrentSettings();

bool loadFromSpiffs(String path);
void handleWebRequests();
void handleRoot();
void handleIndex();
void handleColorPicker();
void handleSingleColor();
void handleMultiColor();

//=============================================================================================
//=============================================================================================
//=============================================================================================

inline unsigned char stringToNum(char a, char b, char c){
  return (a - 48) * 100 + (b - 48) * 10 + (c - 48);
}
//=============================================================================================
void loadSettings(){
  Serial.println("Loading settings");

  // Allocate a buffer to store contents of the files
  std::unique_ptr<char[]> buf(new char[MAX_SETTINGS_FILE_SIZE]);
  DynamicJsonBuffer jsonBuffer(1000);
  
  loadColorPicker(buf, jsonBuffer);
  loadSingleColor(buf, jsonBuffer);
  loadMultiColor(buf, jsonBuffer);
  loadCurrentSettings(buf, jsonBuffer);
  
  isOn = 1; // todo 
  
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




 
