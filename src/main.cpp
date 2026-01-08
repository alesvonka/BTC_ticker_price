#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <TimeLib.h>

#include "display.h"
#include "json.h"
#include "logo.h"
#include "ntp.h"
#include "wifi.h"
#include "wifimanager.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_ADDR 0x3C
Adafruit_SSD1306 display = Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);

/* NTP client */
extern int currentNtpServer;
int timeZone = 1;  // Central European Time
WiFiUDP Udp;
unsigned int localPort = 8888;

String http_adress = "https://api.coingecko.com/api/v3/simple/price?ids=bitcoin&vs_currencies=usd&precision=2";
String curencySymbol = "$";
String curency = "USD";
String crypto = "BTC";
String payload = "";
double price = 0;

unsigned int timer = 60000;  // 60s mezi požadavky na API
unsigned long sent_request_time = 0;
time_t displayReload = 0;
bool ntpSynced = false;

void setup() {
  Serial.begin(115200);
  while (!Serial) { yield(); }
  Serial.println(F("\n\n=== BTC Ticker v1.1 ==="));
  delay(10);

  // Inicializace displeje
  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  
  // Zobrazit logo
  logo();
  
  // Zobrazit status
  displayShowStatus("Starting...", "WiFi Manager");
  delay(1500);
  
  // Inicializace WiFi Manageru (AP + připojení)
  wifiManagerSetup();
  
  // Pokud jsme připojeni, inicializovat NTP
  if (wifiManagerIsConnected()) {
    displayShowStatus("WiFi Connected!", wifiManagerGetIP().c_str());
    delay(2000);
    
    Udp.begin(localPort);
    Serial.print(F("UDP port: "));
    Serial.println(localPort);
    
    displayShowStatus("Syncing time...", "Please wait");
    setSyncProvider(getNtpTime);
    setSyncInterval(300);  // Sync každých 5 minut
    
    // Počkat na první sync
    unsigned long ntpStart = millis();
    while (timeStatus() == timeNotSet && millis() - ntpStart < 10000) {
      delay(100);
      yield();
    }
    ntpSynced = (timeStatus() != timeNotSet);
    
    if (ntpSynced) {
      Serial.println(F("NTP sync OK"));
    } else {
      Serial.println(F("NTP sync failed"));
    }
  } else {
    displayShowStatus("AP Mode Active", "Connect to WiFi:");
    delay(500);
    displayShowStatus("SSID: BTC_Ticker", "Pass: 12345678");
    delay(2000);
  }
  
  displayReload = now();
  set_display();
}

void loop() {
  // KRITICKÉ: Obsluha WiFi Manageru (webserver, DNS)
  wifiManagerLoop();
  
  // Aktualizace displeje každou sekundu
  if (displayReload != now()) {
    set_display();
    displayReload = now();
  }
  
  // Inicializace NTP pokud jsme se právě připojili
  if (!ntpSynced && wifiManagerIsConnected()) {
    static unsigned long lastNtpTry = 0;
    if (millis() - lastNtpTry > 5000) {
      lastNtpTry = millis();
      
      if (!Udp.localPort()) {
        Udp.begin(localPort);
      }
      setSyncProvider(getNtpTime);
      setSyncInterval(300);
      
      if (timeStatus() != timeNotSet) {
        ntpSynced = true;
        Serial.println(F("NTP sync OK (delayed)"));
      }
    }
  }
  
  // Dotaz na cenu BTC (pouze pokud jsme připojeni)
  if (wifiManagerIsConnected() && sent_request_time < millis()) {
    Serial.println(F("Fetching BTC price..."));
    connect_wifi();  // Tato funkce získá cenu
    sent_request_time = millis() + timer;
    Serial.print(F("Next update in "));
    Serial.print(timer / 1000);
    Serial.println(F(" seconds"));
  }
  
  yield();  // Důležité pro ESP8266!
}