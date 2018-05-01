#include <SoftwareSerial.h>
#include <Adafruit_Fingerprint.h>
#include <MemoryFree.h>
#include "sha256.h"

#define SWITCH_1 6
#define SWITCH_2 5
#define SWITCH_3 4
#define RX 11 // TX ESP
#define TX 10 // RX ESP
#define AT_SHORT_TIMEOUT 1000
#define AT_MEDIUM_TIMEOUT 3000
#define AT_LONG_TIMEOUT 5000
#define NOT_NEED_RESPONSE 0
#define NEED_RESPONSE 1
#define NORMAL_RETRY 20
#define NO_RETRY 1
#define INFINITE_RETRY 2

SoftwareSerial esp8266(RX,TX); 
SoftwareSerial fingerprintSerial(2, 3); // YELLOW, BLUE
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&fingerprintSerial,00140042);

String SIGNATURE;
String API_RESPONSE;

void setup() {
  Serial.print(F("FreeMemory(setup)="));
  Serial.println(freeMemory());
  
  API_RESPONSE = "";
  pinMode(SWITCH_1, INPUT);
  pinMode(SWITCH_2, INPUT);
  pinMode(SWITCH_3, INPUT);

  Serial.begin(9600);
  while (!Serial);

  // only first time to change esp8266 bauds
  /*esp8266.begin(115200);
  sendCommand("AT",5,"OK");
  sendCommand("AT+UART_DEF=9600,8,1,0,0",5,"OK");
  esp8266.end();*/
  
  finger.begin(57600);
  
  Serial.println(F("\n\nProyecto Tesina"));

  if (finger.verifyPassword()) {
    Serial.println(F("Found fingerprint sensor!"));
  } else {
    Serial.println(F("Did not find fingerprint sensor :("));
    while (1) { 
      delay(1); 
    }
  }

  esp8266.begin(9600);
  esp8266.listen();
  initializeAt();
    
  const String SERIAL_NUMBER = "75330303035351F02081";
  const String AT_MAC_ADDRESS = "de:4f:22:26:3f:3b";
  const String COMPILE_DATE = __DATE__ " " __TIME__;
  const uint8_t * KEY = "MvzLX99WFmrMilNnfqi0V6rt6zVspIxO";
  SIGNATURE.reserve(32);
  SIGNATURE = "";
  String value;
  value.reserve(58);
  value = "";
  Serial.print(F("Compilate date: "));
  Serial.print(COMPILE_DATE);
  Serial.print(F("\n"));

  Sha256.initHmac((uint8_t * ) KEY, 32);
  value = SERIAL_NUMBER + AT_MAC_ADDRESS + COMPILE_DATE;
  Sha256.print(value);
  uint8_t * result = Sha256.resultHmac();
  Serial.print(F("Got: "));
  for (int i = 0; i < 32; i++) {
      SIGNATURE+=("0123456789abcdef"[result[i] >> 4]);
      SIGNATURE+=("0123456789abcdef"[result[i] & 0xf]);
  }
  Serial.print(SIGNATURE);
  Serial.print(F("\n"));
  Serial.print(F("FreeMemory(setup)="));
  Serial.println(freeMemory());
  Serial.println(F("SELECT MODE..."));
}

void loop() {
  int8_t  switch1State = digitalRead(SWITCH_1);
  int8_t  switch2State = digitalRead(SWITCH_2);
  int8_t  switch3State = digitalRead(SWITCH_3);
  int8_t  resp = 0;
  uint8_t  id;

  if (switch1State == HIGH){ // ENROLL
    Serial.print(F("FreeMemory(loop)="));
    Serial.println(freeMemory());
    esp8266.listen();
    resp = httpRequest();
    if (resp != 0){
      Serial.print(F("FreeMemory(loop)="));
      Serial.println(freeMemory());
      Serial.println(F("SELECT MODE..."));
      return;
    }

    Serial.println(F("Ready to enroll a fingerprint!"));
    Serial.println(F("Please type in the ID # (from 1 to 162) you want to save this finger as..."));
    id = readnumber();
    if (id < 1 || id > 162) {
       Serial.print(F("FreeMemory(loop)="));
       Serial.println(freeMemory());
       Serial.println(F("ID not allowed, try again!"));
       return;
    }
    Serial.print(F("Enrolling ID #"));
    Serial.println(id);
    fingerprintSerial.listen();
    resp = getFingerprintEnroll(id);
    
    if (resp !=0){
      Serial.print(F("FreeMemory(loop)="));
      Serial.println(freeMemory());
      Serial.println(F("SELECT MODE..."));
      return;
    }

    esp8266.listen();
    resp = httpRequest();
    if (resp != 0){
      fingerprintSerial.listen();
      deleteFingerprint(id);
      Serial.print(F("FreeMemory(loop)="));
      Serial.println(freeMemory());
      Serial.println(F("SELECT MODE..."));
      return;
    }
    Serial.print(F("FreeMemory(loop)="));
    Serial.println(freeMemory());
    Serial.println(F("SELECT MODE..."));

  }else if (switch2State == HIGH || switch3State == HIGH){ // IDENTIFY IN OR OUT
    Serial.print(F("FreeMemory(loop)="));
    Serial.println(freeMemory());
    Serial.println(F("Waiting for valid finger..."));
    fingerprintSerial.listen();
    resp = getFingerprintIDez();

    if (resp == -1){
      Serial.print(F("FreeMemory(loop)="));
      Serial.println(freeMemory());
      Serial.println(F("SELECT MODE..."));
      return;
    }

    esp8266.listen();
    resp = httpRequest();
    Serial.print(F("FreeMemory(loop)="));
    Serial.println(freeMemory());
    Serial.println(F("SELECT MODE..."));
  }else{}
}
