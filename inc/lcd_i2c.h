#ifndef LCD_I2C_H
#define LCD_I2C_H

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Dirección I2C común: 0x27 o 0x3F
// Si no funciona con 0x27, prueba con 0x3F
#define LCD_I2C_ADDR 0x27
#define LCD_COLS 16
#define LCD_ROWS 2

// Declarar objeto LCD como externo
extern LiquidCrystal_I2C lcd;

void lcdInit();
void lcdClear();
void lcdPrint(int row, const char* text);
void lcdPrint(int row, String text);
void lcdPrintCentered(int row, const char* text);

#endif