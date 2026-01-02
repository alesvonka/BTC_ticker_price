#include "json.h"
#include <ArduinoJson.h>

extern String payload;
extern double price;
extern String curencySymbol;

void set_json(String payload)
{
  JsonDocument filter;
  filter["bitcoin"]["usd"] = true;

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, payload, DeserializationOption::Filter(filter));

  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }

  JsonObject obj = doc.as<JsonObject>();
  serializeJsonPretty(doc, payload);

  price = obj["bitcoin"]["usd"].as<double>();
  
  Serial.println(payload);
  Serial.print(curencySymbol);
  Serial.println(price);
}