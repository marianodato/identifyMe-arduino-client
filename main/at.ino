int8_t  httpRequest(){
  //const String API = "OR4T3SPD39MMS80O";
  //const String HOST = "api.thingspeak.com";
  //const String FIELD = "field1";
  const String HOST = "identifyme-backend-api.herokuapp.com";
  const String PORT = "443";
  API_RESPONSE = "";
  int8_t  resp = -1;
  int8_t  valSensor = getSensorData();

  Serial.println(F("-------------------------"));
  resp = sendCommandWithRetry("AT+CIPSTART=\"SSL\",\""+ HOST +"\","+ PORT, AT_MEDIUM_TIMEOUT, "OK", NO_NEED_RESPONSE, NORMAL_RETRY);
  if (resp == -1){
    Serial.println(F("Error al ejecutar request"));
    initializeAt();
    return -1;
  }

  const String getData = "GET /ping HTTP/1.0\015\012Host: identifyme-backend-api.herokuapp.com\015\012";
  //const String getData = "GET /update?api_key="+ API +"&"+ FIELD +"="+String(valSensor) + " HTTP/1.1\015\012Host: api.thingspeak.com\015\012";
  resp = sendCommandWithRetry("AT+CIPSEND=" + String(getData.length()+2), AT_SHORT_TIMEOUT, ">", NO_NEED_RESPONSE, NORMAL_RETRY);
  
  if (resp == -1){
    Serial.println(F("Error al ejecutar request"));
    initializeAt();
    return -1;
  }
  
  resp = sendCommandWithRetry(getData, AT_SHORT_TIMEOUT, "+IPD,", NEED_RESPONSE, NORMAL_RETRY);
  
  if (resp == -1){
    Serial.println(F("Error al ejecutar request"));
    initializeAt();
    return -1;
  }


  Serial.println(F("La respuesta: "));
  Serial.println(API_RESPONSE);
  
  sendCommandWithRetry("AT+CIPCLOSE", AT_SHORT_TIMEOUT, "OK", NO_NEED_RESPONSE, NO_RETRY);  
  return 0;
}

int8_t  getSensorData(){
  return random(127);
}

void initializeAt(){
  const String AP = "TH14";
  const String PASS = "TheInvincibles26W12D0L";
  //const String AP = "D200";
  //const String PASS = "pokemon1234";
  sendCommandWithRetry("AT+CIPCLOSE", AT_SHORT_TIMEOUT, "OK", NO_NEED_RESPONSE, NO_RETRY);
  sendCommandWithRetry("AT+CWQAP", AT_SHORT_TIMEOUT, "OK", NO_NEED_RESPONSE, INFINITE_RETRY);
  sendCommandWithRetry("AT+RST", AT_SHORT_TIMEOUT, "OK", NO_NEED_RESPONSE, INFINITE_RETRY);
  sendCommandWithRetry("AT", AT_SHORT_TIMEOUT, "OK", NO_NEED_RESPONSE, INFINITE_RETRY);
  sendCommandWithRetry("AT+CWMODE=1", AT_SHORT_TIMEOUT, "OK", NO_NEED_RESPONSE, INFINITE_RETRY);
  sendCommandWithRetry("AT+CWJAP=\""+ AP +"\",\""+ PASS +"\"", AT_LONG_TIMEOUT, "OK", NO_NEED_RESPONSE, INFINITE_RETRY);
  sendCommandWithRetry("AT+CIPMUX=0", AT_SHORT_TIMEOUT, "OK", NO_NEED_RESPONSE, INFINITE_RETRY);
  sendCommandWithRetry("AT+CIPSSLSIZE=4096", AT_SHORT_TIMEOUT, "OK", NO_NEED_RESPONSE, INFINITE_RETRY);
  return;
}

int8_t  sendCommandWithRetry(const String command, int timeout, char readReplay[], int8_t  mode, int8_t  retries){
  int8_t  i = 0;
  int8_t  resp = 0;
  if (retries == INFINITE_RETRY){
    while (true){
      resp = sendCommand(command, timeout, readReplay, mode);
      if (resp != -1){
        break;
      }
      i ++;
      Serial.print(F("Reintento "));
      Serial.print(i);
      Serial.println(F(" "));
    }
  }else{
    while (i < retries){
      resp = sendCommand(command, timeout, readReplay, mode);
      if (resp != -1){
        break;
      }
      i ++;
      Serial.print(F("Reintento "));
      Serial.print(i);
      Serial.println(F(" "));
    }
  }
  return resp;
}

int8_t  sendCommand(const String command, int timeout, char readReplay[], int8_t  mode){
    char c;
    boolean found = false;
    boolean foundEspace = false;
    boolean foundIPD = false;
    unsigned long time;
    
    // empty buffer
    while(esp8266.available() != 0){
      c = esp8266.read();
      if (c == '\0') continue;
    }

    Serial.print(F("at command => "));
    Serial.print(command);
    Serial.println(F(" "));

    esp8266.println(command);

    API_RESPONSE = "";
    time = millis();
    
    while ((time + timeout) > millis())
    {
        if (found == true){
          break;
        }
        while (esp8266.available())
        {
            c = esp8266.read();

            if (mode == NEED_RESPONSE){
              Serial.print(c);
            }
            
            if (strstr("+IPD,", readReplay)){
              API_RESPONSE += c;
              if (c == '\012'){
                if (foundIPD == false && strstr(API_RESPONSE.c_str(),"+IPD,")){
                  foundIPD = true;
                }else if (foundIPD == true && API_RESPONSE.equals("\015\012")){
                    API_RESPONSE = "";
                    foundEspace = true;
                }else{
                    if (foundEspace == true && foundIPD==true){
                      
                    }else{
                      API_RESPONSE = "";
                    }  
                }
              }
            }else{
              API_RESPONSE += c;
              if (mode == NO_NEED_RESPONSE){
                if(strstr(API_RESPONSE.c_str(), readReplay))
                {
                  found = true;
                  break;
                }
              }
            }
            if (c == '\0') continue;
        }
    } 

    if (mode == NEED_RESPONSE){
      if(strstr(API_RESPONSE.c_str(), readReplay) || foundIPD == true)
      {
        found = true;
      }  
    }else{
      API_RESPONSE = "";
    }

    if(found == true)
    {
      Serial.println(F("Response OK"));
      return 0;
    }
  
    if(found == false)
    {
      Serial.println(F("Response Fail"));
      return -1;
    }
}
