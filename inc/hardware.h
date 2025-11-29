#ifndef HARDWARE_H
#define HARDWARE_H

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include "buttons.h"

// ---- Pines (pensados para ESP32 30 pines en Wokwi/real) ----
extern const uint8_t PIN_SERVO;
extern const uint8_t PIN_MOTOR;
extern const uint8_t PIN_HOME;
extern const uint8_t PIN_SLOT;
extern const uint8_t PIN_BTN_UP;
extern const uint8_t PIN_BTN_DOWN;
extern const uint8_t PIN_BTN_OK;
extern const uint8_t PIN_LED_HEARTBEAT;

// Ángulos del “servo lógico”
extern const int SERVO_REST_ANGLE;
extern const int SERVO_PRESS_ANGLE;

// LCD global
extern LiquidCrystal_I2C lcd;

// Botones globales
extern Button btnUp;
extern Button btnDown;
extern Button btnOk;

// Inicialización completa de hardware
void hardwareInit();

// Motor carrusel
void motorOn();
void motorOff();

// Finales de carrera
bool readLimitHome();
bool readLimitSlot();

// Servo manejado por PWM (sin librería Servo)
void servoInit();
void servoSetAngle(int angle);

#endif
