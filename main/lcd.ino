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

  lcd.setCursor(0, 0);
  lcd.print(gLcdBuffer);

  gLcdIndex += 1;
}
