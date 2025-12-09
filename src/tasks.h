#ifndef TASKS_H
#define TASKS_H

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

// Definición de tareas
void taskButtonHandler(void *pvParameters);
void taskMotorControl(void *pvParameters);
void taskLCDUpdate(void *pvParameters);
void taskSystemSupervisor(void *pvParameters);

// Colas y semáforos
extern QueueHandle_t buttonQueue;
extern QueueHandle_t motorCommandQueue;
extern QueueHandle_t lcdUpdateQueue;
extern SemaphoreHandle_t lcdMutex;

// Estructuras para comunicación
struct ButtonEvent {
  uint8_t buttonId;  // 1:UP, 2:DOWN, 3:OK, 4:RESET
  bool pressed;
};

struct MotorCommand {
  uint8_t command;  // 1:GOTO_HOME, 2:GOTO_POS, 3:STOP
  uint8_t position;
  uint8_t comboId;  // 0 para bebida simple
};

struct LCDUpdate {
  uint8_t line;     // 0 o 1
  char text[17];    // Texto a mostrar
};

// Estados del sistema (compartidos entre tareas)
enum SystemState {
  SYS_STATE_INIT,
  SYS_STATE_HOMING,
  SYS_STATE_MAIN_MENU,
  SYS_STATE_SELECT_SINGLE,
  SYS_STATE_SELECT_COMBO,
  SYS_STATE_MOVING,
  SYS_STATE_DISPENSING,
  SYS_STATE_DONE
};

extern SystemState currentSystemState;

void tasksInit();

#endif