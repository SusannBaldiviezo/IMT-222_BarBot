#include <Arduino.h>
#include <Wire.h>
#include "hardware.h"
#include "buttons.h"
#include "motor_control.h"
#include "lcd_i2c.h"

// ============ ESTADOS ============
enum State {
  STATE_INIT,      // Esperando inicio
  STATE_HOMING,    // Buscando HOME (pos1)
  STATE_SELECT,    // Seleccionar posición 1-6
  STATE_MOVING,    // Moviéndose a posición
  STATE_READY,     // Listo para dispensar
  STATE_DISPENSING,// Dispensando
  STATE_DONE       // Terminado
};

State currentState = STATE_INIT;
unsigned long stateStartTime = 0;
uint8_t selectedPosition = 1;  // Por defecto posición 1
const unsigned long DISPENSE_TIME = 5000;  // 5 segundos

// Buffer para LCD
char lcdLine[17];

// ============ SETUP ============
void setup() {
  Serial.begin(115200);
  Serial.println("=== SISTEMA BOTELLAS 1-6 ===");
  Serial.println("HOME = Posición 1");
  Serial.println("CONTADOR = Posiciones 2-6");
  
  hardwareInit();
  motorControlInit();
  lcdInit();
  
  lcdPrint(0, "  SISTEMA DE");
  lcdPrint(1, "   BOTELLAS");
  delay(2000);
  
  lcdPrint(0, "Presione OK para");
  lcdPrint(1, "buscar POSICION 1");
  
  currentState = STATE_INIT;
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
    
    // ----------------------------------
    case STATE_INIT:
      snprintf(lcdLine, 17, "OK: Buscar Pos 1");
      lcdPrint(0, lcdLine);
      lcdPrint(1, "                ");
      
      if (btnOk.pressedEvent) {
        lcdPrint(0, "BUSCANDO...");
        lcdPrint(1, "Posicion 1 (HOME)");
        currentState = STATE_HOMING;
        stateStartTime = now;
      }
      break;
    
    // ----------------------------------
    case STATE_HOMING:
      if (goToHome()) {
        lcdPrint(0, "EN POSICION 1");
        lcdPrint(1, "HOMING COMPLETO");
        delay(1500);
        currentState = STATE_SELECT;
        stateStartTime = now;
      }
      break;
    
    // ----------------------------------
    case STATE_SELECT: {
      snprintf(lcdLine, 17, "SELECCIONE: %d/6", selectedPosition);
      lcdPrint(0, lcdLine);
      lcdPrint(1, "U/D:Camb  OK:Ir");
      
      // Botón UP: aumentar posición
      if (btnUp.pressedEvent) {
        selectedPosition++;
        if (selectedPosition > 6) selectedPosition = 6;
        Serial.print("Seleccionado: posición ");
        Serial.println(selectedPosition);
      }
      
      // Botón DOWN: disminuir posición
      if (btnDown.pressedEvent) {
        selectedPosition--;
        if (selectedPosition < 1) selectedPosition = 1;
        Serial.print("Seleccionado: posición ");
        Serial.println(selectedPosition);
      }
      
      // Botón OK: confirmar y mover
      if (btnOk.pressedEvent) {
        Serial.print("\n👉 MOVER A POSICIÓN ");
        Serial.println(selectedPosition);
        
        snprintf(lcdLine, 17, "IR A POS: %d", selectedPosition);
        lcdPrint(0, lcdLine);
        lcdPrint(1, "CONTANDO...");
        
        currentState = STATE_MOVING;
        stateStartTime = now;
      }
      break;
    }
    
    // ----------------------------------
    case STATE_MOVING: {
      // Mostrar progreso en LCD
      snprintf(lcdLine, 17, "Obj:%d Act:%d", selectedPosition, currentPosition);
      lcdPrint(0, lcdLine);
      
      if (selectedPosition > currentPosition) {
        lcdPrint(1, "-> AVANZANDO ->");
      } else if (selectedPosition < currentPosition) {
        lcdPrint(1, "<- RETROCEDIENDO");
      } else {
        lcdPrint(1, "EN POSICION");
      }
      
      // Ejecutar movimiento
      if (goToPosition(selectedPosition)) {
        Serial.print("✅ LLEGÓ A POSICIÓN ");
        Serial.println(currentPosition);
        
        snprintf(lcdLine, 17, "EN POSICION %d", currentPosition);
        lcdPrint(0, lcdLine);
        lcdPrint(1, "OK: DISPENSAR");
        
        currentState = STATE_READY;
        stateStartTime = now;
      }
      break;
    }
    
    // ----------------------------------
    case STATE_READY: {
      snprintf(lcdLine, 17, "POSICION %d/6", currentPosition);
      lcdPrint(0, lcdLine);
      snprintf(lcdLine, 17, "OK: SERVIR 5seg");
      lcdPrint(1, lcdLine);
      
      if (btnOk.pressedEvent) {
        Serial.println("🚰 INICIANDO DISPENSADO");
        lcdPrint(0, "SIRVIENDO...");
        lcdPrint(1, "5 segundos");
        servoSetAngle(SERVO_DISPENSE_ANGLE);
        currentState = STATE_DISPENSING;
        stateStartTime = now;
      }
      break;
    }
    
    // ----------------------------------
    case STATE_DISPENSING: {
      // Calcular tiempo restante
      int segundos = (DISPENSE_TIME - (now - stateStartTime)) / 1000 + 1;
      if (segundos < 0) segundos = 0;
      
      snprintf(lcdLine, 17, "SIRVIENDO...");
      lcdPrint(0, lcdLine);
      snprintf(lcdLine, 17, "Quedan: %d seg", segundos);
      lcdPrint(1, lcdLine);
      
      if (now - stateStartTime >= DISPENSE_TIME) {
        servoSetAngle(SERVO_REST_ANGLE);
        Serial.println("✅ DISPENSADO COMPLETO");
        lcdPrint(0, "TERMINADO");
        lcdPrint(1, "OK: NUEVO CICLO");
        currentState = STATE_DONE;
        stateStartTime = now;
      }
      break;
    }
    
    // ----------------------------------
    case STATE_DONE:
      snprintf(lcdLine, 17, "CICLO COMPLETO");
      lcdPrint(0, lcdLine);
      lcdPrint(1, "OK: OTRA BOTELLA");
      
      if (btnOk.pressedEvent) {
        Serial.println("\n🔄 NUEVO CICLO");
        lcdPrint(0, "NUEVO CICLO");
        lcdPrint(1, "SELECCIONE 1-6");
        selectedPosition = 1;  // Reset a posición 1
        currentState = STATE_SELECT;
        stateStartTime = now;
      }
      break;
  }
  
  delay(50);
}