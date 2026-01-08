# 🔶 BTC Ticker

ESP8266-based Bitcoin price display with OLED screen, real-time updates, and WiFi configuration portal.

![ESP8266](https://img.shields.io/badge/ESP8266-D1_Mini-blue)
![License](https://img.shields.io/badge/license-MIT-green)
![PlatformIO](https://img.shields.io/badge/PlatformIO-ready-orange)

## ✨ Features

- **Real-time BTC/USD price** from CoinGecko API (updates every 60s)
- **Large digital clock** with automatic time sync
- **WiFi Manager** - no hardcoded credentials, configure via web interface
- **Captive Portal** - automatically opens browser when connecting to AP
- **Auto DST** - Central European Time with automatic summer/winter switching
- **NTP Fallback** - 4 reliable time servers (PTB, Google, Cloudflare, pool.ntp.org)
- **Dual access** - Web interface available on both AP and LAN IP

## 🛠️ Hardware Required

| Component | Description |
|-----------|-------------|
| ESP8266 | WeMos D1 Mini (or compatible) |
| Display | SSD1306 OLED 128x64 I2C |
| Power | USB 5V |

### Wiring

| OLED Pin | ESP8266 Pin |
|----------|-------------|
| VCC | 3.3V |
| GND | GND |
| SCL | D1 (GPIO5) |
| SDA | D2 (GPIO4) |

## 📦 Installation

### PlatformIO (recommended)

```bash
git clone https://github.com/alesvonka/BTC_ticker_price.git
cd BTC_ticker_price
platformio run --target upload
```

### Arduino IDE

1. Install ESP8266 board support
2. Install libraries: Adafruit GFX, Adafruit SSD1306, ArduinoJson, Time
3. Open `src/main.cpp` and upload

## 🚀 First Setup

1. **Power on** the device
2. **Connect to WiFi** `BTC_Ticker` (password: `12345678`)
3. **Browser opens automatically** (or go to `192.168.4.1`)
4. **Select your WiFi** and enter password
5. **Done!** Device connects and shows BTC price

## 📱 Display Layout

```
┌────────────────────────────┐
│ BTC: $ 98543               │  ← Current price
├────────────────────────────┤
│                            │
│      14    25              │  ← Hours : Minutes
│         :                  │
│        32                  │  ← Seconds
│                            │
├────────────────────────────┤
│ IP:192.168.1.42            │  ← Network status
└────────────────────────────┘
```

## ⚙️ Web Interface

Access configuration at:
- **AP Mode:** `192.168.4.1`
- **Connected:** IP shown on display

### Available settings:
- WiFi network selection
- WiFi password
- AP password change
- Reset to defaults

## 🔧 Configuration

Default settings:

| Setting | Value |
|---------|-------|
| AP SSID | `BTC_Ticker` |
| AP Password | `12345678` |
| AP IP | `192.168.4.1` |
| Timezone | CET/CEST (auto DST) |
| Update interval | 60 seconds |

## 📡 API

Uses [CoinGecko API](https://www.coingecko.com/en/api) (free, no API key required):

```
https://api.coingecko.com/api/v3/simple/price?ids=bitcoin&vs_currencies=usd
```

## 🕐 Time Synchronization

NTP servers with automatic fallback:
1. `ptbtime1.ptb.de` - German national metrology institute
2. `time.google.com` - Google Public NTP
3. `time.cloudflare.com` - Cloudflare NTP
4. `pool.ntp.org` - NTP Pool

Automatic DST switching for Central Europe (last Sunday of March/October).

## 📁 Project Structure

```
src/
├── main.cpp          # Main program loop
├── display.cpp/h     # OLED rendering
├── ntp.cpp/h         # Time sync + DST
├── wifi.cpp/h        # HTTPS API client
├── wifimanager.cpp/h # WiFi Manager + portal
├── json.cpp/h        # JSON parsing
└── logo.cpp/h        # Bitcoin logo bitmap
```

## 🐛 Troubleshooting

| Problem | Solution |
|---------|----------|
| No WiFi connection | Check SSID/password, restart device |
| Time not syncing | Check internet connection |
| Display blank | Check I2C wiring (SDA/SCL) |
| Can't access web | Connect to same network or AP |

## 📄 License

MIT License - feel free to use and modify.

## 🙏 Credits

- [CoinGecko](https://www.coingecko.com/) for free API
- [Adafruit](https://www.adafruit.com/) for display libraries
- [PlatformIO](https://platformio.org/) for great tooling