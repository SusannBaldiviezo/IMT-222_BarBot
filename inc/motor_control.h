#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

#include <Arduino.h>

// Estado del motor
extern uint8_t currentPosition;  // Posición actual: 0 (HOME), 1-6 (botellas)
extern uint8_t targetPosition;   // Posición objetivo
extern bool isHomed;             // ¿Ha encontrado HOME?

// Velocidad del motor (0-255)
extern const uint8_t MOTOR_SPEED;

// Timeout de seguridad (ms)
extern const unsigned long MOVE_TIMEOUT_MS;

// Inicialización
void motorControlInit();

// Ir a HOME (posición 0)
bool goToHome();  // Retorna true cuando llega

// Ir a posición específica (1-6)
bool goToPosition(uint8_t pos);  // Retorna true cuando llega

// Actualizar estado del motor (llamar en loop)
void motorControlUpdate();

#endif