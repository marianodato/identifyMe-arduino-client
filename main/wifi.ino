int8_t  httpRequest(){
  const char * host = "identifyme-backend-api.herokuapp.com";
  // const char * host = "api.thingspeak.com";
  // const char * api = "OR4T3SPD39MMS80O";
  // const char * field = "field1";
  // int8_t  valSensor = getSensorData();
  String request;
  const int port = 443;
  const char * fingerprint = "08 3B 71 72 02 43 6E CA ED 42 86 93 BA 7E DF 81 C4 BC 62 30"; // SHA1
  // const char * fingerprint = "78 60 18 44 81 35 BF DF 77 84 D4 0A 22 0D 9B 4E 6C DC 57 2C"; // SHA1
  bool foundBlankLine = false;
  String url;
  unsigned long timeout;
  char c;
  API_RESPONSE = "";
  
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

  url = "/users/fingerprintStatus/pending";
   
  /*url = "/update?api_key=";
  url += api;
  url += "&";
  url += field;
  url += "=";
  url += valSensor;*/

  request = "{\"serialNumber\": \"";
  request += SERIAL_NUMBER;
  request += "\", \"atMacAddress\": \"";
  request += AT_MAC_ADDRESS;
  request += "\", \"compileDate\": \"";
  request += COMPILE_DATE;
  request += "\", \"signature\": \"";
  request += SIGNATURE;
  request += "\"}";
 
  Serial.print(F("URL of request: https://"));
  Serial.print(host);
  Serial.print(F(":"));
  Serial.print(port);
  Serial.println(url);
  Serial.print(F("Data: "));
  Serial.println(request);
 
  client.print(String("GET ") + url + " HTTP/1.0\r\n" +
                "Host: " + host + "\r\n" +
                "Content-length: " + request.length() + "\r\n\r\n" + 
                request);
  timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > TIMEOUT) {
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
  Serial.println(F("Closing connection"));

  Serial.println(F("Response: "));
  Serial.println(API_RESPONSE);
  API_RESPONSE = "";
  return 0;
}

int8_t  getSensorData(){
  return random(127);
}

void connectToWifi(){
  const char * ssid = "TH14";
  const char * password = "TheInvincibles26W12D0L";
  // const char * ssid = "D200";
  // const char * password = "pokemon1234";
  
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
