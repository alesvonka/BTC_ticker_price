void eepromFirstStartInicialize()
{
  int first = EEPROM.read(0);

  if (first == 255) {
    Serial.print("First start after writing .ino file - EEPROM.read(0): ");
    Serial.println(first);
    Serial.print("SET EEPROM address 0, 255 on : ");
    EEPROM.write(0, 1);
    delay(100);
    Serial.println(EEPROM.read(0));

    eepromPut("Your SSID", "");
  }
  eepromGet();
}

void eepromPut(String eepromSSID, String eepromPSWD)
{
  lWo = {eepromSSID, eepromPSWD};
  EEPROM.put(eeAddress, lWo);
  delay(100);
  EEPROM.commit();
  delay(100);
}

void eepromGet()
{
  EEPROM.get(eeAddress, lWo);
  wssid = lWo.ssid;
  wpswd = lWo.pswd;

  Serial.print("EEPROM SSID: ");
  Serial.println(wssid);
  Serial.print("EEPROM PSWD: ");
  Serial.println(wpswd);
}
