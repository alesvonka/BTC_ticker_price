# BTC Ticker - Project Context for AI Assistant

## 📋 Project Overview
**BTC Ticker** is an ESP8266-based Bitcoin price display with OLED screen, featuring real-time price updates, NTP time synchronization with automatic DST, and a WiFi Manager with captive portal.

## 🛠️ Hardware
- **MCU:** ESP8266 (WeMos D1 Mini) - single core, 80MHz, 80KB RAM
- **Display:** SSD1306 OLED 128x64 I2C
- **Power:** USB 5V

## 📁 File Structure
```
src/
├── main.cpp          # Main loop, setup, global variables
├── display.cpp/h     # OLED display rendering (price, clock, IP/AP status)
├── ntp.cpp/h         # NTP time sync with 4-server fallback + CET/CEST DST
├── wifi.cpp/h        # HTTPS client for CoinGecko API
├── wifimanager.cpp/h # WiFi Manager: AP mode, captive portal, EEPROM storage
├── json.cpp/h        # ArduinoJson parsing for API responses
└── logo.cpp/h        # Bitcoin logo bitmap
```

## ⚠️ Important Constraints (ESP8266)
1. **Single core** - always use `yield()` in long loops
2. **Limited RAM (80KB)** - use `F()` macro for strings, PROGMEM for large data
3. **No HTTPS cert validation** - using `setInsecure()` for BearSSL
4. **WiFi modes:** WIFI_STA, WIFI_AP, WIFI_AP_STA (currently using AP_STA)

## 🔧 Key Features Implemented
- **WiFi Manager:** Auto AP mode if no WiFi, captive portal, EEPROM credentials storage
- **NTP Fallback:** ptbtime1.ptb.de → time.google.com → time.cloudflare.com → pool.ntp.org
- **Auto DST:** Central European Time (CET/CEST) with last Sunday March/October rules
- **Web Interface:** Available on both AP IP (192.168.4.1) and LAN IP

## 📡 API
- **Endpoint:** `https://api.coingecko.com/api/v3/simple/price?ids=bitcoin&vs_currencies=usd&precision=2`
- **Update interval:** 60 seconds
- **Response format:** `{"bitcoin":{"usd":12345.67}}`

## 🎨 Display Layout (128x64)
```
Row 0-11:  BTC ticker (price) + separator line at Y=12
Row 12-53: Large clock (hours:minutes, seconds small)
Row 54:    Separator line
Row 55-63: Status line (IP: xxx.xxx.xxx.xxx or AP: BTC_Ticker)
```

## 🔑 Default Configuration
- **AP SSID:** BTC_Ticker
- **AP Password:** 12345678
- **AP IP:** 192.168.4.1
- **Timezone:** CET/CEST (UTC+1/+2)

## 📦 Dependencies (platformio.ini)
- Adafruit GFX Library
- Adafruit SSD1306
- ArduinoJson
- TimeLib
- ESP8266WiFi, ESP8266WebServer, ESP8266HTTPClient
- DNSServer, EEPROM

## 🚫 Common Pitfalls
1. `const` variables need `extern` for cross-file linkage in C++
2. EEPROM needs `begin()`, `put()`, `commit()`, `end()` sequence
3. WebServer handlers must be registered before `begin()`
4. DNS server needed for captive portal to work properly
5. `WiFi.persistent(false)` saves flash wear

## 📝 Coding Style
- Comments in Czech or English
- Use `Serial.print(F("..."))` for debug strings
- Prefix private/static vars appropriately
- Test on real hardware before committing
