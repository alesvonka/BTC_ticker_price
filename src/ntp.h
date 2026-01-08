#ifndef NTP_H
#define NTP_H

#include <TimeLib.h>
#include <ESP8266WiFi.h>

time_t getNtpTime();
void digitalClockDisplay();
void sendNTPpacket(IPAddress &address);
bool isDST(int day, int month, int dow, int hour);  // Detekce letního času
int getTimeOffset(time_t utc);  // Vrací offset v hodinách (1 nebo 2 pro CET/CEST)

#endif