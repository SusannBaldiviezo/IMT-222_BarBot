#ifndef HARDWARE_H
#define HARDWARE_H

#include <Arduino.h>
#include <Wire.h>        // ¡AGREGAR ESTO!
#include "buttons.h"

// ============ PINES BTS7960 (Motor) ============
extern const uint8_t PIN_MOTOR_REN;   // Enable derecha
extern const uint8_t PIN_MOTOR_LEN;   // Enable izquierda
extern const uint8_t PIN_MOTOR_RPWM;  // PWM derecha
extern const uint8_t PIN_MOTOR_LPWM;  // PWM izquierda

// ============ FINALES DE CARRERA (SOLO 2) ============
extern const uint8_t PIN_HOME;        // Posición 0 (HOME)
extern const uint8_t PIN_COUNTER;     // Contador de botellas (1-6)

// ============ SERVO ============
extern const uint8_t PIN_SERVO;
extern const int SERVO_REST_ANGLE;    // Ángulo reposo (0°)
extern const int SERVO_DISPENSE_ANGLE; // Ángulo dispensar (90°)

// ============ BOTONES ============
extern const uint8_t PIN_BTN_UP;
extern const uint8_t PIN_BTN_DOWN;
extern const uint8_t PIN_BTN_OK;

// ============ LED ============
extern const uint8_t PIN_LED_HEARTBEAT;

// ============ LCD I2C ============
extern const uint8_t LCD_SDA;
extern const uint8_t LCD_SCL;

// ============ VARIABLES GLOBALES ============
extern Button btnUp;
extern Button btnDown;
extern Button btnOk;

// ============ FUNCIONES ============
void hardwareInit();

// Motor
void motorRight(uint8_t speed);  // Girar derecha (0-255)
void motorLeft(uint8_t speed);   // Girar izquierda (0-255)
void motorStop();                // Detener motor

// Finales de carrera (2 sensores)
bool readLimitHome();            // Leer HOME
bool readLimitPos();             // Leer CONTADOR (único para todas las posiciones)

// Servo
void servoInit();
void servoSetAngle(int angle);

#endif