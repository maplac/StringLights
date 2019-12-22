//#include <ESP8266mDNS.h>
#include <WiFi.h>
#include <WebServer.h>
#include <SPIFFS.h>
#include <NeoPixelBus.h>
#include <ArduinoJson.h>

#define SAVED_COLORS_COUNT  30
#define MAX_LED_COUNT       1000
#define COLOR_NAME_LENGTH   13 // 12 chars + \0
#define EFFECT_SINGLE       0
#define EFFECT_MULTI        1
#define MULTI_COLOR_COUNT   10
#define SINGLE_COLOR_COUNT   10
#define MAX_SETTINGS_FILE_SIZE  8192
#define MAX_WIFI_CHAR_LENGTH    31
#define MULTICOLOR_ASSIGNMENT_REPEAT  0
#define MULTICOLOR_ASSIGNMENT_SPREAD  1

struct wifi_struct {
  char ssid[MAX_WIFI_CHAR_LENGTH];
  char password[MAX_WIFI_CHAR_LENGTH];
  unsigned char lastIp[4];
  unsigned char currentIp[4];
  bool staticActive;
  unsigned char staticIp[4];
  unsigned char subnet[4];
  unsigned char gateway[4];
  unsigned char dns[4];
  bool isSoftAP;
} wifiSettings;

struct button_struct {
  int state = HIGH;
  volatile unsigned long lastDebounceTime = 0;
  volatile bool debouncing = false;
  unsigned long lastPressTime = 0;
  bool longPressEnabled = false;
} button1, button2;

struct connecting_to_wifi_struct {
  bool connected;
  unsigned long lastTime;
} wifiStatus;

const int gpioLed2B = 33;
const int gpioLed2G = 32;
const int gpioLed2R = 25;
const int gpioLed1B = 27;
const int gpioLed1G = 26;
const int gpioLed1R = 14;
//const int gpioLedProcessing = 16;
const int gpioSwitch1 = 16;
const int gpioBut1 = 22;
const int gpioBut2 = 23;
const unsigned long debounceDelay = 50;
const unsigned long longPressTime = 1000;
int ledCount = 1;

volatile int butCounter = 0;
volatile int lastButCounter = 0;

NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod > *strip;
WebServer server(80);

unsigned char savedColors[SAVED_COLORS_COUNT][3] ;
char savedNames[SAVED_COLORS_COUNT][COLOR_NAME_LENGTH];
unsigned char singleColor[SINGLE_COLOR_COUNT][3];
int singleColorIndex;
unsigned char multiColor[MAX_LED_COUNT][3];
int multiColorLength;
//int multiColorCount = 10;
int multiColorIndex;
int multiColorAssignment = 0;
char multiColorNames[MULTI_COLOR_COUNT][COLOR_NAME_LENGTH];
int currentEffect;
int isOn;
volatile int dummy = 0;
bool setupFailed;

char *fileBuffer;
StaticJsonDocument<MAX_SETTINGS_FILE_SIZE> jsonDoc;
String errorMessage;


int loadSingleColor();
int loadColorPicker();
int loadMultiColor();
int loadMultiColorSlot();
int loadCurrentSettings();
int loadSystemSettings();

bool saveColorPickerSettings();
bool saveSingleColor();
bool saveMultiColor();
bool saveMultiColorSlot();
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
void handleTransparent();

void handleButton1();
void handleButton2();

//=============================================================================================
//=============================================================================================
//=============================================================================================
#define NONE   0x0
#define RED   0x1
#define GREEN 0x2
#define BLUE  0x4

void setLedColor(int led, int colorMask) {
  if (led == 1) {
    if (colorMask & RED) {
      digitalWrite(gpioLed1R, HIGH);
    } else {
      digitalWrite(gpioLed1R, LOW);
    }
    if (colorMask & GREEN) {
      digitalWrite(gpioLed1G, HIGH);
    } else {
      digitalWrite(gpioLed1G, LOW);
    }
    if (colorMask & BLUE) {
      digitalWrite(gpioLed1B, HIGH);
    } else {
      digitalWrite(gpioLed1B, LOW);
    }
  } else if (led == 2){
    if (colorMask & RED) {
      digitalWrite(gpioLed2R, HIGH);
    } else {
      digitalWrite(gpioLed2R, LOW);
    }
    if (colorMask & GREEN) {
      digitalWrite(gpioLed2G, HIGH);
    } else {
      digitalWrite(gpioLed2G, LOW);
    }
    if (colorMask & BLUE) {
      digitalWrite(gpioLed2B, HIGH);
    } else {
      digitalWrite(gpioLed2B, LOW);
    }
  }
}

