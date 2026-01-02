#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <TimeLib.h>

#include "display.h"
#include "json.h"
#include "logo.h"
#include "ntp.h"
#include "wifi.h"

#define SCREEN_WIDTH 128                                                          // OLED display width, in pixels
#define SCREEN_HEIGHT 64                                                          // OLED display height, in pixels
#define OLED_RESET -1                                                             // Reset pin # (or -1 if sharing Arduino reset pin)
#define OLED_ADDR 0x3C                                                            // I2C adresa displaye
Adafruit_SSD1306 display = Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);  // 0X3C+SA0 - 0x3C or 0x3D

ESP8266WiFiMulti WiFiMulti;

/* NTP client */
const char* ntpServerName = "cz.pool.ntp.org";
int timeZone = 1;  // Central European Time
WiFiUDP Udp;
unsigned int localPort = 8888;  // local port to listen for UDP packets

String http_adress = "https://api.coingecko.com/api/v3/simple/price?ids=bitcoin&vs_currencies=usd&precision=2";
String curencySymbol = "$";
String curency = "USD";
String crypto = "BTC";
String payload = "";  // Global payload for HTTP response
double price = 0;

unsigned int timer = 60000;  //600000; //600s // 10s
unsigned long sent_request_time = millis();
time_t displayReload = now();

void setup() {
  Serial.begin(115200);
  while (!Serial) {}
  Serial.println("Serial.begin 115200");
  delay(1000);

  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  display.setTextSize(3);
  display.setTextColor(SSD1306_WHITE);  // Draw white text

  logo();

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP("MERCUSYS_457A", "71405254");
  delay(500);

  Serial.println("Pripojeni k wifi:");
  WiFiMulti.run();

  Udp.begin(localPort);
  Serial.print("Local port: ");
  Serial.println(Udp.localPort());
  Serial.println("waiting for sync");
  setSyncProvider(getNtpTime);
  setSyncInterval(150);

  set_display();
}

void loop() {

  if (displayReload != now()) {
    set_display();
    displayReload = now();
  }

  /* Pripojeni k burze */
  if (sent_request_time < millis()) {

    connect_wifi();

    if ((WiFiMulti.run() == WL_CONNECTED)) {
      Serial.println("Wait " + String(timer) + " millis before next round...");
      sent_request_time = millis() + timer;
    }
  }
}