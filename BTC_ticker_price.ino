/* Display SSD1306 */
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET    -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define OLED_ADDR     0x3C // I2C adresa displaye
Adafruit_SSD1306 display = Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire); // 0X3C+SA0 - 0x3C or 0x3D

#include <ArduinoJson.h>
String payload = "{}";

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#ifndef APSSID
#define APSSID "BTC_Ticker"
#define APPSK  "12345678"
#endif
/* Set these to your desired credentials. */
const char *ssid = APSSID;
const char *password = APPSK;
const int led = LED_BUILTIN;
ESP8266WebServer server(80);


#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>

ESP8266WiFiMulti WiFiMulti;

/* NTP client */
#include <TimeLib.h>
#include <WiFiUdp.h>
static const char ntpServerName[] = "cz.pool.ntp.org";
const int timeZone = 1;     // Central European Time
WiFiUDP Udp;
unsigned int localPort = 8888;  // local port to listen for UDP packets
time_t getNtpTime();
void digitalClockDisplay();
void printDigits(int digits);
void sendNTPpacket(IPAddress &address);
/* NTP client konec */

String http_adress      = "https://blockchain.info/ticker";
String curencySymbol    = "$";
String curency          = "USD";
String crypto           = "BTC";
int price               = 0;

unsigned int timer              = 10000;//600000; //600s // 10min
unsigned long sent_request_time = millis();

void setup() {
  Serial.begin(115200);
  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  display.setTextSize(3);
  display.setTextColor(SSD1306_WHITE); // Draw white text

  logo();
  //WiFi.mode(WIFI_OFF);
  //delay(500);
  WiFi.mode(WIFI_STA);
  delay(500);

  Serial.println("Pripojeni k wifi:");
  WiFiMulti.run();

  Serial.print("Configuring access point...");
  IPAddress Ip(192, 168, 1, 1);
  IPAddress NMask(255, 255, 255, 0);
  WiFi.softAPConfig(Ip, Ip, NMask);
  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  server.on("/", handleRoot);
  server.on("/setConnect/", handleSetConnect);
  server.on("/postform/", handleForm);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");

  Udp.begin(localPort);
  Serial.print("Local port: ");
  Serial.println(Udp.localPort());
  Serial.println("waiting for sync");
  setSyncProvider(getNtpTime);
  setSyncInterval(150);
}

time_t prevDisplay = 0;

void loop() {
  server.handleClient();

  if (now() != prevDisplay) {
    set_display();
    prevDisplay = now();
  }

  /* Pripojeni k burze */
  if (sent_request_time < millis()) {

    connect_wifi();

    if ((WiFiMulti.run() == WL_CONNECTED)) {

      set_display();

      Serial.println("Wait " + String(timer) + " millis before next round...");
      sent_request_time = millis() + timer;
    }

  }

}
