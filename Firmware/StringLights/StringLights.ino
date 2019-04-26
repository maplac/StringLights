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
  bool accessPointActive;
} wifiSettings;

struct button_struct {
  int state = HIGH;
  volatile unsigned long lastDebounceTime = 0;
  volatile bool debouncing = false;
  unsigned long lastPressTime = 0;
} button1, button2;

const int gpioLedStatus = 12;
const int gpioLedProcessing = 13;
const int gpioLedAcessPoint = 14;
const int gpioBut1 = 4;
const int gpioBut2 = 5;
const unsigned long debounceDelay = 50;
const unsigned long longPressTime = 2000;
int ledCount = 1;

volatile int butCounter = 0;
volatile int lastButCounter = 0;

NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod > *strip;
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

// Allocate a buffer to store contents of files
std::unique_ptr<char[]> buf(new char[MAX_SETTINGS_FILE_SIZE]);
DynamicJsonBuffer jsonBuffer(1000);

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

void handleInterruptBut1(){
  button1.lastDebounceTime = millis();
  button1.debouncing = true;
}
void handleInterruptBut2(){
  button2.lastDebounceTime = millis();
  button2.debouncing = true;
}

inline unsigned char stringToNum(char a, char b, char c){
  return (a - 48) * 100 + (b - 48) * 10 + (c - 48);
}
void stringToArray(String &str, char arr[], int maxLength){
  for(int i = 0; i < maxLength; i++) {
    if (i == str.length()){
      arr[i] = '\0';
    } else if (i > str.length()){
      break;
    }
    arr[i] = str[i];
  }
}

void stringToArray(char *str, char arr[], int maxLength){
  for(int i = 0; i < maxLength; i++) {
    arr[i] = str[i];
    if (arr[i] == '\0'){
      break;
    }
  }
}

void printIp(unsigned char ip[]){
  Serial.print(ip[0]);Serial.print(".");
  Serial.print(ip[1]);Serial.print(".");
  Serial.print(ip[2]);Serial.print(".");
  Serial.print(ip[3]);
}

