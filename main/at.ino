int8_t  httpRequest(){
  const String API = "OR4T3SPD39MMS80O";
  const String HOST = "api.thingspeak.com";
  const String PORT = "443";
  const String FIELD = "field1";
  API_RESPONSE = "";
  int8_t  resp = -1;
  int8_t  valSensor = getSensorData();
  const String getData = "GET /update?api_key="+ API +"&"+ FIELD +"="+String(valSensor);

  Serial.println(F("-------------------------"));
  resp = sendCommandWithRetry("AT+CIPSTART=\"SSL\",\""+ HOST +"\","+ PORT, AT_MEDIUM_TIMEOUT, "OK", NEED_RESPONSE, NORMAL_RETRY);
  if (resp == -1){
    Serial.println(F("Error al ejecutar request"));
    initializeAt();
    return -1;
  }
  
  resp = sendCommandWithRetry("AT+CIPSEND="+String(getData.length()+2), AT_SHORT_TIMEOUT, ">", NEED_RESPONSE, NORMAL_RETRY);
  if (resp == -1){
    Serial.println(F("Error al ejecutar request"));
    initializeAt();
    return -1;
  }
  
  resp = sendCommandWithRetry(getData, AT_SHORT_TIMEOUT, "SEND OK", NEED_RESPONSE, NORMAL_RETRY);
  if (resp == -1){
    Serial.println(F("Error al ejecutar request"));
    initializeAt();
    return -1;
  }

  Serial.println(F("La respuesta: "));
  Serial.println(API_RESPONSE);
  
  sendCommandWithRetry("AT+CIPCLOSE", AT_SHORT_TIMEOUT, "OK", NEED_RESPONSE, NO_RETRY);  
  return 0;
}

int8_t  getSensorData(){
  return random(127);
}

void initializeAt(){
  const String AP = "TH14";
  //const String AP = "D200";
  const String PASS = "TheInvincibles26W12D0L";
  //const String PASS = "pokemon1234";
  sendCommandWithRetry("AT+CIPCLOSE", AT_SHORT_TIMEOUT, "OK", NEED_RESPONSE, NO_RETRY);
  sendCommandWithRetry("AT", AT_SHORT_TIMEOUT, "OK", NEED_RESPONSE, INFINITE_RETRY);
  sendCommandWithRetry("AT+CWMODE=1", AT_SHORT_TIMEOUT, "OK", NEED_RESPONSE, INFINITE_RETRY);
  sendCommandWithRetry("AT+CWJAP=\""+ AP +"\",\""+ PASS +"\"", AT_LONG_TIMEOUT, "OK", NEED_RESPONSE, INFINITE_RETRY);
  sendCommandWithRetry("AT+CIPMUX=0", AT_SHORT_TIMEOUT, "OK", NEED_RESPONSE, INFINITE_RETRY);
  sendCommandWithRetry("AT+CIPSSLSIZE=4096", AT_SHORT_TIMEOUT, "OK", NEED_RESPONSE, INFINITE_RETRY);
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
    String response;
    response = "";
    unsigned long previous;
    boolean found = false;
    // empty buffer
    //previous = millis();
    //do{
      while(esp8266.available() != 0){    
        response += esp8266.readString();
      }
      response = "";
    //}while(((millis() - previous) < timeout));

    Serial.print(F("at command => "));
    Serial.print(command);
    Serial.println(F(" "));

    esp8266.println(command);
    
    previous = millis();
    // this loop waits for the answer
    do{
        // if there are data in the UART input buffer, reads it
        if(esp8266.available() != 0){    
            response += esp8266.readString();
        }

        if (mode == NOT_NEED_RESPONSE){
          if(strstr(response.c_str(), readReplay))
          {
            found = true;
            break;
          }
        }
        
    // Waits for the answer with time out
    } while(((millis() - previous) < timeout));

    if (mode == NEED_RESPONSE){
      Serial.println(F("La respuesta: "));
      Serial.println(response);
      if(strstr(response.c_str(), readReplay))
      {
        found = true;
      }  
      API_RESPONSE = response;
    }
    response = "";

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
