#include <Arduino.h>
#include "hardware.h"
#include "fsm.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// Prototipos de tareas
void TaskBarbot(void *pvParameters);
void TaskHeartbeat(void *pvParameters);

void setup() {
  Serial.begin(115200);
  Serial.println("Iniciando BarBot con FreeRTOS...");

  hardwareInit();
  fsmInit();

  // Tarea principal: máquina de estados del BarBot
  xTaskCreatePinnedToCore(
    TaskBarbot,
    "TaskBarbot",
    4096,
    nullptr,
    1,
    nullptr,
    1   // Core 1
  );

  // Tarea de "heartbeat" (parpadeo LED para ver el RTOS vivo)
  xTaskCreatePinnedToCore(
    TaskHeartbeat,
    "TaskHeartbeat",
    2048,
    nullptr,
    0,
    nullptr,
    1   // Core 1 también
  );
}

void loop() {
  // No usamos loop, todo corre en tareas
  vTaskDelay(1000 / portTICK_PERIOD_MS);
}

// ----------------- Tareas -----------------

void TaskBarbot(void *pvParameters) {
  (void) pvParameters;
  for (;;) {
    fsmStep();  // Un paso de la máquina de estados
    vTaskDelay(10 / portTICK_PERIOD_MS);  // 10 ms
  }
}

void TaskHeartbeat(void *pvParameters) {
  (void) pvParameters;
  pinMode(PIN_LED_HEARTBEAT, OUTPUT);
  for (;;) {
    digitalWrite(PIN_LED_HEARTBEAT, HIGH);
    vTaskDelay(500 / portTICK_PERIOD_MS);
    digitalWrite(PIN_LED_HEARTBEAT, LOW);
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}
