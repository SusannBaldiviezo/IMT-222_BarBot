#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

#include <Arduino.h>

// Estado del motor - POSICIONES 1-6 (HOME = 1)
extern uint8_t currentPosition;  // Posición actual: 1-6 (botellas)
extern uint8_t targetPosition;   // Posición objetivo: 1-6
extern bool isHomed;             // ¿Ha encontrado HOME (posición 1)?

// Velocidad del motor (0-255)
extern const uint8_t MOTOR_SPEED;

// Timeout de seguridad (ms)
extern const unsigned long MOVE_TIMEOUT_MS;

// Inicialización
void motorControlInit();

// Ir a HOME (posición 1)
bool goToHome();  // Retorna true cuando llega

// Ir a posición específica (1-6)
bool goToPosition(uint8_t pos);  // Retorna true cuando llega

// Actualizar estado del motor
void motorControlUpdate();

#endif