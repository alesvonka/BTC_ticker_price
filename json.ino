void set_json(String payload)
{
  StaticJsonDocument<100> filter;
  filter[curency]["15m"] = true;

  StaticJsonDocument<100> doc;
  DeserializationError error = deserializeJson(doc, payload, DeserializationOption::Filter(filter));

  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    restartEsp++;
    return;
  }

  JsonObject obj = doc.as<JsonObject>();
  serializeJsonPretty(doc, payload);

  price = obj[curency]["15m"].as<int>() ;
  
  Serial.println(payload);
  Serial.print(curencySymbol);
  Serial.println(price);
}
