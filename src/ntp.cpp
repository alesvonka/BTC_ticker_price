#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include "ntp.h"
#include "display.h"

extern WiFiUDP Udp;
extern int timeZone;
extern unsigned int localPort;

// NTP servery - spolehlivé s fallbackem
const char* ntpServers[] = {
  "ptbtime1.ptb.de",      // PTB Německo - primární metrologický institut
  "time.google.com",       // Google Public NTP
  "time.cloudflare.com",   // Cloudflare NTP
  "pool.ntp.org"           // NTP Pool fallback
};
const int ntpServerCount = 4;
int currentNtpServer = 0;

const int NTP_PACKET_SIZE = 48;      // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE];  //buffer to hold incoming & outgoing packets

void digitalClockDisplay() {
  // digital clock display moved to set_display()
  // Serial output for debugging
  Serial.print(day());
  Serial.print(".");
  Serial.print(month());
  Serial.print(".");
  Serial.print(year());
  Serial.print(" ");

  if (hour() < 10) Serial.print('0');
  Serial.print(hour());
  Serial.print(':');
  if (minute() < 10) Serial.print('0');
  Serial.print(minute());
  Serial.print(':');
  if (second() < 10) Serial.print('0');
  Serial.print(second());
  Serial.println();
}

/*-------- DST (Daylight Saving Time) pro střední Evropu ----------*/

// Vypočítá poslední neděli v daném měsíci
int lastSundayOfMonth(int year, int month) {
  // Zeller's algorithm pro den v týdnu
  int lastDay;
  if (month == 3) lastDay = 31;  // březen
  else if (month == 10) lastDay = 31;  // říjen
  else return 0;
  
  // Výpočet dne v týdnu pro poslední den měsíce
  int y = year;
  int m = month;
  if (m < 3) { m += 12; y--; }
  int k = y % 100;
  int j = y / 100;
  int dow = (lastDay + (13 * (m + 1)) / 5 + k + k / 4 + j / 4 - 2 * j) % 7;
  // Převod: 0=So, 1=Ne, 2=Po, ... 6=Pá
  dow = (dow + 6) % 7;  // 0=Ne, 1=Po, ... 6=So
  
  return lastDay - dow;
}

// Kontrola zda je letní čas (CEST) pro střední Evropu
// Letní čas: poslední neděle v březnu 2:00 UTC -> poslední neděle v říjnu 3:00 UTC
bool isDST(int day, int month, int year, int hour) {
  if (month < 3 || month > 10) return false;  // Leden, únor, listopad, prosinec = zimní
  if (month > 3 && month < 10) return true;   // Duben až září = letní
  
  int lastSunMarch = lastSundayOfMonth(year, 3);
  int lastSunOct = lastSundayOfMonth(year, 10);
  
  if (month == 3) {
    // Březen: letní čas začíná poslední neděli ve 2:00 UTC
    if (day > lastSunMarch) return true;
    if (day < lastSunMarch) return false;
    return (hour >= 2);  // Přesně poslední neděle - záleží na hodině
  }
  
  if (month == 10) {
    // Říjen: letní čas končí poslední neděli ve 3:00 UTC
    if (day < lastSunOct) return true;
    if (day > lastSunOct) return false;
    return (hour < 3);  // Přesně poslední neděle - záleží na hodině
  }
  
  return false;
}

// Vrací časový offset v hodinách pro střední Evropu
int getTimeOffset(time_t utc) {
  tmElements_t tm;
  breakTime(utc, tm);
  
  if (isDST(tm.Day, tm.Month, tm.Year + 1970, tm.Hour)) {
    return 2;  // CEST (letní čas)
  }
  return 1;    // CET (zimní čas)
}

/*-------- NTP code with fallback ----------*/

time_t getNtpTime() {
  IPAddress ntpServerIP;
  
  // Zkusit všechny servery postupně
  for (int attempt = 0; attempt < ntpServerCount; attempt++) {
    int serverIndex = (currentNtpServer + attempt) % ntpServerCount;
    const char* serverName = ntpServers[serverIndex];
    
    while (Udp.parsePacket() > 0);
    
    Serial.print("NTP Request [");
    Serial.print(attempt + 1);
    Serial.print("/");
    Serial.print(ntpServerCount);
    Serial.print("]: ");
    Serial.println(serverName);
    
    if (WiFi.hostByName(serverName, ntpServerIP)) {
      Serial.print("IP: ");
      Serial.println(ntpServerIP);
      sendNTPpacket(ntpServerIP);
      
      uint32_t beginWait = millis();
      while (millis() - beginWait < 2000) {
        int size = Udp.parsePacket();
        if (size >= NTP_PACKET_SIZE) {
          Serial.println("NTP Response OK");
          Udp.read(packetBuffer, NTP_PACKET_SIZE);
          unsigned long secsSince1900;
          secsSince1900 = (unsigned long)packetBuffer[40] << 24;
          secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
          secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
          secsSince1900 |= (unsigned long)packetBuffer[43];
          
          // Převod na Unix čas (UTC)
          time_t utcTime = secsSince1900 - 2208988800UL;
          
          // Automatický offset pro střední Evropu (CET/CEST)
          int offset = getTimeOffset(utcTime);
          Serial.print("Timezone offset: UTC+");
          Serial.print(offset);
          Serial.println(offset == 2 ? " (CEST - letní čas)" : " (CET - zimní čas)");
          
          // Zapamatovat si úspěšný server pro příště
          currentNtpServer = serverIndex;
          return utcTime + offset * SECS_PER_HOUR;
        }
      }
      Serial.println("No response, trying next server...");
    } else {
      Serial.println("DNS failed, trying next server...");
    }
    delay(100);  // Krátká pauza před dalším pokusem
  }
  
  Serial.println("All NTP servers failed!");
  return 0;
}

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address) {
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;  // LI, Version, Mode
  packetBuffer[1] = 0;           // Stratum, or type of clock
  packetBuffer[2] = 6;           // Polling Interval
  packetBuffer[3] = 0xEC;        // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123);  //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}