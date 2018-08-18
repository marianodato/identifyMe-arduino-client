String getCommonBody() {
  String body = "";
  body = "{\"serialNumber\": \"";
  body += gkSerialNumber;
  body += "\", \"atMacAddress\": \"";
  body += gkAtMacAddress;
  body += "\", \"compileDate\": \"";
  body += gkCompileDate;
  body += "\", \"signature\": \"";
  body += gSignature;
  return body;
}

uint16_t parseResponseInt(String field) {
  char c;
  uint16_t i = 0;
  String temp = "";
  uint16_t index = 0;
  uint16_t  resp = 0;

  index = gApiResponse.indexOf(field);

  if (index < 0) {
    return -1;
  }

  for (i = (index + field.length()); ; i++) {
    c = gApiResponse[i];
    if (c == ',') {
      break;
    }
    temp += c;
  }

  resp = temp.toInt();

  if (resp == 0) {
    return -1;
  }

  return resp;
}

String parseResponseString(String field) {
  char c;
  uint16_t i = 0;
  String temp = "";
  uint16_t index = 0;

  index = gApiResponse.indexOf(field);

  if (index < 0) {
    return "";
  }

  for (i = (index + field.length()); ; i++) {
    c = gApiResponse[i];
    if (c == '"') {
      break;
    }
    temp += c;
  }
  return temp;
}

int8_t getPendingUser() {
  String temp = "";
  uint16_t  resp = 0;
  String body = getCommonBody();
  body += "\"}";
  String httpUri = "/users?fingerprintStatus=pending";

  gUserId = 0;
  gUsername = "";
  gEnrollModeFingerprintId = 0;

  resp = httpsRequest(gkHttpGetMethod, httpUri, body);
  if (resp != 0) {
    return resp;
  }

  if (gApiResponseStatus.indexOf(gkHttpGetPutStatusOk) < 0) {
    Serial.println(F("Bad Response Status!"));
    gApiResponse = "";
    gApiResponseStatus = "";
    return -1;
  }

  resp = parseResponseInt("\"id\":");

  Serial.print(F("UserId: "));
  Serial.println(resp);

  if (resp == -1) {
    gUserId = 0;
    gApiResponse = "";
    gApiResponseStatus = "";
    Serial.println(F("Cannot parse field: id!"));
    return -1;
  }

  gUserId = resp;

  resp = parseResponseInt("\"fingerprintId\":");

  Serial.print(F("FingerprintId: "));
  Serial.println(resp);

  if (resp == -1) {
    gUserId = 0;
    gApiResponse = "";
    gApiResponseStatus = "";
    gEnrollModeFingerprintId = 0;
    Serial.println(F("Cannot parse field: fingerprintId!"));
    return -1;
  }

  gEnrollModeFingerprintId = resp;

  temp = parseResponseString("\"username\":\"");

  Serial.print(F("Username: "));
  Serial.println(temp);

  if (temp == "") {
    gUserId = 0;
    gUsername = "";
    gApiResponse = "";
    gApiResponseStatus = "";
    gEnrollModeFingerprintId = 0;
    Serial.println(F("Cannot parse field: username!"));
    return -1;
  }

  gUsername = temp;

  gApiResponse = "";
  gApiResponseStatus = "";
  return 0;
}

int8_t putEnrolledUser() {
  int8_t  resp = 0;
  String httpUri = "/users/";
  httpUri += gUserId;
  String body = getCommonBody();
  body += "\", \"fingerprintStatus\": \"enrolled";
  body += "\"}";

  resp = httpsRequest(gkHttpPutMethod, httpUri, body);
  if (resp != 0) {
    gUserId = 0;
    gUsername = "";
    return resp;
  }

  if (gApiResponseStatus.indexOf(gkHttpGetPutStatusOk) < 0) {
    Serial.println(F("Bad Response Status!"));
    gUserId = 0;
    gUsername = "";
    gApiResponse = "";
    gApiResponseStatus = "";
    return -1;
  }

  gUserId = 0;
  gApiResponse = "";
  gApiResponseStatus = "";
  gEnrollModeFingerprintId = 0;
  return 0;
}

int8_t postUserRegistrationRecord() {
  String temp = "";
  int8_t  resp = 0;
  String body = getCommonBody();
  body += "\", \"fingerprintId\": ";
  body += gIdentifyModeFingerprintId;
  body += "}";
  String httpUri = "/users/registration/records";

  gUsername = "";

  resp = httpsRequest(gkHttpPostMethod, httpUri, body);
  if (resp != 0) {
    gIdentifyModeFingerprintId = 0;
    return resp;
  }

  if (gApiResponseStatus.indexOf(gkHttpPostStatusOk) < 0) {
    Serial.println(F("Bad Response Status!"));
    gIdentifyModeFingerprintId = 0;
    return -1;
  }

  temp = parseResponseString("\"userUsername\":\"");

  Serial.print(F("UserUsername: "));
  Serial.println(temp);

  if (temp == "") {
    gIdentifyModeFingerprintId = 0;
    gUsername = "";
    Serial.println(F("Cannot parse field: userUsername!"));
    return -1;
  }

  gUsername = temp;

  gIdentifyModeFingerprintId = 0;
  return 0;
}

