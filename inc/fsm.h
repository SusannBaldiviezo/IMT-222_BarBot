#ifndef FSM_H
#define FSM_H

#include <Arduino.h>
#include "recipes.h"

// Estados de alto nivel
enum BarbotState {
  ST_IDLE,        // estado 0 - esperar
  ST_BOOT,
  ST_HOME,
  ST_MENU,
  ST_MOVE1,
  ST_DISPENSE1,
  ST_MOVE2,
  ST_DISPENSE2,
  ST_DONE
};

extern BarbotState currentState;
extern uint8_t currentPos;    // posición actual 0..5
extern uint8_t targetPos;     // posición destino
extern int8_t  selectedRecipe;

void fsmInit();
void fsmStep();       // se llama periódicamente desde la tarea
void lcdShowMenu();
void goToState(BarbotState newState);

#endif
