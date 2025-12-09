#include "lcd_i2c.h"

// Crear objeto LCD
LiquidCrystal_I2C lcd(LCD_I2C_ADDR, LCD_COLS, LCD_ROWS);

void lcdInit() {
  // Inicializar LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("INICIANDO...");
  delay(500);
  
  // Limpiar pantalla
  lcd.clear();
}

void lcdClear() {
  lcd.clear();
}

void lcdPrint(int row, const char* text) {
  if (row < 0 || row >= LCD_ROWS) return;
  
  lcd.setCursor(0, row);
  // Limpiar la línea primero
  for (int i = 0; i < LCD_COLS; i++) {
    lcd.print(" ");
  }
  
  lcd.setCursor(0, row);
  lcd.print(text);
}

void lcdPrint(int row, String text) {
  lcdPrint(row, text.c_str());
}

void lcdPrintCentered(int row, const char* text) {
  int len = strlen(text);
  if (len > LCD_COLS) len = LCD_COLS;
  
  int pos = (LCD_COLS - len) / 2;
  if (pos < 0) pos = 0;
  
  lcd.setCursor(0, row);
  // Limpiar la línea
  for (int i = 0; i < LCD_COLS; i++) {
    lcd.print(" ");
  }
  
  lcd.setCursor(pos, row);
  lcd.print(text);
}