void lcdScroll()
{
  uint8_t i = 0;
  if (gLcdIndex >= gkLcdTextSize + gkLcdScrollingSpaces) {
    gLcdIndex = 0;
  }

  for (i; i < gkLcdColumns; i++)
  {
    if ((gLcdIndex + i) % (gkLcdTextSize + gkLcdScrollingSpaces) < sizeof(gkLcdText) - 1) {
      gLcdBuffer[i] = gkLcdText[(i + gLcdIndex) % (gkLcdTextSize + gkLcdScrollingSpaces)];
    }
    else {
      gLcdBuffer[i] = ' ';
    }
  }

  lcd.setCursor(0, 1);
  lcd.print(gLcdBuffer);

  gLcdIndex += 1;
  return;
}

void lcdPrint(uint8_t firstLineX, uint8_t secondLineX, const __FlashStringHelper *firstLineText, const __FlashStringHelper *secondLineText, bool needDelay, bool needPiezo) {
  lcd.clear();
  lcd.setCursor(firstLineX, 0);
  lcd.print(firstLineText);
  lcd.setCursor(secondLineX, 1);
  lcd.print(secondLineText);
  if (needPiezo) {
    tone(PIEZO_PIN, gkPiezoFreq, gkPiezoTimeout);
  }
  if (needDelay) {
    delay(gkDisplayTimeout);
  }
  return;
}

void lcdPrintVariable(uint8_t firstLineX, uint8_t secondLineX, const __FlashStringHelper *firstLineText, String secondLineText, bool needDelay, bool needPiezo) {
  lcd.clear();
  lcd.setCursor(firstLineX, 0);
  lcd.print(firstLineText);
  lcd.setCursor(secondLineX, 1);
  lcd.print(secondLineText);
  if (needPiezo) {
    tone(PIEZO_PIN, gkPiezoFreq, gkPiezoTimeout);
  }
  if (needDelay) {
    delay(gkDisplayTimeout);
  }
  return;
}

void testScreens() {
  lcdPrint(3, 2, F("identifyMe"), F("Iniciando..."), true, false);
  lcdPrint(5, 2, F("Error"), F("al iniciar!"), true, false);
  lcdPrint(1, 0, F("Elija un modo:"), F("1)Carga 2)Entrada 3)Salida"), true, false);
  lcdPrint(0, 2, F("Buscando nuevos"), F("usuarios..."), true, false);
  lcdPrint(1, 1, F("No se encontro"), F("ningun usuario!"), true, false);
  lcdPrint(3, 1, F("Por favor,"), F("inserte huella"), true, false);
  lcdPrint(2, 2, F("La huella ya"), F("fue cargada!"), true, false);
  lcdPrint(4, 1, F("Error al"), F("cargar huella!"), true, false);
  lcdPrint(3, 1, F("Por favor,"), F("remueva huella"), true, false);
  lcdPrint(2, 2, F("Configurando"), F("al usuario..."), true, false);
  lcdPrint(4, 3, F("Error al"), F("configurar!"), true, false);
  lcdPrint(0, 4, F("Huella cargada a"), F("USERNAME"), true, false);
  lcdPrint(0, 0, F("Huella cargada a"), F("USERNAME mas largo que 16 chars"), true, false);
  lcdPrint(0, 3, F("No se reconocio"), F("la huella!"), true, false);
  lcdPrint(3, 3, F("Registrando"), F("entrada..."), true, false);
  lcdPrint(3, 3, F("Registrando"), F("salida..."), true, false);
  lcdPrint(1, 3, F("Error al crear"), F("registro!"), true, false);
  lcdPrint(3, 0, F("Bienvenido,"), F("USERNAME"), true, false);
  lcdPrint(2, 0, F("Hasta luego,"), F("USERNAME"), true, false);
  lcdPrint(2, 2, F("Las  huellas"), F("no coinciden!"), true, false);
  return;
}
