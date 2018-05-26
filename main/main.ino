#include <SoftwareSerial.h>
#include <ESP8266WiFi.h> 
#include <Adafruit_Fingerprint.h>
#include "sha256.h"

#define SWITCH_1 D1
#define SWITCH_2 D2
#define SWITCH_3 D4
#define TIMEOUT_WIFI 10000
#define TIMEOUT_FINGER 10000

SoftwareSerial fingerprintSerial(D5, D6); // YELLOW CABLE, BLUE CABLE
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&fingerprintSerial,00140042);

String SIGNATURE;
String API_RESPONSE;
const char * SERIAL_NUMBER = "008BD46B";
const char * AT_MAC_ADDRESS = "68:C6:3A:8B:D4:6B";
const char * COMPILE_DATE = __DATE__ " " __TIME__;

void setup() {
  API_RESPONSE = "";
  pinMode(SWITCH_1, INPUT);
  pinMode(SWITCH_2, INPUT);
  pinMode(SWITCH_3, INPUT);

  Serial.begin(9600);
  while (!Serial);
  finger.begin(57600);
  
  Serial.println(F("\n\nFINAL PROJECT"));
  if (finger.verifyPassword()) {
    Serial.println(F("Found fingerprint sensor!"));
  } else {
    Serial.println(F("Did not find fingerprint sensor :("));
    while (1) { 
      delay(1); 
    }
  }

  connectToWifi();
  getSignature();
  Serial.println(F("SELECT MODE..."));
}

void loop() {
  int8_t  switch1State = digitalRead(SWITCH_1);
  int8_t  switch2State = digitalRead(SWITCH_2);
  int8_t  switch3State = digitalRead(SWITCH_3);
  int8_t  resp = 0;
  uint8_t  id;

  if (switch1State == HIGH){ // ENROLL
    resp = httpRequest();
    if (resp != 0){
      Serial.println(F("SELECT MODE..."));
      return;
    }

    Serial.println(F("Ready to enroll a fingerprint!"));
    Serial.println(F("Please type in the ID # (from 1 to 162) you want to save this finger as..."));
    id = readnumber();
    if (id < 1 || id > 162) {
       Serial.println(F("ID not allowed, try again!"));
       return;
    }
    Serial.print(F("Enrolling ID #"));
    Serial.println(id);
    resp = getFingerprintEnroll(id);
    
    if (resp !=0){
      Serial.println(F("SELECT MODE..."));
      return;
    }

    resp = httpRequest();
    if (resp != 0){
      deleteFingerprint(id);
      Serial.println(F("SELECT MODE..."));
      return;
    }
    Serial.println(F("SELECT MODE..."));

  }else if (switch2State == HIGH || switch3State == HIGH){ // IDENTIFY IN OR OUT
    Serial.println(F("Waiting for valid finger..."));
    resp = getFingerprintIDez();

    if (resp <= 0){
      Serial.println(F("SELECT MODE..."));
      return;
    }

    resp = httpRequest();
    Serial.println(F("SELECT MODE..."));
  }else{}
}
