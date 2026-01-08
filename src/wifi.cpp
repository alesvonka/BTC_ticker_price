#include "wifi.h"
#include "json.h"
#include "wifimanager.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>

extern String http_adress;
extern String payload;

void connect_wifi()
{
  // Zkontrolovat připojení přes WiFi Manager
  if (!wifiManagerIsConnected()) {
    Serial.println(F("WiFi not connected"));
    return;
  }
  
  std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
  client->setInsecure();

  HTTPClient https;

  Serial.println(F("[HTTPS] begin..."));
  if (https.begin(*client, http_adress)) {

    Serial.println(F("[HTTPS] GET..."));
    int httpCode = https.GET();

    if (httpCode > 0) {
      Serial.print(F("[HTTPS] code: "));
      Serial.println(httpCode);

      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
        payload = https.getString();
        set_json(payload);
      }
    } else {
      Serial.print(F("[HTTPS] error: "));
      Serial.println(https.errorToString(httpCode));
    }

    https.end();
  } else {
    Serial.println(F("[HTTPS] Unable to connect"));
  }
}