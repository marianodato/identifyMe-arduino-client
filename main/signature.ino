void getSignature(){
  const String serial_number = "75330303035351F02081";
  const String at_mac_address = "de:4f:22:26:3f:3b";
  const String compile_date = __DATE__ " " __TIME__;
  const char * key = "MvzLX99WFmrMilNnfqi0V6rt6zVspIxO";
  String value;
  uint8_t * result;
  int i = 0;
  SIGNATURE.reserve(32);
  SIGNATURE = "";
  value.reserve(58);
  value = "";
  Serial.print(F("Compile date: "));
  Serial.print(compile_date);
  Serial.print(F("\n"));

  Sha256.initHmac((uint8_t * ) key, 32);
  value = serial_number + at_mac_address + compile_date;
  Sha256.print(value);
  result = Sha256.resultHmac();
  Serial.print(F("Got: "));
  for (i = 0; i < 32; i++) {
      SIGNATURE+=("0123456789abcdef"[result[i] >> 4]);
      SIGNATURE+=("0123456789abcdef"[result[i] & 0xf]);
  }
  Serial.print(SIGNATURE);
  Serial.print(F("\n"));
  return;
}
