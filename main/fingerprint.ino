int8_t  getFingerprintEnroll(uint8_t  id) {

  int8_t  p = 0;
  
  Serial.print(F("Waiting for valid finger to enroll as #")); 
  Serial.println(id);

  p = getFingerprintIDez();

  if (p > 0){
    Serial.println(F("Fingerprint already stored!"));
    return -1;
  }else if (p == -1){
    return -1;  
  }
  
  Serial.println(F("Remove finger"));
  delay(2000);
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  Serial.print(F("ID ")); 
  Serial.println(id);
  Serial.println(F("Place same finger again"));

  p = getImage();

  if (p!=FINGERPRINT_OK){
     return -1;
  }

  p = imageToTemplate(2);

  if (p!=FINGERPRINT_OK){
     return -1;
  }
  
  Serial.print(F("Creating model for #"));  
  Serial.println(id);

  p = createModel();

  if (p!=FINGERPRINT_OK){
     return -1;
  }
  
  Serial.print(F("ID ")); 
  Serial.println(id);

  p = storeModel(id);

  if (p!=FINGERPRINT_OK){
     return -1;
  }
  
  return 0;
}

int8_t getImage(){
   int8_t  p = -1;
   unsigned long timeout = millis();
   
   while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      Serial.println(F("Image taken"));
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println(F("."));
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println(F("Communication error"));
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println(F("Imaging error"));
      break;
    default:
      Serial.println(F("Unknown error"));
      break;
    }
    if ((millis() - timeout) > TIMEOUT_FINGER){
      Serial.println(F("Timeout"));
      break;
    }
  }
  return p;
}

int8_t  imageToTemplate(int8_t  slot){
  int8_t  p = -1;
  p = finger.image2Tz(slot);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println(F("Image converted"));
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println(F("Image too messy"));
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println(F("Communication error"));
      break;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println(F("Could not find fingerprint features"));
      break;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println(F("Could not find fingerprint features"));
      break;
    default:
      Serial.println(F("Unknown error"));
      break;
  }
  return p;
}

int8_t  createModel(){
    int8_t  p = -1;
    p = finger.createModel();
    if (p == FINGERPRINT_OK) {
      Serial.println(F("Prints matched!"));
    } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
      Serial.println(F("Communication error"));
    } else if (p == FINGERPRINT_ENROLLMISMATCH) {
      Serial.println(F("Fingerprints did not match"));
    } else {
      Serial.println(F("Unknown error"));
    }  
    return p; 
}

int8_t  storeModel(uint8_t  id){
    int8_t  p=-1;
    p = finger.storeModel(id);
    if (p == FINGERPRINT_OK) {
      Serial.println(F("Stored!"));
    } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
      Serial.println(F("Communication error"));
    } else if (p == FINGERPRINT_BADLOCATION) {
      Serial.println(F("Could not store in that location"));
    } else if (p == FINGERPRINT_FLASHERR) {
      Serial.println(F("Error writing to flash"));
    } else {
      Serial.println(F("Unknown error"));
    } 
    return p;  
}

int8_t  getFingerprintIDez() {
  int8_t  p = 0;
  
  p = getImage();

  if (p!=FINGERPRINT_OK){
     return -1;
  }

  p = imageToTemplate(1);

  if (p!=FINGERPRINT_OK){
    return -1;
  }

  p = fastSearch();
  
  if (p!=FINGERPRINT_OK){
    if (p == FINGERPRINT_NOTFOUND){
      return 0;
    }else{
      return -1;
    }
  }
  
  // FOUND A MATCH!
  Serial.print(F("Found ID #")); 
  Serial.print(finger.fingerID); 
  Serial.print(F(" with confidence of ")); 
  Serial.println(finger.confidence);
  return finger.fingerID; 
}

int8_t  fastSearch(){
  int8_t  p= -1;

  p = finger.fingerFastSearch();
  if (p == FINGERPRINT_OK) {
    Serial.println(F("Found a print match!"));
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println(F("Communication error"));
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println(F("Did not find a match"));
  } else {
    Serial.println(F("Unknown error"));
  }   
  return p;
}

int8_t  deleteFingerprint(int8_t  id) {
  int8_t  p = -1;
  
  p = finger.deleteModel(id);

  if (p == FINGERPRINT_OK) {
    Serial.println(F("Deleted!"));
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println(F("Communication error"));
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println(F("Could not delete in that location"));
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println(F("Error writing to flash"));
  } else {
    Serial.print(F("Unknown error")); 
  }   
  return p;
}
