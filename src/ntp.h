#ifndef NTP_H
#define NTP_H

#include <TimeLib.h>
#include <ESP8266WiFi.h>

time_t getNtpTime();
void digitalClockDisplay();
void sendNTPpacket(IPAddress &address);

#endif