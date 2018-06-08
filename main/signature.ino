void getSignature() {
  uint8_t i = 0;
  uint8_t * result;
  String value = "";
  const char * key = "MvzLX99WFmrMilNnfqi0V6rt6zVspIxO";

  SIGNATURE = "";

  Serial.print(F("\n"));
  Serial.print(F("Compile date: "));
  Serial.print(COMPILE_DATE);
  Serial.print(F("\n"));

  Sha256.initHmac((uint8_t * ) key, 32);
  value = String(SERIAL_NUMBER) + AT_MAC_ADDRESS + COMPILE_DATE;
  Sha256.print(value);
  result = Sha256.resultHmac();
  Serial.print(F("Got: "));
  for (i = 0; i < 32; i++) {
    SIGNATURE += ("0123456789abcdef"[result[i] >> 4]);
    SIGNATURE += ("0123456789abcdef"[result[i] & 0xf]);
  }
  Serial.print(SIGNATURE);
  Serial.print(F("\n"));
  return;
}
