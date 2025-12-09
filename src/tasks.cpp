#include "tasks.h"
#include "hardware.h"
#include "motor_control.h"
#include "lcd_i2c.h"
#include "buttons.h"

// ============ VARIABLES GLOBALES COMPARTIDAS ============
SystemState currentSystemState = SYS_STATE_INIT;

// ============ COLAS Y SEMÁFOROS ============
QueueHandle_t buttonQueue = NULL;
QueueHandle_t motorCommandQueue = NULL;
QueueHandle_t lcdUpdateQueue = NULL;
SemaphoreHandle_t lcdMutex = NULL;

// ============ VARIABLES DE CONTROL ============
static unsigned long dispenseStartTime = 0;
static const unsigned long DISPENSE_TIME = 5000;
static uint8_t comboStep = 0;

// ============ TASK 1: MANEJADOR DE BOTONES ============
void taskButtonHandler(void *pvParameters) {
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xFrequency = pdMS_TO_TICKS(20);
  
  ButtonEvent btnEvent;
  unsigned long now;
  
  Serial.println("🔄 Task ButtonHandler iniciada");
  
  while(1) {
    now = millis();
    
    // Actualizar estado de todos los botones
    updateButton(btnUp, now);
    updateButton(btnDown, now);
    updateButton(btnOk, now);
    updateButton(btnReset, now);
    
    // Enviar eventos a la cola
    if (btnUp.pressedEvent) {
      btnEvent.buttonId = 1;
      btnEvent.pressed = true;
      xQueueSend(buttonQueue, &btnEvent, 0);
      Serial.println("🔼 Botón UP presionado");
    }
    
    if (btnDown.pressedEvent) {
      btnEvent.buttonId = 2;
      btnEvent.pressed = true;
      xQueueSend(buttonQueue, &btnEvent, 0);
      Serial.println("🔽 Botón DOWN presionado");
    }
    
    if (btnOk.pressedEvent) {
      btnEvent.buttonId = 3;
      btnEvent.pressed = true;
      xQueueSend(buttonQueue, &btnEvent, 0);
      Serial.println("✅ Botón OK presionado");
    }
    
    if (btnReset.pressedEvent) {
      btnEvent.buttonId = 4;
      btnEvent.pressed = true;
      xQueueSend(buttonQueue, &btnEvent, 0);
      Serial.println("🔄 Botón RESET presionado");
    }
    
    vTaskDelayUntil(&xLastWakeTime, xFrequency);
  }
}

