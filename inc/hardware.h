#ifndef HARDWARE_H
#define HARDWARE_H

#include <Arduino.h>
#include <Wire.h>
#include "buttons.h"

// PINES BTS7960 (Motor)
extern const uint8_t PIN_MOTOR_REN;
extern const uint8_t PIN_MOTOR_LEN;
extern const uint8_t PIN_MOTOR_RPWM;
extern const uint8_t PIN_MOTOR_LPWM;

//  2 SENSORES 
extern const uint8_t PIN_HOME;        // Sensor HOME (posición 1)
extern const uint8_t PIN_COUNTER;     // Único sensor contador

// SERVO 
extern const uint8_t PIN_SERVO;
extern const int SERVO_REST_ANGLE;
extern const int SERVO_DISPENSE_ANGLE;

// BOTONES
extern const uint8_t PIN_BTN_UP;
extern const uint8_t PIN_BTN_DOWN;
extern const uint8_t PIN_BTN_OK;
extern const uint8_t PIN_BTN_RESET;

// ============ RELÉ (FOCO 220V) ============
extern const uint8_t PIN_RELE_FOCO;

// ============ LCD I2C ============
extern const uint8_t LCD_SDA;
extern const uint8_t LCD_SCL;

// ============ VARIABLES GLOBALES ============
extern Button btnUp;
extern Button btnDown;
extern Button btnOk;
extern Button btnReset;

// ============ FUNCIONES ============
void hardwareInit();

// Motor
void motorRight(uint8_t speed);
void motorLeft(uint8_t speed);
void motorStop();

// Finales de carrera
bool readLimitHome();           // Lectura directa
bool readLimitCounter();        // Lectura directa
bool readHomeSinglePulse();     // Detección de flanco bajada único
bool readCounterSinglePulse();  // Detección de flanco bajada único

// Servo
void servoInit();
void servoSetAngle(int angle);
void servoPress();              // Nueva: función para presionar servo

// Relé (foco 220V)
void releOn();
void releOff();

#endif