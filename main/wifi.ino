int8_t parseResponseInt(String field){
  int8_t i = 0;
  int8_t index = 0;
  String temp = "";
  char c;
  int8_t  resp = 0;
  
  index = API_RESPONSE.indexOf(field);

  if (index < 0){
    return -1;
  }

  for (i=index+(field.length()-1); i++; ){
    c = API_RESPONSE[i];
    if (c == ','){
      break;
    }
    temp += c;
  }

  resp = temp.toInt();

  if (resp == 0){
    return -1;
  }
  
  return resp;
}

String parseResponseString(String field){
  int8_t i = 0;
  int8_t index = 0;
  String temp = "";
  char c;
  int8_t  resp = 0;

  index = API_RESPONSE.indexOf(field);

  if (index < 0){
    return "";
  }

  for (i=index+(field.length()); i++; ){
    c = API_RESPONSE[i];
    if (c == '\"'){
      break;
    }
    temp += c;
  }
  return temp;
}

int8_t getPendingUser(){
  String temp = "";
  int8_t  resp = 0;
  char * httpMethod = "GET ";
  char * httpUri = "/users?fingerprintStatus=pending";
  String body = getCommonBody();
  body += "\"}";
  ID = 0;
  FINGERPRINT_ID = 0;
  NAME = "";
  
  resp = httpsRequest(httpMethod, httpUri, body);
  if (resp != 0){
      return resp;
  }

  if (API_RESPONSE_STATUS.indexOf("200") < 0){
    Serial.println(F("Bad Response Status"));
    API_RESPONSE_STATUS = "";
    API_RESPONSE = "";
    return -1;
  }

  resp = parseResponseInt("\"id\":");

  Serial.print(F("Id: "));
  Serial.println(resp);

  if (resp == -1){
    ID = 0;
    API_RESPONSE_STATUS = "";
    API_RESPONSE = "";
    Serial.println(F("Cannot parse field: id!"));
    return -1;
  }

  ID = resp;

  resp = parseResponseInt("\"fingerprintId\":");

  Serial.print(F("FingerprintId: "));
  Serial.println(resp);

  if (resp == -1){
    ID = 0;
    FINGERPRINT_ID = 0;
    API_RESPONSE_STATUS = "";
    API_RESPONSE = "";
    Serial.println(F("Cannot parse field: fingerprintId!"));
    return -1;
  }

  FINGERPRINT_ID = resp;

  temp = parseResponseString("\"name\":");

  Serial.print(F("Name: "));
  Serial.println(temp);

  if (temp == ""){
    ID = 0;
    FINGERPRINT_ID = 0;
    NAME = "";
    API_RESPONSE_STATUS = "";
    API_RESPONSE = "";
    Serial.println(F("Cannot parse field: name!"));
    return -1;
  }

  NAME = temp;

  API_RESPONSE_STATUS = "";
  API_RESPONSE = "";
  return 0;
}

String getCommonBody(){
  String body;
  body = "{\"serialNumber\": \"";
  body += SERIAL_NUMBER;
  body += "\", \"atMacAddress\": \"";
  body += AT_MAC_ADDRESS;
  body += "\", \"compileDate\": \"";
  body += COMPILE_DATE;
  body += "\", \"signature\": \"";
  body += SIGNATURE;
  return body;
}

int8_t  httpsRequest(char * httpMethod, char * httpUri, String body){
  const char * host = "identifyme-backend-api.herokuapp.com";
  const int port = 443;
  const char * fingerprint = "08 3B 71 72 02 43 6E CA ED 42 86 93 BA 7E DF 81 C4 BC 62 30"; // SHA1
  bool foundBlankLine = false;
  unsigned long timeout;
  char c;
  bool first = true;
  API_RESPONSE = "";
  API_RESPONSE_STATUS = "";
  
  Serial.print(F("Connecting to "));
  Serial.println(host);
 
  WiFiClientSecure client;
  if (!client.connect(host, port)) {
    Serial.println(F("Error connecting to host!"));
    connectToWifi();
    return -1;
  }
 
  if(client.verify(fingerprint,host)){
    Serial.println(F("The certificate is valid!")); 
  } else {
    Serial.println(F("The certificate is invalid!")); 
    client.stop();
    return -1;
  }
 
  Serial.print(F("URL of request: https://"));
  Serial.print(host);
  Serial.print(F(":"));
  Serial.print(port);
  Serial.println(httpUri);
  Serial.print(F("Body: "));
  Serial.println(body);
 
  client.print(String(httpMethod) + httpUri + " HTTP/1.0\r\n" +
                "Host: " + host + "\r\n" +
                "User-Agent: NodeMCU\r\n" +
                "Content-length: " + body.length() + "\r\n\r\n" + 
                body);
  timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > TIMEOUT_WIFI) {
      Serial.println(F("Exceeded the waiting time!"));
      client.stop();
      return -1;
    }
  }
 
  // HTTPS NEEDS 20 KB OF MEMORY
  Serial.println(F("\nFree memory on NodeMCU:")); 
  Serial.print(ESP.getFreeHeap());
  Serial.print(F("Bytes\n\n"));
 
  while(client.available()){
    c = client.read();
    Serial.print(c);
    API_RESPONSE += c;
    if (c == '\n'){
      if(first){
         API_RESPONSE_STATUS = API_RESPONSE;
         first = false;  
      }
      if (API_RESPONSE.equals("\r\n")){
        API_RESPONSE = "";
        foundBlankLine = true;
      }else{
        if (foundBlankLine == true){  
        }else{
          API_RESPONSE = "";
        }  
      }
    }
  }
 
  Serial.println();
  Serial.println();
  Serial.println(F("Closing connection"));

  Serial.println();
  Serial.println(F("Response Status: "));
  Serial.println(API_RESPONSE_STATUS);
  
  Serial.println(F("Response: "));
  Serial.println(API_RESPONSE);
  Serial.println();
  return 0;
}

void connectToWifi(){
  const char * ssid = "TH14";
  const char * password = "TheInvincibles26W12D0L";
  
  Serial.println();
  Serial.println();
  Serial.print(F("Connecting to wifi: "));
  Serial.println(ssid);
 
  WiFi.mode(WIFI_STA); // CLIENT MODE
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
 
  Serial.println("");
  Serial.println(F("WiFi connected")); 
  Serial.println(F("IP address: "));
  Serial.println(WiFi.localIP());
  return;
}