void handleInterruptBut1() {
  button1.lastDebounceTime = millis();
  button1.debouncing = true;
}
void handleInterruptBut2() {
  button2.lastDebounceTime = millis();
  button2.debouncing = true;
}

inline unsigned char stringToNum(char a, char b, char c) {
  return (a - 48) * 100 + (b - 48) * 10 + (c - 48);
}
void stringToArray(String &str, char arr[], int maxLength) {
  for (int i = 0; i < maxLength; i++) {
    if (i == str.length()) {
      arr[i] = '\0';
    } else if (i > str.length()) {
      break;
    }
    arr[i] = str[i];
  }
}

void stringToArray(char *str, char arr[], int maxLength) {
  for (int i = 0; i < maxLength; i++) {
    arr[i] = str[i];
    if (arr[i] == '\0') {
      break;
    }
  }
}

void printIp(unsigned char ip[]) {
  Serial.print(ip[0]); Serial.print(".");
  Serial.print(ip[1]); Serial.print(".");
  Serial.print(ip[2]); Serial.print(".");
  Serial.print(ip[3]);
}

//=============================================================================================
int readJson(char *fileName, int dataOffset) {
  File file = SPIFFS.open(fileName, "r");
  if (!file) {
    errorMessage = "File cannot be opened";
    return -1;
  }
  
  size_t fileSize = file.size();
  if (fileSize > MAX_SETTINGS_FILE_SIZE) {
    file.close();
    errorMessage = "File is too big.";
    return -1;
  }
  
  int bufIndex = 0;
  for (int i = 0; i < fileSize; i++) {
    if (i < dataOffset) {
      file.read();
      continue;
    }
    if (i == (fileSize - 1) && dataOffset != 0){
      file.read();
      continue;
    }
    fileBuffer[bufIndex++] = file.read();
  }
  fileBuffer[bufIndex] = '\0';
  file.close();

  //Serial.print("size: "); Serial.println(fileSize);
  //Serial.print("buf: "); Serial.println(fileBuffer);
  
  DeserializationError err = deserializeJson(jsonDoc, fileBuffer);
  if (err) {
    errorMessage = "Parsing JSON failed: ";
    errorMessage.concat(err.c_str());
    return -1;
  }
  return 0;
}

//=============================================================================================
void applySettings() {
  if (isOn) {
    if (currentEffect == EFFECT_MULTI) {
      if (multiColorAssignment == MULTICOLOR_ASSIGNMENT_SPREAD) {
        int denominator = ceil(ledCount/multiColorLength);
        for(int i = 0; i < ledCount; ++i){
          int index = floor(i/denominator);
          if (index >= multiColorLength) // should not happen
            index = multiColorLength - 1;
          strip->SetPixelColor(i, RgbColor(multiColor[index][0], multiColor[index][1], multiColor[index][2]));
        }
      } else {
        for(int i = 0; i < ledCount; ++i){
          int index = i % multiColorLength;
          strip->SetPixelColor(i, RgbColor(multiColor[index][0], multiColor[index][1], multiColor[index][2]));
        }
      }
    } else if (currentEffect == EFFECT_SINGLE) {
      for (int i = 0; i < ledCount; ++i) {
        strip->SetPixelColor(i, RgbColor(singleColor[singleColorIndex][0], singleColor[singleColorIndex][1], singleColor[singleColorIndex][2]));
      }
    }
  } else {
    for (int i = 0; i < ledCount; ++i) {
      strip->SetPixelColor(i, RgbColor(0, 0, 0));
    }
  }
  strip->Show();
}