//=============================================================================================
void applySettings(){
  if(isOn){
    if(currentEffect == EFFECT_MULTI){
      for(int i = 0; i < ledCount; ++i){
        int index = i % multiColorLength;
        strip->SetPixelColor(i, RgbColor(multiColor[index][0], multiColor[index][1], multiColor[index][2]));
      }
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
  Serial.begin(115200);
  //Serial.setDebugOutput(true);
  Serial.println();
  delay(100);

  Serial.print("Heap start: ");Serial.println(ESP.getFreeHeap());

  pinMode(gpioLedStatus, OUTPUT);
  digitalWrite(gpioLedStatus, LOW);
  pinMode(gpioLedProcessing, OUTPUT);
  digitalWrite(gpioLedProcessing, LOW);
  pinMode(gpioLedAcessPoint, OUTPUT);
  digitalWrite(gpioLedAcessPoint, LOW);
  pinMode(gpioBut1, INPUT);
  pinMode(gpioBut2, INPUT);
  attachInterrupt(digitalPinToInterrupt(gpioBut1), handleInterruptBut1, CHANGE);
  attachInterrupt(digitalPinToInterrupt(gpioBut2), handleInterruptBut2, CHANGE);
  
  //Initialize File System
  SPIFFS.begin();
  Serial.println("File System Initialized");
  
  loadSystemSettings(buf, jsonBuffer);
  Serial.println("SystemSettings loaded.");
  
  Serial.print("Heap load: ");Serial.println(ESP.getFreeHeap());
  
  isOn = 1; // todo smazat
  //ledCount = 1;

  Serial.print("Wifi SSID: ");
  Serial.println(wifiSettings.ssid);
  Serial.print("Led count: ");
  Serial.println(ledCount);
  Serial.print("Last IP: ");
  printIp(wifiSettings.lastIp);Serial.println("");
    
  /*
  //Initialize AP Mode
  WiFi.softAP(ssid);  //Password not used
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("Web Server IP:");
  Serial.println(myIP);
  */
  
  wifiSettings.staticActive = false;// todo smazat
  //stringToArray("unstuckunstuck", wifiSettings.password, MAX_WIFI_CHAR_LENGTH);
  //stringToArray("Tenda", wifiSettings.ssid, MAX_WIFI_CHAR_LENGTH);
  
  if(wifiSettings.staticActive){
    Serial.println("Using static IP address");
    Serial.print("IP: ");
    printIp(wifiSettings.staticIp);Serial.println("");
    Serial.print("Subnet mask: ");
    printIp(wifiSettings.subnet);Serial.println("");
    Serial.print("Gateway: ");
    printIp(wifiSettings.gateway);Serial.println("");
    Serial.print("DNS: ");
    printIp(wifiSettings.dns);Serial.println("");
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
  
  //WiFi.hostname("StringLights");
  /*
  // FIX >>>>>
  WiFi.persistent(false);
  WiFi.mode(WIFI_OFF);   // this is a temporary line, to be removed after SDK update to 1.5.4
  WiFi.mode(WIFI_STA);//*/
  // <<<<<<<<<
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  WiFi.begin(wifiSettings.ssid, wifiSettings.password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  
  IPAddress ipAddress = WiFi.localIP();
  
  // Print local IP address
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(ipAddress);
    
  IPAddress ipAddressLast(wifiSettings.lastIp[0], wifiSettings.lastIp[1], wifiSettings.lastIp[2], wifiSettings.lastIp[3]);
  if(ipAddress != ipAddressLast){
    Serial.println("IP address changed since last time.");
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
  MDNS.addService("http", "tcp", 80);
*/
  //Initialize Webserver
  server.on("/",handleRoot);
  server.on("/index", HTTP_POST, handleIndex);
  server.on("/color-picker", HTTP_POST, handleColorPicker);
  server.on("/single-color", HTTP_POST, handleSingleColor);
  server.on("/multi-color", HTTP_POST, handleMultiColor);
  server.onNotFound(handleWebRequests);
  server.begin();
  Serial.println("HTTP server started");

  strip = new NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod >(ledCount);
  strip->Begin();

  loadColorPicker(buf, jsonBuffer);
  Serial.println("ColorPicker loaded.");
  loadSingleColor(buf, jsonBuffer);
  Serial.println("SingleColor loaded.");
  loadMultiColor(buf, jsonBuffer);
  Serial.println("MultiColor loaded.");
  loadCurrentSettings(buf, jsonBuffer);
  Serial.println("CurrentSettings loaded.");
  
  applySettings();
  Serial.println("Settings applied.");

  Serial.println("Setup finished.");
  digitalWrite(gpioLedStatus, 1);
  Serial.print("Heap end: ");Serial.println(ESP.getFreeHeap());
}

//=============================================================================================
void loop() {
  //MDNS.update();
  server.handleClient();

  if (button1.debouncing) {
    if ((millis() - button1.lastDebounceTime) > debounceDelay) {
      button1.debouncing = false;
      int butState = digitalRead(gpioBut1);
      // button pressed
      if ( (button1.state == 1) && (butState == 0)) {
        //Serial.println("But1 pressed");
        button1.lastPressTime = millis();
      }
      // button released
      if ( (button1.state == 0) && (butState == 1)) {
        if ((millis() - button1.lastPressTime) > longPressTime) {
          Serial.println("But1 long press");
        } else {
          Serial.println("But1 short press");
        }
      }
      button1.state = butState;
    }
  }
  if (button2.debouncing) {
    if ((millis() - button2.lastDebounceTime) > debounceDelay) {
      button2.debouncing = false;
      int butState = digitalRead(gpioBut2);
      // button pressed
      if ( (button2.state == 1) && (butState == 0)) {
        //Serial.println("But2 pressed");
        button2.lastPressTime = millis();
      }
      // button released
      if ( (button2.state == 0) && (butState == 1)) {
        if ((millis() - button2.lastPressTime) > longPressTime) {
          Serial.println("But2 long press");
        } else {
          Serial.println("But2 short press");
        }
      }
      button2.state = butState;
    }
  }
}




 
