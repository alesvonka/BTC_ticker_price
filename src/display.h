#ifndef DISPLAY_H
#define DISPLAY_H

#include <Adafruit_SSD1306.h>

extern Adafruit_SSD1306 display;

void set_display();
void displayShowStatus(const char* line1, const char* line2);  // Pro zobrazení statusu

#endif