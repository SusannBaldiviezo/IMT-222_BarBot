#include <Arduino.h>
#include <Wire.h>
#include "hardware.h"
#include "buttons.h"
#include "motor_control.h"
#include "lcd_i2c.h"
#include "tasks.h"

// SETUP 
void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n====================================");
  Serial.println("=== BAR AUTOMATICO CIRCULAR ===");
  Serial.println("=== SISTEMA CON FreeRTOS ===");
  Serial.println("====================================");
  
  // Inicializar hardware
  hardwareInit();
  
  // Inicializar control de motor
  motorControlInit();
  
  // Inicializar LCD
  lcdInit();
  
  // Mostrar mensaje de inicio
  lcdPrint(0, "  BAR AUTOMATICO");
  lcdPrint(1, "  FreeRTOS v1.0");
  delay(2000);
  
  lcdPrint(0, "Presione OK para");
  lcdPrint(1, "  inicializar");
  
  // Inicializar FreeRTOS
  tasksInit();
  
  Serial.println("✅ Sistema listo. Presione OK para iniciar.");
}

// LOOP PRINCIPAL 
void loop() {
  // Loop vacío - Todo se maneja en las tareas de FreeRTOS
  // Solo mantenemos viva la tarea IDLE
  vTaskDelay(pdMS_TO_TICKS(1000));
  
  // Monitoreo opcional del sistema
  static unsigned long lastReport = 0;
  if (millis() - lastReport > 10000) {
    lastReport = millis();
    
    // Reportar estado de las tareas
    Serial.print("Estado del sistema: ");
    switch(currentSystemState) {
      case SYS_STATE_INIT: Serial.println("INICIO"); break;
      case SYS_STATE_HOMING: Serial.println("BUSCANDO HOME"); break;
      case SYS_STATE_MAIN_MENU: Serial.println("MENU PRINCIPAL"); break;
      case SYS_STATE_SELECT_SINGLE: Serial.println("SELECCION BEBIDA"); break;
      case SYS_STATE_SELECT_COMBO: Serial.println("SELECCION COMBO"); break;
      case SYS_STATE_MOVING: Serial.println("MOVIENDO"); break;
      case SYS_STATE_DISPENSING: Serial.println("SIRVIENDO"); break;
      case SYS_STATE_DONE: Serial.println("TERMINADO"); break;
    }
    
    Serial.print("Posición actual: ");
    Serial.print(currentPosition);
    Serial.print(" (");
    Serial.print(bebidasNombres[currentPosition]);
    Serial.println(")");
    
    Serial.print("Memoria libre: ");
    Serial.print(esp_get_free_heap_size());
    Serial.println(" bytes");
  }
}