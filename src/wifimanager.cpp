#include "wifimanager.h"
#include "display.h"
#include <EEPROM.h>

// Konstanty
#define EEPROM_SIZE 256
#define EEPROM_ADDR 0
#define AP_NAME "BTC_Ticker"
#define AP_DEFAULT_PASSWORD "12345678"
#define DNS_PORT 53
#define WEB_PORT 80
#define CONNECT_TIMEOUT 15000  // 15 sekund timeout pro připojení
#define AP_IP IPAddress(192, 168, 4, 1)

// Globální objekty
ESP8266WebServer webServer(WEB_PORT);
DNSServer dnsServer;

// Stav
WiFiManagerState wmState = WM_CONNECTING;
WiFiCredentials credentials;
unsigned long connectStartTime = 0;
unsigned long lastConnectAttempt = 0;
bool apActive = false;
String currentIP = "";
String apName = AP_NAME;

// HTML stránky (uložené v PROGMEM pro úsporu RAM)
const char HTML_HEADER[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html><head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>BTC Ticker Setup</title>
<style>
*{box-sizing:border-box;margin:0;padding:0}
body{font-family:Arial,sans-serif;background:#1a1a2e;color:#eee;padding:20px}
.container{max-width:400px;margin:0 auto}
h1{color:#f7931a;text-align:center;margin-bottom:20px}
.card{background:#16213e;border-radius:10px;padding:20px;margin-bottom:15px}
.card h2{color:#f7931a;margin-bottom:15px;font-size:18px}
input[type=text],input[type=password]{width:100%;padding:12px;margin:8px 0;border:1px solid #0f3460;border-radius:5px;background:#0f3460;color:#eee;font-size:16px}
input[type=submit]{width:100%;padding:14px;background:#f7931a;color:#1a1a2e;border:none;border-radius:5px;font-size:16px;font-weight:bold;cursor:pointer;margin-top:10px}
input[type=submit]:hover{background:#e8820d}
.status{padding:10px;border-radius:5px;text-align:center;margin-bottom:15px}
.success{background:#27ae60}
.error{background:#c0392b}
.info{background:#2980b9}
.networks{max-height:200px;overflow-y:auto}
.network{padding:10px;background:#0f3460;margin:5px 0;border-radius:5px;cursor:pointer}
.network:hover{background:#1a4a8a}
.signal{float:right;color:#888}
a{color:#f7931a;text-decoration:none}
.footer{text-align:center;margin-top:20px;color:#666;font-size:12px}
</style></head><body><div class="container">
<h1>🔶 BTC Ticker</h1>
)rawliteral";

const char HTML_FOOTER[] PROGMEM = R"rawliteral(
<div class="footer">BTC Ticker v1.1 | ESP8266</div>
</div></body></html>
)rawliteral";

// Funkce pro výpočet checksumu
uint32_t calculateChecksum(WiFiCredentials* creds) {
  uint32_t sum = 0;
  uint8_t* ptr = (uint8_t*)creds;
  for (size_t i = 0; i < sizeof(WiFiCredentials) - sizeof(uint32_t); i++) {
    sum += ptr[i] * (i + 1);
  }
  return sum ^ 0xB7C12345;
}

// Kontrola validity uložených credentials
bool isValidCredentials(WiFiCredentials* creds) {
  if (creds->ssid[0] == 0 || creds->ssid[0] == 0xFF) return false;
  return (creds->checksum == calculateChecksum(creds));
}

// Načtení credentials z EEPROM
void loadWiFiCredentials() {
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.get(EEPROM_ADDR, credentials);
  EEPROM.end();
  
  if (!isValidCredentials(&credentials)) {
    memset(&credentials, 0, sizeof(credentials));
    strcpy(credentials.apPassword, AP_DEFAULT_PASSWORD);
    Serial.println(F("No valid WiFi credentials found"));
  } else {
    Serial.print(F("Loaded SSID: "));
    Serial.println(credentials.ssid);
  }
}

// Uložení WiFi credentials
void saveWiFiCredentials(const char* ssid, const char* password) {
  // Vymazat staré SSID a heslo
  memset(credentials.ssid, 0, sizeof(credentials.ssid));
  memset(credentials.password, 0, sizeof(credentials.password));
  
  // Kopírovat nové hodnoty
  strncpy(credentials.ssid, ssid, sizeof(credentials.ssid) - 1);
  strncpy(credentials.password, password, sizeof(credentials.password) - 1);
  
  // Zajistit null-terminaci
  credentials.ssid[sizeof(credentials.ssid) - 1] = '\0';
  credentials.password[sizeof(credentials.password) - 1] = '\0';
  
  // Pokud AP heslo není nastaveno, použít výchozí
  if (credentials.apPassword[0] == 0 || credentials.apPassword[0] == 0xFF) {
    strcpy(credentials.apPassword, AP_DEFAULT_PASSWORD);
  }
  
  credentials.checksum = calculateChecksum(&credentials);
  
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.put(EEPROM_ADDR, credentials);
  bool success = EEPROM.commit();
  EEPROM.end();
  
  Serial.print(F("Saved SSID: "));
  Serial.print(ssid);
  Serial.print(F(" - EEPROM commit: "));
  Serial.println(success ? F("OK") : F("FAILED"));
}

// Uložení AP hesla
void saveAPPassword(const char* apPassword) {
  strncpy(credentials.apPassword, apPassword, sizeof(credentials.apPassword) - 1);
  credentials.checksum = calculateChecksum(&credentials);
  
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.put(EEPROM_ADDR, credentials);
  EEPROM.commit();
  EEPROM.end();
  
  Serial.print(F("Saved AP password"));
}

// Smazání credentials
void clearWiFiCredentials() {
  memset(&credentials, 0, sizeof(credentials));
  strcpy(credentials.apPassword, AP_DEFAULT_PASSWORD);
  
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.put(EEPROM_ADDR, credentials);
  EEPROM.commit();
  EEPROM.end();
  
  Serial.println(F("Credentials cleared"));
}

// Skenování WiFi sítí
String scanNetworks() {
  String html = "<div class=\"networks\">";
  int n = WiFi.scanNetworks();
  
  if (n == 0) {
    html += "<p>Žádné sítě nenalezeny</p>";
  } else {
    for (int i = 0; i < n && i < 10; i++) {
      int rssi = WiFi.RSSI(i);
      String signal;
      if (rssi > -50) signal = "████";
      else if (rssi > -60) signal = "███░";
      else if (rssi > -70) signal = "██░░";
      else signal = "█░░░";
      
      html += "<div class=\"network\" onclick=\"document.getElementById('ssid').value='" + WiFi.SSID(i) + "'\">";
      html += WiFi.SSID(i);
      html += "<span class=\"signal\">" + signal + "</span>";
      html += "</div>";
    }
  }
  html += "</div>";
  WiFi.scanDelete();
  return html;
}

// Hlavní stránka
void handleRoot() {
  String html = FPSTR(HTML_HEADER);
  
  // Status
  if (WiFi.status() == WL_CONNECTED) {
    html += "<div class=\"status success\">✓ Připojeno k: " + WiFi.SSID() + "</div>";
  } else {
    html += "<div class=\"status error\">✗ Nepřipojeno k WiFi</div>";
  }
  
  // WiFi formulář
  html += "<div class=\"card\"><h2>📶 WiFi nastavení</h2>";
  html += "<form action=\"/save\" method=\"POST\">";
  html += "<input type=\"text\" id=\"ssid\" name=\"ssid\" placeholder=\"Název WiFi sítě\" required>";
  html += "<input type=\"password\" name=\"password\" placeholder=\"Heslo WiFi\">";
  html += "<input type=\"submit\" value=\"Připojit\">";
  html += "</form>";
  html += "<p style=\"margin-top:15px;font-size:14px\">Dostupné sítě:</p>";
  html += scanNetworks();
  html += "</div>";
  
  // AP heslo
  html += "<div class=\"card\"><h2>🔐 Heslo AP</h2>";
  html += "<form action=\"/ap\" method=\"POST\">";
  html += "<input type=\"password\" name=\"appass\" placeholder=\"Nové heslo AP (min 8 znaků)\" minlength=\"8\">";
  html += "<input type=\"submit\" value=\"Změnit heslo AP\">";
  html += "</form></div>";
  
  // Reset
  html += "<div class=\"card\"><h2>⚠️ Reset</h2>";
  html += "<a href=\"/reset\" onclick=\"return confirm('Opravdu smazat nastavení?')\">Smazat uložené WiFi</a>";
  html += "</div>";
  
  html += FPSTR(HTML_FOOTER);
  webServer.send(200, "text/html", html);
}

// Uložení WiFi
void handleSave() {
  String ssid = webServer.arg("ssid");
  String password = webServer.arg("password");
  
  if (ssid.length() > 0) {
    saveWiFiCredentials(ssid.c_str(), password.c_str());
    
    String html = FPSTR(HTML_HEADER);
    html += "<div class=\"status success\">✓ WiFi uloženo!</div>";
    html += "<div class=\"card\"><p>Připojuji se k síti: <strong>" + ssid + "</strong></p>";
    html += "<p>Počkejte prosím...</p></div>";
    html += "<meta http-equiv=\"refresh\" content=\"10;url=/\">";
    html += FPSTR(HTML_FOOTER);
    webServer.send(200, "text/html", html);
    
    delay(1000);
    wmState = WM_CONNECTING;
    connectStartTime = millis();
  } else {
    webServer.sendHeader("Location", "/", true);
    webServer.send(302, "text/plain", "");
  }
}

// Změna AP hesla
void handleAPPassword() {
  String appass = webServer.arg("appass");
  
  String html = FPSTR(HTML_HEADER);
  
  if (appass.length() >= 8) {
    saveAPPassword(appass.c_str());
    html += "<div class=\"status success\">✓ Heslo AP změněno!</div>";
    html += "<div class=\"card\"><p>Nové heslo bude platit po restartu AP.</p></div>";
  } else {
    html += "<div class=\"status error\">✗ Heslo musí mít min. 8 znaků!</div>";
  }
  
  html += "<a href=\"/\">← Zpět</a>";
  html += FPSTR(HTML_FOOTER);
  webServer.send(200, "text/html", html);
}

// Reset nastavení
void handleReset() {
  clearWiFiCredentials();
  
  String html = FPSTR(HTML_HEADER);
  html += "<div class=\"status info\">Nastavení smazáno</div>";
  html += "<meta http-equiv=\"refresh\" content=\"3;url=/\">";
  html += FPSTR(HTML_FOOTER);
  webServer.send(200, "text/html", html);
}

// Captive portal - přesměrování všech požadavků (pouze pro AP klienty)
void handleNotFound() {
  // Zjistit IP adresu klienta
  IPAddress clientIP = webServer.client().remoteIP();
  
  // Pokud klient přichází z AP sítě (192.168.4.x), přesměrovat na AP IP
  if (clientIP[0] == 192 && clientIP[1] == 168 && clientIP[2] == 4) {
    webServer.sendHeader("Location", "http://" + AP_IP.toString(), true);
    webServer.send(302, "text/plain", "");
  } else {
    // Pro LAN klienty - zobrazit 404 nebo přesměrovat na hlavní stránku
    webServer.sendHeader("Location", "/", true);
    webServer.send(302, "text/plain", "");
  }
}

// Speciální endpointy pro captive portal detekci
void handleCaptivePortal() {
  webServer.sendHeader("Location", "http://" + AP_IP.toString(), true);
  webServer.send(302, "text/plain", "");
}

// Spuštění AP módu
void wifiManagerStartAP() {
  WiFi.mode(WIFI_AP_STA);
  
  const char* apPass = (strlen(credentials.apPassword) >= 8) ? credentials.apPassword : AP_DEFAULT_PASSWORD;
  
  WiFi.softAPConfig(AP_IP, AP_IP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(AP_NAME, apPass);
  
  // DNS pro captive portal
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(DNS_PORT, "*", AP_IP);
  
  // Web server routes
  webServer.on("/", handleRoot);
  webServer.on("/save", HTTP_POST, handleSave);
  webServer.on("/ap", HTTP_POST, handleAPPassword);
  webServer.on("/reset", handleReset);
  
  // Captive portal detection endpoints
  webServer.on("/generate_204", handleCaptivePortal);  // Android
  webServer.on("/fwlink", handleCaptivePortal);        // Microsoft
  webServer.on("/hotspot-detect.html", handleCaptivePortal);  // Apple
  webServer.on("/canonical.html", handleCaptivePortal);
  webServer.on("/success.txt", handleCaptivePortal);
  webServer.on("/ncsi.txt", handleCaptivePortal);
  
  webServer.onNotFound(handleNotFound);
  webServer.begin();
  
  apActive = true;
  wmState = WM_AP_MODE;
  currentIP = AP_IP.toString();
  
  Serial.println(F("AP Started"));
  Serial.print(F("SSID: "));
  Serial.println(AP_NAME);
  Serial.print(F("Password: "));
  Serial.println(apPass);
  Serial.print(F("IP: "));
  Serial.println(currentIP);
}

// Zastavení AP
void wifiManagerStopAP() {
  if (apActive) {
    dnsServer.stop();
    webServer.stop();
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_STA);
    apActive = false;
    Serial.println(F("AP Stopped"));
  }
}

// Pokus o připojení k WiFi
bool tryConnect() {
  if (credentials.ssid[0] == 0) return false;
  
  Serial.print(F("Connecting to: "));
  Serial.println(credentials.ssid);
  
  WiFi.begin(credentials.ssid, credentials.password);
  
  unsigned long start = millis();
  while (millis() - start < CONNECT_TIMEOUT) {
    if (WiFi.status() == WL_CONNECTED) {
      currentIP = WiFi.localIP().toString();
      Serial.print(F("Connected! IP: "));
      Serial.println(currentIP);
      return true;
    }
    delay(100);
    yield();  // Důležité pro ESP8266!
  }
  
  Serial.println(F("Connection failed"));
  return false;
}

// Setup
void wifiManagerSetup() {
  WiFi.persistent(false);  // Neukládat do flash (šetří paměť)
  WiFi.setAutoReconnect(true);
  
  loadWiFiCredentials();
  
  // Pokus o připojení k uložené síti
  if (credentials.ssid[0] != 0) {
    wmState = WM_CONNECTING;
    
    if (tryConnect()) {
      wmState = WM_CONNECTED;
      // I po úspěšném připojení spustit AP + webserver pro konfiguraci
      wifiManagerStartAP();
      return;
    }
  }
  
  // Spustit AP pokud se nepodařilo připojit
  wifiManagerStartAP();
}

// Loop - volat v hlavní smyčce
void wifiManagerLoop() {
  // Obsluha DNS a web serveru v AP módu
  if (apActive) {
    dnsServer.processNextRequest();
    webServer.handleClient();
  }
  
  // Pokud jsme ve stavu CONNECTING, zkusit připojení
  if (wmState == WM_CONNECTING) {
    Serial.println(F("Attempting WiFi connection..."));
    
    // Načíst credentials z EEPROM (mohly být právě uloženy)
    loadWiFiCredentials();
    
    if (tryConnect()) {
      wmState = WM_CONNECTED;
      currentIP = WiFi.localIP().toString();
      Serial.print(F("Connected! IP: "));
      Serial.println(currentIP);
    } else {
      Serial.println(F("Connection failed, staying in AP mode"));
      wmState = WM_AP_MODE;
    }
  }
  
  // Kontrola připojení každých 5 sekund
  static unsigned long lastCheck = 0;
  if (millis() - lastCheck > 5000) {
    lastCheck = millis();
    
    if (WiFi.status() == WL_CONNECTED) {
      if (wmState != WM_CONNECTED) {
        wmState = WM_CONNECTED;
        currentIP = WiFi.localIP().toString();
        Serial.print(F("WiFi connected, IP: "));
        Serial.println(currentIP);
      }
    } else if (wmState == WM_CONNECTED) {
      // Ztraceno připojení
      Serial.println(F("WiFi connection lost, starting AP..."));
      wmState = WM_AP_MODE;
      if (!apActive) {
        wifiManagerStartAP();
      }
    }
  }
  
  yield();  // Důležité pro ESP8266!
}

// Gettery
bool wifiManagerIsConnected() {
  return (WiFi.status() == WL_CONNECTED);
}

WiFiManagerState wifiManagerGetState() {
  return wmState;
}

String wifiManagerGetIP() {
  if (WiFi.status() == WL_CONNECTED) {
    return WiFi.localIP().toString();
  }
  return AP_IP.toString();
}

String wifiManagerGetAPName() {
  return String(AP_NAME);
}
