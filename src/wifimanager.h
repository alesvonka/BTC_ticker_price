#ifndef WIFIMANAGER_H
#define WIFIMANAGER_H

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>

// Stavy WiFi manageru
enum WiFiManagerState {
  WM_CONNECTING,      // Pokus o připojení k uložené WiFi
  WM_CONNECTED,       // Připojeno k WiFi
  WM_AP_MODE,         // AP mód aktivní
  WM_AP_CONFIG        // Konfigurace přes AP
};

// Struktura pro uložení credentials
struct WiFiCredentials {
  char ssid[33];
  char password[65];
  char apPassword[33];
  uint32_t checksum;
};

// Hlavní funkce
void wifiManagerSetup();
void wifiManagerLoop();
bool wifiManagerIsConnected();
WiFiManagerState wifiManagerGetState();
String wifiManagerGetIP();
String wifiManagerGetAPName();
void wifiManagerStartAP();
void wifiManagerStopAP();

// EEPROM funkce
void loadWiFiCredentials();
void saveWiFiCredentials(const char* ssid, const char* password);
void saveAPPassword(const char* apPassword);
void clearWiFiCredentials();

// Pomocné funkce
uint32_t calculateChecksum(WiFiCredentials* creds);
bool isValidCredentials(WiFiCredentials* creds);

#endif
