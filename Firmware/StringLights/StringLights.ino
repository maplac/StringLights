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
#define MAX_WIFI_CHAR_LENGTH    31

struct wifi_struct{
  char ssid[MAX_WIFI_CHAR_LENGTH];
  char password[MAX_WIFI_CHAR_LENGTH];
  unsigned char lastIp[4];
  bool staticActive;
  unsigned char staticIp[4];
  unsigned char subnet[4];
  unsigned char gateway[4];
  unsigned char dns[4];
} wifiSettings;

const int led = 13;
int ledCount = 1;

#define colorSaturation 128
NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod > *strip;
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
int loadSystemSettings(std::unique_ptr<char[]> &charBuffer, DynamicJsonBuffer &jsonBuffer);

bool saveColorPickerSettings();
bool saveCurrentSettings();
bool saveSystemSettings();
bool savePassword();

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
  loadSystemSettings(buf, jsonBuffer);
}

void applySettings(){
   
  if(isOn){
    if(currentEffect == EFFECT_MULTI){
      for(int i = 0; i < ledCount; ++i){
        int index = i % multiColorLength;
        strip->SetPixelColor(i, RgbColor(multiColor[index][0], multiColor[index][1], multiColor[index][2]));
      }
      strip->Show();
    }else if(currentEffect == EFFECT_SINGLE){
      for(int i = 0; i < ledCount; ++i){
        strip->SetPixelColor(i, RgbColor(singleColor[0], singleColor[1], singleColor[2]));
      }
    }
  }else{
    for(int i = 0; i < ledCount; ++i){
      strip->SetPixelColor(i, RgbColor(0, 0, 0));
    }
  }
  strip->Show();
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

  loadSettings();
  isOn = 1; // todo

  Serial.print("Wifi SSID: ");
  Serial.println(wifiSettings.ssid);
  Serial.print("Led count: ");
  Serial.println(ledCount);
  Serial.print("Last IP: ");
  Serial.print(wifiSettings.lastIp[0]);Serial.print(".");
  Serial.print(wifiSettings.lastIp[1]);Serial.print(".");
  Serial.print(wifiSettings.lastIp[2]);Serial.print(".");
  Serial.println(wifiSettings.lastIp[3]);;

  strip = new NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod >(ledCount);
  strip->Begin();

  applySettings();
  
  /*
  //Initialize AP Mode
  WiFi.softAP(ssid);  //Password not used
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("Web Server IP:");
  Serial.println(myIP);
  */
  
  /*
  wifiSettings.staticActive = true; // todo smazat
  String a("unstuckunstuck");
  a.toCharArray(wifiSettings.password, MAX_WIFI_CHAR_LENGTH);
  String b("Tenda");
  b.toCharArray(wifiSettings.ssid, MAX_WIFI_CHAR_LENGTH);
  */
  WiFi.disconnect();
  
  Serial.print("Connecting to ");
  Serial.println(wifiSettings.ssid);
  if(wifiSettings.staticActive){
    Serial.println("Using static IP address");
    Serial.print("IP: ");
    Serial.print(wifiSettings.staticIp[0]);Serial.print(".");
    Serial.print(wifiSettings.staticIp[1]);Serial.print(".");
    Serial.print(wifiSettings.staticIp[2]);Serial.print(".");
    Serial.println(wifiSettings.staticIp[3]);
    Serial.print("Subnet mask: ");
    Serial.print(wifiSettings.subnet[0]);Serial.print(".");
    Serial.print(wifiSettings.subnet[1]);Serial.print(".");
    Serial.print(wifiSettings.subnet[2]);Serial.print(".");
    Serial.println(wifiSettings.subnet[3]);
    Serial.print("Gateway: ");
    Serial.print(wifiSettings.gateway[0]);Serial.print(".");
    Serial.print(wifiSettings.gateway[1]);Serial.print(".");
    Serial.print(wifiSettings.gateway[2]);Serial.print(".");
    Serial.println(wifiSettings.gateway[3]);
    Serial.print("DNS: ");
    Serial.print(wifiSettings.dns[0]);Serial.print(".");
    Serial.print(wifiSettings.dns[1]);Serial.print(".");
    Serial.print(wifiSettings.dns[2]);Serial.print(".");
    Serial.println(wifiSettings.dns[3]);
    /*IPAddress ip(10,0,0,220);   
    IPAddress gateway(10,0,0,138);   
    IPAddress subnet(255,255,255,0);
    IPAddress dns(8,8,8,8);*/
    IPAddress ip(wifiSettings.staticIp[0], wifiSettings.staticIp[1], wifiSettings.staticIp[2], wifiSettings.staticIp[3]);
    IPAddress gateway(wifiSettings.gateway[0], wifiSettings.gateway[1], wifiSettings.gateway[2], wifiSettings.gateway[3]);
    IPAddress subnet(wifiSettings.subnet[0], wifiSettings.subnet[1], wifiSettings.subnet[2], wifiSettings.subnet[3]);
    IPAddress dns(wifiSettings.dns[0], wifiSettings.dns[1], wifiSettings.dns[2], wifiSettings.dns[3]);
    WiFi.config(ip, subnet, gateway, dns);
  } else {
    Serial.println("Using DHCP");
  }
  
  WiFi.hostname("StringLights");
  
  WiFi.begin(wifiSettings.ssid, wifiSettings.password);
  WiFi.mode(WIFI_STA);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  IPAddress ipAddress = WiFi.localIP();
  
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(ipAddress);
    
  IPAddress ipAddressLast(wifiSettings.lastIp[0], wifiSettings.lastIp[1], wifiSettings.lastIp[2], wifiSettings.lastIp[3]);
  if(ipAddress != ipAddressLast){
    Serial.println("IP address changed since last time.");
     Serial.print("Previous IP: ");
    Serial.print(wifiSettings.lastIp[0]);Serial.print(".");
    Serial.print(wifiSettings.lastIp[1]);Serial.print(".");
    Serial.print(wifiSettings.lastIp[2]);Serial.print(".");
    Serial.println(wifiSettings.lastIp[3]);
    wifiSettings.lastIp[0] = ipAddress[0];
    wifiSettings.lastIp[1] = ipAddress[1];
    wifiSettings.lastIp[2] = ipAddress[2];
    wifiSettings.lastIp[3] = ipAddress[3];
    saveSystemSettings();
  }

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

  /*
  for(int i = 0; i < ledCount; ++i){
    strip->SetPixelColor(i, RgbColor(singleColor[0],singleColor[1],singleColor[2]));
  }
  strip->Show();*/

  //MDNS.addService("http", "tcp", 80);

  Serial.println("Setup finished.");
}

//=============================================================================================
void loop() {
  //MDNS.update();
  server.handleClient();
}




 
