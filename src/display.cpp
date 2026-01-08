#include "display.h"
#include "ntp.h"
#include "wifimanager.h"
#include <Adafruit_GFX.h>

extern String crypto;
extern String curency;
extern String curencySymbol;
extern double price;

void set_display() {
  display.clearDisplay();

  // --- TOP TICKER BAND (height 12px) ---
  // Static Bitcoin price display
  String tickerText = crypto + ": " + curencySymbol + " " + String(price, 0);
  display.setCursor(0, 1);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.print(tickerText);
  
  // --- SEPARATOR LINE ---
  display.drawLine(0, 12, 127, 12, SSD1306_WHITE);
  
  // --- BOTTOM CLOCK (large format) ---
  display.setTextSize(4);  // Large numbers
  display.setTextColor(SSD1306_WHITE);
  
  // Hours
  display.setCursor(7, 19);
  if (hour() < 10) display.print('0');
  display.print(hour());
  
  // Minutes
  display.setCursor(78, 19);
  if (minute() < 10) display.print('0');
  display.print(minute());
  
  // Seconds (small, centered)
  display.setTextSize(1);
  display.setCursor(58, 32);
  if (second() < 10) display.print('0');
  display.print(second());
  
  // --- BOTTOM STATUS LINE (IP / AP info) ---
  display.drawLine(0, 54, 127, 54, SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 57);
  
  if (wifiManagerIsConnected()) {
    // Připojeno k WiFi - zobrazit IP
    display.print("IP:");
    display.print(wifiManagerGetIP());
  } else {
    // AP mód - zobrazit název AP
    display.print("AP:");
    display.print(wifiManagerGetAPName());
  }
  
  display.display();
}

// Funkce pro zobrazení statusové zprávy (např. při startu)
void displayShowStatus(const char* line1, const char* line2) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  
  display.setCursor(0, 20);
  display.print(line1);
  
  display.setCursor(0, 35);
  display.print(line2);
  
  display.display();
}