void getSignature() {
  uint8_t i = 0;
  uint8_t * hash;
  String value = "";
  const uint8_t kKeySize = 32;
  const char * kHexaChars = "0123456789abcdef";

  gSignature = "";

  Serial.print(F("\n"));
  Serial.print(F("Compile date: "));
  Serial.print(gkCompileDate);
  Serial.print(F("\n"));

  Sha256.initHmac((uint8_t * ) gkSignatureKey, kKeySize);
  value = String(gkSerialNumber) + gkAtMacAddress + gkCompileDate;
  Sha256.print(value);
  hash = Sha256.resultHmac();
  Serial.print(F("Got: "));
  for (i = 0; i < kKeySize; i++) {
    gSignature += (kHexaChars[hash[i] >> 4]);
    gSignature += (kHexaChars[hash[i] & 0xf]);
  }
  Serial.print(gSignature);
  Serial.print(F("\n"));
  return;
}
