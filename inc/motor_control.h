#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

#include <Arduino.h>

// Nombres de las bebidas
extern const char* bebidasNombres[];

// Estado del motor
extern uint8_t currentPosition;
extern uint8_t targetPosition;
extern bool isHomed;

extern const uint8_t MOTOR_SPEED;
extern const unsigned long MOVE_TIMEOUT_MS;

// Variables de selección (usadas por otras tareas)
extern uint8_t selectedPosition;
extern uint8_t selectedCombo;

void motorControlInit();
bool goToHome();
bool goToPosition(uint8_t pos);
void resetSystem();
bool getCurrentDirection();

#endif