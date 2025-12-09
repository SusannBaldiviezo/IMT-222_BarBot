#include <Arduino.h>
#include <Wire.h>               // ¡IMPORTANTE!
#include "hardware.h"
#include "buttons.h"
#include "motor_control.h"
#include "lcd_i2c.h"           // Incluir LCD

// ============ ESTADOS DE LA MÁQUINA ============
enum State {
  STATE_INIT,           // Inicialización
  STATE_HOMING,         // Buscando HOME
  STATE_SELECT,         // Elegir posición con UP/DOWN
  STATE_MOVING,         // Moviéndose a posición
  STATE_READY,          // Listo para dispensar
  STATE_DISPENSING,     // Dispensando (servo activo)
  STATE_DONE            // Terminado, esperando nuevo ciclo
};

State currentState = STATE_INIT;
unsigned long stateStartTime = 0;

// Posición seleccionada por el usuario
uint8_t selectedPosition = 1;  // Por defecto botella 1

// Tiempo de dispensado (ms)
const unsigned long DISPENSE_TIME = 5000;  // 5 segundos

// Variable para mostrar tiempo en LCD
int segundosRestantes = 0;

// ============ SETUP ============
void setup() {
  Serial.begin(115200);
  Serial.println("=== Sistema de Control ===");
  
  hardwareInit();
  motorControlInit();
  lcdInit();  // Inicializar LCD
  
  // Mostrar mensaje inicial en LCD
  lcdPrint(0, "BOTELLAS 1-6");
  lcdPrint(1, "OK para inicio");
  
  currentState = STATE_INIT;
  stateStartTime = millis();
  
  Serial.println("Sistema iniciado. Presiona OK.");
}

// ============ LOOP ============
void loop() {
  unsigned long now = millis();
  
  // Actualizar botones
  updateButton(btnUp, now);
  updateButton(btnDown, now);
  updateButton(btnOk, now);
  
  // LED heartbeat
  static unsigned long lastBlink = 0;
  if (now - lastBlink > 500) {
    digitalWrite(PIN_LED_HEARTBEAT, !digitalRead(PIN_LED_HEARTBEAT));
    lastBlink = now;
  }
  
  // ============ MÁQUINA DE ESTADOS ============
  switch (currentState) {
    
    case STATE_INIT:
      lcdPrint(0, "INICIO");
      lcdPrint(1, "OK para homing");
      
      if (btnOk.pressedEvent) {
        Serial.println("\nIniciando homing...");
        lcdPrint(0, "BUSCANDO HOME");
        lcdPrint(1, "Espere...");
        currentState = STATE_HOMING;
        stateStartTime = now;
      }
      break;
    
    case STATE_HOMING:
      if (goToHome()) {
        Serial.println("Homing completado");
        lcdPrint(0, "HOME ENCONTRADO");
        lcdPrint(1, "Sel. botella 1-6");
        delay(1000);
        currentState = STATE_SELECT;
        stateStartTime = now;
      }
      break;
    
    case STATE_SELECT: {
      // Actualizar LCD con posición seleccionada
      char lcdLine[17];
      snprintf(lcdLine, 17, "Botella: %d/6", selectedPosition);
      lcdPrint(0, lcdLine);
      lcdPrint(1, "UP/DOWN OK-confirm");
      
      // UP incrementa posición
      if (btnUp.pressedEvent) {
        selectedPosition++;
        if (selectedPosition > 6) selectedPosition = 6;
        Serial.print("Botella: ");
        Serial.println(selectedPosition);
      }
      
      // DOWN decrementa posición
      if (btnDown.pressedEvent) {
        selectedPosition--;
        if (selectedPosition < 1) selectedPosition = 1;
        Serial.print("Botella: ");
        Serial.println(selectedPosition);
      }
      
      // OK confirma
      if (btnOk.pressedEvent) {
        snprintf(lcdLine, 17, "-> Botella %d", selectedPosition);
        lcdPrint(0, lcdLine);
        lcdPrint(1, "Moviendo...");
        currentState = STATE_MOVING;
        stateStartTime = now;
      }
      break;
    }
    
    case STATE_MOVING: {
      // Mostrar progreso en LCD
      char lcdLine[17];
      snprintf(lcdLine, 17, "Obj:%d Act:%d", selectedPosition, currentPosition);
      lcdPrint(0, lcdLine);
      lcdPrint(1, "En movimiento...");
      
      if (goToPosition(selectedPosition)) {
        Serial.println("Posicion alcanzada");
        lcdPrint(0, "POSICION OK");
        snprintf(lcdLine, 17, "Botella: %d", selectedPosition);
        lcdPrint(1, lcdLine);
        delay(1000);
        currentState = STATE_READY;
        stateStartTime = now;
      }
      break;
    }
    
    case STATE_READY: {
      char lcdLine[17];
      lcdPrint(0, "EN POSICION");
      snprintf(lcdLine, 17, "OK dispensar %d", selectedPosition);
      lcdPrint(1, lcdLine);
      
      if (btnOk.pressedEvent) {
        Serial.println("Dispensando...");
        lcdPrint(0, "DISPENSANDO");
        lcdPrint(1, "5 segundos...");
        servoSetAngle(SERVO_DISPENSE_ANGLE);
        currentState = STATE_DISPENSING;
        stateStartTime = now;
      }
      break;
    }
    
    case STATE_DISPENSING: {
      // Calcular segundos restantes
      segundosRestantes = (DISPENSE_TIME - (now - stateStartTime)) / 1000 + 1;
      if (segundosRestantes < 0) segundosRestantes = 0;
      
      // Mostrar cuenta regresiva
      char lcdLine[17];
      lcdPrint(0, "DISPENSANDO...");
      snprintf(lcdLine, 17, "Tiempo: %d seg", segundosRestantes);
      lcdPrint(1, lcdLine);
      
      if (now - stateStartTime >= DISPENSE_TIME) {
        Serial.println("Dispensado completo");
        servoSetAngle(SERVO_REST_ANGLE);
        lcdPrint(0, "TERMINADO");
        lcdPrint(1, "OK para nuevo");
        currentState = STATE_DONE;
        stateStartTime = now;
      }
      break;
    }
    
    case STATE_DONE:
      lcdPrint(0, "CICLO COMPLETO");
      lcdPrint(1, "OK para nuevo");
      
      if (btnOk.pressedEvent) {
        Serial.println("Nuevo ciclo");
        lcdPrint(0, "SELECCIONAR");
        lcdPrint(1, "Botella 1-6");
        selectedPosition = 1;
        currentState = STATE_SELECT;
        stateStartTime = now;
      }
      break;
  }
  
  delay(50);
}