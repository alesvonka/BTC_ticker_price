#include "display.h"
#include "ntp.h"
#include <Adafruit_GFX.h>

extern String crypto;
extern String curency;
extern String curencySymbol;
extern double price;

void set_display() {
  display.clearDisplay();

  display.setTextSize(1);
  digitalClockDisplay();

  display.setCursor(20, 15);
  display.setTextSize(2);
  display.println(crypto + "/" + curency);

  display.setCursor(0, 38);
  display.setTextSize(2);
  display.print(curencySymbol);
  display.print(" ");
  display.println(price);

  display.display();
}