//=============================================================================================
void checkWifiConnection() {
  unsigned long timeNow = millis();
  if ((timeNow - wifiStatus.lastTime) > 1000) {
    wifiStatus.lastTime = timeNow;
    if (WiFi.status() != WL_CONNECTED) {
      static int dotCounter = 0;
      if (wifiStatus.connected) {
        wifiStatus.connected = false;
        dotCounter = 0;
        Serial.println("WiFi connection lost. Reconnecting...");
        setLedColor(1, GREEN);
      } else {
        dotCounter++;
        if (dotCounter == 60) {
          dotCounter = 0;
          Serial.println(".");
        } else {
          Serial.print(".");
        }
      }
    } else {
      if (!wifiStatus.connected) {
        wifiStatus.connected = true;
        Serial.println("WiFi connected.");
        IPAddress ipAddress = WiFi.localIP();
        IPAddress ipAddressLast(wifiSettings.currentIp[0], wifiSettings.currentIp[1], wifiSettings.currentIp[2], wifiSettings.currentIp[3]);
      
        Serial.print("IP address: ");
        Serial.println(ipAddress);
      
        if (ipAddress != ipAddressLast) {
          Serial.println("IP address changed since last time.");
        }
        wifiSettings.lastIp[0] = wifiSettings.currentIp[0];
        wifiSettings.lastIp[1] = wifiSettings.currentIp[1];
        wifiSettings.lastIp[2] = wifiSettings.currentIp[2];
        wifiSettings.lastIp[3] = wifiSettings.currentIp[3];
        wifiSettings.currentIp[0] = ipAddress[0];
        wifiSettings.currentIp[1] = ipAddress[1];
        wifiSettings.currentIp[2] = ipAddress[2];
        wifiSettings.currentIp[3] = ipAddress[3];
        saveSystemSettings();

        server.begin();
        Serial.println("HTTP server started");
        setLedColor(1, BLUE);
      }
    }
  }
}
//=============================================================================================
void setup() {
  int freeHeap = ESP.getFreeHeap();
  setupFailed = false;
  
  fileBuffer = new char[MAX_SETTINGS_FILE_SIZE];
  Serial.begin(115200);
  //Serial.setDebugOutput(true);
  Serial.println();
  //delay(100);

  // Initialize GPIO
  pinMode(gpioLed2B, OUTPUT);
  pinMode(gpioLed2G, OUTPUT);
  pinMode(gpioLed2R, OUTPUT);
  pinMode(gpioLed1B, OUTPUT);
  pinMode(gpioLed1G, OUTPUT);
  pinMode(gpioLed1R, OUTPUT);
  setLedColor(1, GREEN);
  setLedColor(2, NONE);
  pinMode(gpioSwitch1, INPUT);
  pinMode(gpioBut1, INPUT);
  pinMode(gpioBut2, INPUT);

  //Initialize File System
  if(!SPIFFS.begin(true)){
    Serial.println("An Error has occurred while mounting SPIFFS");
    setupFailed = true;
    setLedColor(1, RED);
    return;
  }
  Serial.println("File System Initialized");

  // Load system setting
  if (loadSystemSettings() < 0){
    Serial.println("Load system setting failed.");
    setupFailed = true;
    setLedColor(1, RED);
    return;
  }
  Serial.println("System settings loaded.");

  // Initialize WS2812 (NeoPixel) strip
  strip = new NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod >(ledCount, 21);
  strip->Begin();

  int res = 0;
  res += loadColorPicker();
  res += loadSingleColor();
  res += loadMultiColor();
  res += loadMultiColorSlot();
  res += loadCurrentSettings();

  if (res != 0) {
    Serial.println("Load color settings failed.");
    setupFailed = true;
    setLedColor(1, RED);
    return;
  }
   
  applySettings();
  Serial.println("Color settings loaded.");
  Serial.print("Single color: index = ");Serial.print(singleColorIndex);Serial.print(", color = [");
  Serial.print(singleColor[singleColorIndex][0]);Serial.print(", ");
  Serial.print(singleColor[singleColorIndex][1]);Serial.print(", ");
  Serial.print(singleColor[singleColorIndex][2]);Serial.println("]");
  Serial.print("Multi color: index = ");Serial.println(multiColorIndex);
  Serial.print("Current effect: ");Serial.println(currentEffect);
  
  // Initialize Webserver
  server.on("/", handleRoot);
  server.on("/index", HTTP_POST, handleIndex);
  server.on("/color-picker", HTTP_POST, handleColorPicker);
  server.on("/single-color", HTTP_POST, handleSingleColor);
  server.on("/multi-color", HTTP_POST, handleMultiColor);
  server.on("/transparent", HTTP_POST, handleTransparent);
  server.onNotFound(handleWebRequests);
  Serial.println("HTTP server configured.");

  // Override settings (for debugging)
  //ledCount = 1;
  //stringToArray("", wifiSettings.password, MAX_WIFI_CHAR_LENGTH);
  //stringToArray("Tenda", wifiSettings.ssid, MAX_WIFI_CHAR_LENGTH);

  if (digitalRead(gpioSwitch1) == HIGH) {
    wifiSettings.isSoftAP = true;
  } else {
    wifiSettings.isSoftAP = false;
  }
  
  if (wifiSettings.isSoftAP) {
    Serial.println("Using soft-AP.");
    bool result = WiFi.softAP("StringLights01", "busylion");
    if (result == true) {
      setLedColor(1, GREEN | BLUE);
      Serial.println("Ready");
      server.begin();
      Serial.println("HTTP server started");
    } else {
      setLedColor(1, GREEN | RED);
      Serial.println("Failed!");
    }
    Serial.println("Wifi SSID: StringLights01");
    Serial.println("Wifi password: busylion");
    Serial.print("AP IP address: ");
    Serial.println(WiFi.softAPIP());
  } else {
    Serial.print("Wifi SSID: ");
    Serial.println(wifiSettings.ssid);
    //Serial.print("Wifi pwd: ");
    //Serial.println(wifiSettings.password);
    Serial.print("Led count: ");
    Serial.println(ledCount);
    Serial.print("Last IP: ");
    printIp(wifiSettings.lastIp); Serial.println("");
  
    if (wifiSettings.staticActive) {
      Serial.println("Using static IP.");
      Serial.print("IP: ");
      printIp(wifiSettings.staticIp); Serial.println("");
      Serial.print("Subnet mask: ");
      printIp(wifiSettings.subnet); Serial.println("");
      Serial.print("Gateway: ");
      printIp(wifiSettings.gateway); Serial.println("");
      Serial.print("DNS: ");
      printIp(wifiSettings.dns); Serial.println("");
      //IPAddress ip(10,0,0,220);
      //IPAddress gateway(10,0,0,138);
      //IPAddress subnet(255,255,255,0);
      //IPAddress dns(8,8,8,8);
      IPAddress ip(wifiSettings.staticIp[0], wifiSettings.staticIp[1], wifiSettings.staticIp[2], wifiSettings.staticIp[3]);
      IPAddress gateway(wifiSettings.gateway[0], wifiSettings.gateway[1], wifiSettings.gateway[2], wifiSettings.gateway[3]);
      IPAddress subnet(wifiSettings.subnet[0], wifiSettings.subnet[1], wifiSettings.subnet[2], wifiSettings.subnet[3]);
      IPAddress dns(wifiSettings.dns[0], wifiSettings.dns[1], wifiSettings.dns[2], wifiSettings.dns[3]);
      WiFi.config(ip, dns, gateway, subnet);
    } else {
      Serial.println("Using DHCP.");
    }

    WiFi.persistent(false);
    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);
    WiFi.mode(WIFI_STA);
    //WiFi.hostname("StringLights");
    //WiFi.setSleepMode(WIFI_NONE_SLEEP);
    WiFi.begin(wifiSettings.ssid, wifiSettings.password);
    
    wifiStatus.connected = false;
    wifiStatus.lastTime = millis();
  }

  /*
    if (MDNS.begin("test")) {              // Start the mDNS responder for esp8266.local
      Serial.println("mDNS responder started");
    } else {
      Serial.println("Error setting up MDNS responder!");
    }
    MDNS.addService("http", "tcp", 80);
  */

  attachInterrupt(digitalPinToInterrupt(gpioBut1), handleInterruptBut1, CHANGE);
  attachInterrupt(digitalPinToInterrupt(gpioBut2), handleInterruptBut2, CHANGE);

  Serial.print("Free heap at start:    "); Serial.println(freeHeap);
  Serial.print("Free heap after setup: "); Serial.println(ESP.getFreeHeap());
  Serial.println("Setup finished.");
  if (!wifiSettings.isSoftAP)
    Serial.println("Connecting to WiFi...");
}

//=============================================================================================
void loop() {
  if (setupFailed)
    return;
  
  if (!wifiSettings.isSoftAP)
    checkWifiConnection();
  server.handleClient();
  handleButton1();
  handleButton2();
  //MDNS.update();
}