int8_t putUserRegistrationRecord() {
  String temp = "";
  int8_t  resp = 0;
  String body = getCommonBody();
  body += "\"}";
  String httpUri = "/users/registration/records/";
  httpUri += gIdentifyModeFingerprintId;

  gUsername = "";

  resp = httpsRequest(gkHttpPutMethod, httpUri, body);
  if (resp != 0) {
    gIdentifyModeFingerprintId = 0;
    return resp;
  }

  if (gApiResponseStatus.indexOf(gkHttpGetPutStatusOk) < 0) {
    Serial.println(F("Bad Response Status!"));
    gIdentifyModeFingerprintId = 0;
    return -1;
  }

  temp = parseResponseString("\"userUsername\":\"");

  Serial.print(F("UserUsername: "));
  Serial.println(temp);

  if (temp == "") {
    gIdentifyModeFingerprintId = 0;
    gUsername = "";
    Serial.println(F("Cannot parse field: userUsername!"));
    return -1;
  }

  gUsername = temp;

  gIdentifyModeFingerprintId = 0;
  return 0;
}

int8_t  httpsRequest(const char * httpMethod, String httpUri, String body) {
  char c;
  bool first = true;
  unsigned long timeout;
  bool foundBlankLine = false;

  gApiResponse = "";
  gApiResponseStatus = "";

  Serial.print(F("Connecting to "));
  Serial.println(gkHttpsHost);

  WiFiClientSecure client;
  if (!client.connect(gkHttpsHost, gkHttpsPort)) {
    Serial.println(F("Error connecting to host!"));
    connectToWifi();
    return -1;
  }

  if (client.verify(gkHttpsHostFingerprint, gkHttpsHost)) {
    Serial.println(F("The certificate is valid!"));
  } else {
    Serial.println(F("The certificate is invalid!"));
    client.stop();
    return -1;
  }

  Serial.print(F("URL of request: https://"));
  Serial.print(gkHttpsHost);
  Serial.print(F(":"));
  Serial.print(gkHttpsPort);
  Serial.println(httpUri);
  Serial.print(F("Body: "));
  Serial.println(body);

  client.print(String(httpMethod) + httpUri + " " + gkHttpProtocol + "\r\n" +
               "Host: " + gkHttpsHost + "\r\n" +
               "User-Agent: " + gkHttpUserAgent + "\r\n" +
               "Content-length: " + body.length() + "\r\n\r\n" +
               body);

  timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > gkTimeoutWifi) {
      Serial.println(F("Exceeded the waiting time!"));
      client.stop();
      return -1;
    }
  }

  // HTTPS NEEDS 20 KB OF MEMORY
  Serial.println(F("\nFree memory on NodeMCU:"));
  Serial.print(ESP.getFreeHeap());
  Serial.print(F("Bytes\n\n"));

  while (client.available()) {
    c = client.read();
    Serial.print(c);
    gApiResponse += c;
    if (c == '\n') {
      if (first) {
        gApiResponseStatus = gApiResponse;
        first = false;
      }
      if (gApiResponse.equals("\r\n")) {
        gApiResponse = "";
        foundBlankLine = true;
      } else {
        if (foundBlankLine == true) {
        } else {
          gApiResponse = "";
        }
      }
    }
  }

  Serial.println();
  Serial.println();
  Serial.println(F("Closing connection..."));

  Serial.println();
  Serial.println(F("Response Status: "));
  Serial.println(gApiResponseStatus);

  Serial.println(F("Response: "));
  Serial.println(gApiResponse);
  Serial.println();
  return 0;
}

void connectToWifi() {
  Serial.println();
  Serial.print(F("Connecting to wifi: "));
  Serial.println(gkWifiSsid);

  WiFi.mode(WIFI_STA); // CLIENT MODE
  WiFi.begin(gkWifiSsid, gkWifiPassword);

  while (WiFi.status() != WL_CONNECTED) {
    delay(gkSmallTimeout);
    Serial.print(".");
  }

  Serial.println();
  Serial.println(F("WiFi connected!"));
  Serial.print(F("IP address: "));
  Serial.print(WiFi.localIP());
  return;
}
