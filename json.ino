void set_json(String payload)
{
  StaticJsonDocument<100> filter;
  filter[curency]["15m"] = true;
  //filter[curency]["symbol"] = true;

  StaticJsonDocument<100> doc;
  deserializeJson(doc, payload, DeserializationOption::Filter(filter));

  JsonObject obj = doc.as<JsonObject>();

  //serializeJsonPretty(doc, payload);

  //curencySymbol     = obj[curency]["symbol"].as<char*>();
  price             = obj[curency]["15m"].as<int>() ;

  Serial.print(curencySymbol);
  Serial.println(price);

}
