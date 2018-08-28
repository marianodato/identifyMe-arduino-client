#include <Wire.h>
#include <sha256.h>
#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_Fingerprint.h>

#define BUTTON_1 D1
#define BUTTON_2 D2
#define BUTTON_3 D4
#define PIEZO_PIN D8
#define LCD_SCL_PIN D3
#define LCD_SDA_PIN D7
#define FINGERPRINT_TX D5 // FINGERPRINT YELLOW CABLE
#define FINGERPRINT_RX D6 // FINGERPRINT BLUE CABLE

const char * gkSerialNumber = "008BD46B";
const char * gkAtMacAddress = "68:C6:3A:8B:D4:6B";
const char * gkCompileDate = __DATE__ " " __TIME__;

const uint16_t gkBigTimeout = 2000;
const uint16_t gkPiezoTimeout = 100;
const uint16_t gkSmallTimeout = 500;
const uint16_t gkTimeoutWifi = 10000;
const uint16_t gkTimeoutScroll = 1000;
const uint16_t gkDisplayTimeout = 3000;
const uint16_t gkTimeoutFingerprint = 10000;

const int gkFingerprintSignarute = 00140042;
const uint16_t gkSerialTransmissionSpeed = 9600;
const uint16_t gkFingerprintTransmissionSpeed = 57600;

const char * gkWifiSsid = "D200";
const char * gkWifiPassword = "pokemon1234";
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

const uint8_t gkLcdFiles = 2;
const uint8_t gkLcdColumns = 16;
const uint8_t gkLcdAddress = 0x27; // FOR A 16 CHARS AND 2 LINE DISPLAY
const uint8_t gkLcdScrollingSpaces = 1;
const char gkLcdText[] = "1)Carga 2)Entrada 3)Salida";
const uint8_t gkLcdTextSize = sizeof(gkLcdText) - 1;

const uint16_t gkPiezoFreq = 4000;

bool gLoopSetTime = true;
char gLcdBuffer[gkLcdColumns];
unsigned long gLoopTimeout = 0;
int16_t gLcdIndex = -15;

uint16_t gUserId = 0;
String gUsername = "";
String gSignature = "";
String gApiResponse = "";
String gApiResponseStatus = "";
uint8_t gEnrollModeFingerprintId = 0;
uint8_t gIdentifyModeFingerprintId = 0;

LiquidCrystal_I2C lcd(gkLcdAddress, gkLcdColumns, gkLcdFiles);
SoftwareSerial fingerprintSerial(FINGERPRINT_TX, FINGERPRINT_RX);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&fingerprintSerial, gkFingerprintSignarute);

void setup() {
  pinMode(BUTTON_1, INPUT);
  pinMode(BUTTON_2, INPUT);
  pinMode(BUTTON_3, INPUT);

  Wire.begin(LCD_SDA_PIN, LCD_SCL_PIN);
  lcd.begin(gkLcdColumns, gkLcdFiles);
  lcd.init();
  lcd.backlight();
  lcdPrint(3, 2, F("identifyMe"), F("Iniciando..."), false, false);

  Serial.begin(gkSerialTransmissionSpeed);
  while (!Serial);

  finger.begin(gkFingerprintTransmissionSpeed);

  Serial.println(F("\n\nidentifyMe"));
  if (finger.verifyPassword()) {
    Serial.println(F("Found fingerprint sensor!"));
  } else {
    Serial.println(F("Did not find fingerprint sensor!"));
    lcdPrint(5, 2, F("Error"), F("al iniciar!"), false, true);
    while (true) {
      delay(gkSmallTimeout);
    }
  }

  connectToWifi();
  getSignature();
  Serial.println(F("Select mode..."));
  lcdPrint(1, 0, F("Elija un modo:"), F(""), false, true);

  //testScreens();
  return;
}