// ============ TASK 2: CONTROL DE MOTOR ============
void taskMotorControl(void *pvParameters) {
  MotorCommand cmd;
  LCDUpdate lcdMsg;
  
  Serial.println("🔄 Task MotorControl iniciada");
  
  while(1) {
    if (xQueueReceive(motorCommandQueue, &cmd, portMAX_DELAY) == pdTRUE) {
      
      // Tomar mutex para LCD
      if (xSemaphoreTake(lcdMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
        
        switch(cmd.command) {
          case 1: // GOTO_HOME
            snprintf(lcdMsg.text, 17, "BUSCANDO HOME...");
            lcdMsg.line = 0;
            xQueueSend(lcdUpdateQueue, &lcdMsg, 0);
            
            snprintf(lcdMsg.text, 17, "Espere por favor");
            lcdMsg.line = 1;
            xQueueSend(lcdUpdateQueue, &lcdMsg, 0);
            
            xSemaphoreGive(lcdMutex);
            
            // Ejecutar homing
            while(!goToHome()) {
              vTaskDelay(pdMS_TO_TICKS(10));
            }
            
            // Actualizar estado del sistema
            currentSystemState = SYS_STATE_MAIN_MENU;
            break;
            
          case 2: // GOTO_POS
            if (cmd.comboId == 0) {
              // Bebida simple
              snprintf(lcdMsg.text, 17, "Yendo a Pos %d", cmd.position);
              lcdMsg.line = 0;
              xQueueSend(lcdUpdateQueue, &lcdMsg, 0);
              
              snprintf(lcdMsg.text, 17, "%s", bebidasNombres[cmd.position]);
              lcdMsg.line = 1;
              xQueueSend(lcdUpdateQueue, &lcdMsg, 0);
            } else {
              // Combo (parte de combo)
              snprintf(lcdMsg.text, 17, "Combo %d/2", comboStep);
              lcdMsg.line = 0;
              xQueueSend(lcdUpdateQueue, &lcdMsg, 0);
              
              snprintf(lcdMsg.text, 17, "Pos %d", cmd.position);
              lcdMsg.line = 1;
              xQueueSend(lcdUpdateQueue, &lcdMsg, 0);
            }
            
            xSemaphoreGive(lcdMutex);
            
            // Ejecutar movimiento
            currentSystemState = SYS_STATE_MOVING;
            while(!goToPosition(cmd.position)) {
              vTaskDelay(pdMS_TO_TICKS(10));
            }
            
            // PRESIONAR SERVO AL LLEGAR
            servoPress();
            
            // Si es bebida simple, iniciar dispensación
            if (cmd.comboId == 0) {
              currentSystemState = SYS_STATE_DISPENSING;
              dispenseStartTime = millis();
              releOn();  // Encender foco
            }
            break;
            
          case 3: // STOP
            motorStop();
            Serial.println("🛑 Motor detenido por comando");
            break;
        }
      }
    }
    
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

// ============ TASK 3: ACTUALIZACIÓN LCD ============
void taskLCDUpdate(void *pvParameters) {
  LCDUpdate lcdMsg;
  
  Serial.println("🔄 Task LCDUpdate iniciada");
  
  while(1) {
    // Procesar mensajes de actualización de LCD
    if (xQueueReceive(lcdUpdateQueue, &lcdMsg, pdMS_TO_TICKS(50)) == pdTRUE) {
      if (xSemaphoreTake(lcdMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        lcdPrint(lcdMsg.line, lcdMsg.text);
        xSemaphoreGive(lcdMutex);
      }
    }
    
    // Si estamos en modo dispensación, mostrar cuenta regresiva
    if (currentSystemState == SYS_STATE_DISPENSING) {
      unsigned long elapsed = millis() - dispenseStartTime;
      int secondsLeft = (DISPENSE_TIME - elapsed) / 1000 + 1;
      
      if (secondsLeft > 0) {
        char line[17];
        snprintf(line, 17, "Sirviendo... %d", secondsLeft);
        
        if (xSemaphoreTake(lcdMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
          lcdPrint(1, line);
          xSemaphoreGive(lcdMutex);
        }
      }
      
      // Finalizar dispensación
      if (elapsed >= DISPENSE_TIME) {
        servoSetAngle(0);  // Servo a reposo
        releOff();         // Apagar foco
        currentSystemState = SYS_STATE_MAIN_MENU;
        
        if (xSemaphoreTake(lcdMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
          lcdPrint(0, "TERMINADO!");
          lcdPrint(1, "Listo para servir");
          xSemaphoreGive(lcdMutex);
        }
        
        vTaskDelay(pdMS_TO_TICKS(1500));
      }
    }
    
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

// ============ TASK 4: SUPERVISOR DEL SISTEMA ============
void taskSystemSupervisor(void *pvParameters) {
  ButtonEvent btnEvent;
  MotorCommand motorCmd;
  LCDUpdate lcdMsg;
  
  // Definición de combinaciones
  struct Combinacion {
    uint8_t pos1;
    uint8_t pos2;
    const char* nombre;
    const char* abreviatura;
  };
  
  const Combinacion combinaciones[] = {
    {1, 2, "Cuba Libre",       "CO+RO"},
    {1, 3, "Fernet con Coca",  "CO+FE"},
    {4, 5, "Chuflay",          "SP+SI"},
    {4, 6, "Vodka Sprite",     "SP+VO"}
  };
  
  Serial.println("🔄 Task SystemSupervisor iniciada");
  
  while(1) {
    // Procesar eventos de botones
    if (xQueueReceive(buttonQueue, &btnEvent, pdMS_TO_TICKS(50)) == pdTRUE) {
      
      if (xSemaphoreTake(lcdMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        
        switch(currentSystemState) {
          case SYS_STATE_INIT:
            if (btnEvent.buttonId == 3) { // OK
              currentSystemState = SYS_STATE_HOMING;
              motorCmd.command = 1; // GOTO_HOME
              motorCmd.position = 1;
              motorCmd.comboId = 0;
              xQueueSend(motorCommandQueue, &motorCmd, 0);
            }
            break;
            
          case SYS_STATE_MAIN_MENU:
            if (btnEvent.buttonId == 1) { // UP - Bebida simple
              currentSystemState = SYS_STATE_SELECT_SINGLE;
              selectedPosition = currentPosition;
              
              lcdPrint(0, "SELECCIONE BEBIDA");
              char posText[17];
              snprintf(posText, 17, "Pos %d: %s", currentPosition, bebidasNombres[currentPosition]);
              lcdPrint(1, posText);
            }
            else if (btnEvent.buttonId == 2) { // DOWN - Combo
              currentSystemState = SYS_STATE_SELECT_COMBO;
              selectedCombo = 0;
              
              lcdPrint(0, "SELECCIONE COMBO");
              lcdPrint(1, "U/D:Camb  OK:Sel");
            }
            break;
            
          case SYS_STATE_SELECT_SINGLE:
            if (btnEvent.buttonId == 1) { // UP - Incrementar
              selectedPosition = (selectedPosition < 6) ? selectedPosition + 1 : 1;
            }
            else if (btnEvent.buttonId == 2) { // DOWN - Decrementar
              selectedPosition = (selectedPosition > 1) ? selectedPosition - 1 : 6;
            }
            else if (btnEvent.buttonId == 3) { // OK - Confirmar
              motorCmd.command = 2; // GOTO_POS
              motorCmd.position = selectedPosition;
              motorCmd.comboId = 0;
              xQueueSend(motorCommandQueue, &motorCmd, 0);
            }
            
            // Mostrar selección actual
            char selectText[17];
            snprintf(selectText, 17, "Pos %d: %s", selectedPosition, bebidasNombres[selectedPosition]);
            lcdPrint(1, selectText);
            break;
            
          case SYS_STATE_SELECT_COMBO:
            if (btnEvent.buttonId == 1) { // UP - Siguiente combo
              selectedCombo = (selectedCombo < 3) ? selectedCombo + 1 : 0;
            }
            else if (btnEvent.buttonId == 2) { // DOWN - Anterior combo
              selectedCombo = (selectedCombo > 0) ? selectedCombo - 1 : 3;
            }
            else if (btnEvent.buttonId == 3) { // OK - Preparar combo
              comboStep = 1;
              motorCmd.command = 2; // GOTO_POS
              motorCmd.position = combinaciones[selectedCombo].pos1;
              motorCmd.comboId = selectedCombo + 1;
              xQueueSend(motorCommandQueue, &motorCmd, 0);
            }
            
            // Mostrar combo actual
            char comboText[17];
            snprintf(comboText, 17, "%s", combinaciones[selectedCombo].abreviatura);
            lcdPrint(1, comboText);
            break;
            
          case SYS_STATE_DISPENSING:
            // Durante dispensación, solo RESET está activo
            break;
        }
        
        // Botón RESET siempre funciona
        if (btnEvent.buttonId == 4) { // RESET
          Serial.println("🔄 Reset del sistema");
          
          motorCmd.command = 3; // STOP
          xQueueSend(motorCommandQueue, &motorCmd, 0);
          
          releOff();
          comboStep = 0;
          currentSystemState = SYS_STATE_MAIN_MENU;
          
          lcdPrint(0, "SISTEMA REINICIADO");
          lcdPrint(1, "Listo para usar");
          
          vTaskDelay(pdMS_TO_TICKS(2000));
        }
        
        xSemaphoreGive(lcdMutex);
      }
    }
    
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

// ============ INICIALIZACIÓN DE TAREAS ============
void tasksInit() {
  Serial.println("🎯 Inicializando FreeRTOS tasks...");
  
  // Crear mutex para LCD
  lcdMutex = xSemaphoreCreateMutex();
  
  // Crear colas
  buttonQueue = xQueueCreate(10, sizeof(ButtonEvent));
  motorCommandQueue = xQueueCreate(5, sizeof(MotorCommand));
  lcdUpdateQueue = xQueueCreate(5, sizeof(LCDUpdate));
  
  // Crear tareas
  xTaskCreatePinnedToCore(
    taskButtonHandler,    // Función de la tarea
    "BtnHandler",         // Nombre de la tarea
    4096,                 // Tamaño de stack
    NULL,                 // Parámetros
    2,                    // Prioridad (media)
    NULL,                 // Handle de la tarea
    0                     // Núcleo (0 o 1)
  );
  
  xTaskCreatePinnedToCore(
    taskMotorControl,
    "MotorCtrl",
    4096,
    NULL,
    3,                    // Prioridad alta (motor es crítico)
    NULL,
    1                     // Núcleo diferente para mejor rendimiento
  );
  
  xTaskCreatePinnedToCore(
    taskLCDUpdate,
    "LCDUpdate",
    4096,
    NULL,
    1,                    // Prioridad baja
    NULL,
    0
  );
  
  xTaskCreatePinnedToCore(
    taskSystemSupervisor,
    "SysSupervisor",
    4096,
    NULL,
    4,                    // Prioridad más alta (coordinación)
    NULL,
    0
  );
  
  Serial.println("✅ Tasks de FreeRTOS creadas exitosamente");
}