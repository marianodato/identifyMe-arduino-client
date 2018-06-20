#include <sha256.h>
#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
#include <Adafruit_Fingerprint.h>

#define BUTTON_1 D1
#define BUTTON_2 D2
#define BUTTON_3 D4
#define FINGERPRINT_TX D5 // FINGERPRINT YELLOW CABLE
#define FINGERPRINT_RX D6 // FINGERPRINT BLUE CABLE

uint16_t gUserId = 0;
String gUsername = "";
String gSignature = "";
String gApiResponse = "";
String gApiResponseStatus = "";
uint8_t gEnrollModeFingerprintId = 0;
uint8_t gIdentifyModeFingerprintId = 0;

const char * gkSerialNumber = "008BD46B";
const char * gkAtMacAddress = "68:C6:3A:8B:D4:6B";
const char * gkCompileDate = __DATE__ " " __TIME__;

const uint16_t gkBigTimeout = 2000;
const uint16_t gkSmallTimeout = 500;
const uint16_t gkTimeoutWifi = 10000;
const uint16_t gkTimeoutFingerprint = 10000;

const int gkFingerprintSignarute = 00140042;
const uint16_t gkSerialTransmissionSpeed = 9600;
const uint16_t gkFingerprintTransmissionSpeed = 57600;

const char * gkWifiSsid = "TH14";
const char * gkWifiPassword = "TheInvincibles26W12D0L";
const char * gkSignatureKey = "MvzLX99WFmrMilNnfqi0V6rt6zVspIxO";

const uint16_t gkHttpsPort = 443;
const char * gkHttpGetMethod = "GET ";
const char * gkHttpPutMethod = "PUT ";
const char * gkHttpPostMethod = "POST ";
const char * gkHttpPostStatusOk = "201";
const char * gkHttpUserAgent = "NodeMCU";
const char * gkHttpProtocol = "HTTP/1.0";
const char * gkHttpGetPutStatusOk = "200";
const char * gkHttpsHost = "identifyme-backend-api.herokuapp.com";
const char * gkHttpsHostFingerprint = "08 3B 71 72 02 43 6E CA ED 42 86 93 BA 7E DF 81 C4 BC 62 30"; // SHA1

SoftwareSerial fingerprintSerial(FINGERPRINT_TX, FINGERPRINT_RX);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&fingerprintSerial, gkFingerprintSignarute);

void setup() {
  pinMode(BUTTON_1, INPUT);
  pinMode(BUTTON_2, INPUT);
  pinMode(BUTTON_3, INPUT);

  Serial.begin(gkSerialTransmissionSpeed);
  while (!Serial);

  finger.begin(gkFingerprintTransmissionSpeed);

  Serial.println(F("\n\nFINAL PROJECT"));
  if (finger.verifyPassword()) {
    Serial.println(F("Found fingerprint sensor!"));
  } else {
    Serial.println(F("Did not find fingerprint sensor :("));
    while (true) {
      delay(gkSmallTimeout);
    }
  }

  connectToWifi();
  getSignature();
  Serial.println(F("SELECT MODE..."));
}

void loop() {
  int8_t  resp = 0;
  uint8_t  switch1State = digitalRead(BUTTON_1);
  uint8_t  switch2State = digitalRead(BUTTON_2);
  uint8_t  switch3State = digitalRead(BUTTON_3);

  if (switch1State == HIGH) { // ENROLL
    resp = getPendingUser();
    if (resp != 0) {
      Serial.println(F("Error getting pending user"));
      Serial.println(F("SELECT MODE..."));
      return;
    }

    Serial.println(F("Ready to enroll a fingerprint!"));
    Serial.print(F("Enrolling ID #"));
    Serial.println(gEnrollModeFingerprintId);
    resp = getFingerprintEnroll(gEnrollModeFingerprintId);

    if (resp != 0) {
      Serial.println(F("Error enrolling fingerprint"));
      Serial.println(F("SELECT MODE..."));
      return;
    }

    resp = putEnrolledUser();
    if (resp != 0) {
      Serial.println(F("Error updating fingerprint status"));
      Serial.println(F("Deleting ID #"));
      Serial.println(gEnrollModeFingerprintId);
      deleteFingerprint(gEnrollModeFingerprintId);
      gEnrollModeFingerprintId = 0;
      Serial.println(F("SELECT MODE..."));
      return;
    }

    Serial.print(F("Username enrolled: "));
    Serial.print((gUsername));
    gUsername = "";
    Serial.println();
    Serial.println(F("SELECT MODE..."));

  } else if (switch2State == HIGH || switch3State == HIGH) { // IDENTIFY IN OR OUT
    Serial.println(F("Waiting for valid finger..."));
    resp = getFingerprintIDez();

    if (resp <= 0) {
      Serial.println(F("Error identifying fingerprint"));
      gIdentifyModeFingerprintId = 0;
      Serial.println(F("SELECT MODE..."));
      return;
    }

    if (switch2State == HIGH) {
      resp = postUserRegistrationRecord();
      if (resp != 0) {
        Serial.println(F("Error posting entry record"));
        Serial.println(F("SELECT MODE..."));
        return;
      }
    } else {
      resp = putUserRegistrationRecord();
      if (resp != 0) {
        Serial.println(F("Error posting exit record"));
        Serial.println(F("SELECT MODE..."));
        return;
      }
    }

    Serial.print(F("Username enrolled: "));
    Serial.print((gUsername));
    gUsername = "";
    Serial.println();
    Serial.println(F("SELECT MODE..."));
  } else {}
}