void loop() {
  int8_t  resp = 0;
  uint8_t  switch1State = digitalRead(BUTTON_1);
  uint8_t  switch2State = digitalRead(BUTTON_2);
  uint8_t  switch3State = digitalRead(BUTTON_3);

  if (gLoopSetTime)
  {
    gLoopTimeout = millis();
    gLoopSetTime = false;
  }

  if (millis() - gLoopTimeout > gkTimeoutScroll)
  {
    lcdScroll();
    gLoopSetTime = true;
  }

  if (switch1State == HIGH) { // ENROLL
    lcdPrint(0, 2, F("Buscando nuevos"), F("usuarios..."), false, false);
    resp = getPendingUser();

    if (resp != 0) {
      Serial.println(F("Error getting pending user!"));
      lcdPrint(1, 1, F("No se encontro"), F("ningun usuario!"), true, true);
      Serial.println(F("Select mode..."));
      lcdPrint(1, 0, F("Elija un modo:"), F(""), false, false);
      return;
    }

    Serial.println(F("Ready to enroll a fingerprint!"));
    Serial.print(F("Enrolling ID #"));
    Serial.println(gEnrollModeFingerprintId);
    resp = getFingerprintEnroll(gEnrollModeFingerprintId);

    if (resp != 0) {
      Serial.println(F("Error enrolling fingerprint!"));
      if (resp > 0) {
        lcdPrint(2, 2, F("La huella ya"), F("fue cargada!"), true, true);
      } else {
        lcdPrint(4, 1, F("Error al"), F("cargar huella!"), true, true);
      }
      Serial.println(F("Select mode..."));
      lcdPrint(1, 0, F("Elija un modo:"), F(""), false, false);
      return;
    }

    lcdPrint(2, 2, F("Configurando"), F("al usuario..."), false, false);
    resp = putEnrolledUser();

    if (resp != 0) {
      Serial.println(F("Error updating fingerprint status!"));
      lcdPrint(4, 3, F("Error al"), F("configurar!"), true, true);
      Serial.println(F("Deleting ID #"));
      Serial.println(gEnrollModeFingerprintId);
      deleteFingerprint(gEnrollModeFingerprintId);
      gEnrollModeFingerprintId = 0;
      Serial.println(F("Select mode..."));
      lcdPrint(1, 0, F("Elija un modo:"), F(""), false, false);
      return;
    }

    Serial.print(F("Username enrolled: "));
    Serial.print((gUsername));
    lcdPrintVariable(0, 0, F("Huella cargada a"), gUsername, true, true);
    gUsername = "";
    Serial.println();
    Serial.println(F("Select mode..."));
    lcdPrint(1, 0, F("Elija un modo:"), F(""), false, false);

  } else if (switch2State == HIGH || switch3State == HIGH) { // IDENTIFY IN OR OUT
    Serial.println(F("Waiting for valid finger..."));
    resp = getFingerprintIDez();

    if (resp <= 0) {
      Serial.println(F("Error identifying fingerprint!"));
      lcdPrint(0, 3, F("No se reconocio"), F("la huella!"), true, true);
      gIdentifyModeFingerprintId = 0;
      Serial.println(F("Select mode..."));
      lcdPrint(1, 0, F("Elija un modo:"), F(""), false, false);
      return;
    }

    if (switch2State == HIGH) {
      lcdPrint(3, 3, F("Registrando"), F("entrada..."), false, false);
      resp = postUserRegistrationRecord();

      if (resp != 0) {
        Serial.println(F("Error posting entry record!"));
        lcdPrint(1, 3, F("Error al crear"), F("registro!"), true, true);
        Serial.println(F("Select mode..."));
        lcdPrint(1, 0, F("Elija un modo:"), F(""), false, false);
        return;
      }

      Serial.print(F("Username identified: "));
      Serial.print((gUsername));
      lcdPrintVariable(3, 0, F("Bienvenido,"), gUsername, true, true);

    } else {
      lcdPrint(3, 3, F("Registrando"), F("salida..."), false, false);
      resp = putUserRegistrationRecord();

      if (resp != 0) {
        Serial.println(F("Error posting exit record!"));
        lcdPrint(1, 3, F("Error al crear"), F("registro!"), true, true);
        Serial.println(F("Select mode..."));
        lcdPrint(1, 0, F("Elija un modo:"), F(""), false, false);
        return;
      }

      Serial.print(F("Username identified: "));
      Serial.print((gUsername));
      lcdPrintVariable(2, 0, F("Hasta luego,"), gUsername, true, true);
    }

    gUsername = "";
    Serial.println();
    Serial.println(F("Select mode..."));
    lcdPrint(1, 0, F("Elija un modo:"), F(""), false, false);
  } else {}
}
