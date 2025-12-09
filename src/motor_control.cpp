#include "motor_control.h"
#include "hardware.h"

// ============ VARIABLES GLOBALES ============
uint8_t currentPosition = 0;
uint8_t targetPosition = 0;
bool isHomed = false;

const uint8_t MOTOR_SPEED = 180;  // Velocidad 0-255 del motor
const unsigned long MOVE_TIMEOUT_MS = 10000;  // 10 segundos

// Variables internas para 2 sensores
static bool isMoving = false;
static unsigned long moveStartTime = 0;
static bool lastCounterState = false;  // Para detectar flancos
static bool expectingHome = true;      // Solo para homing

// ============ INICIALIZACIÓN ============
void motorControlInit() {
  currentPosition = 0;
  targetPosition = 0;
  isHomed = false;
  isMoving = false;
  lastCounterState = false;
  expectingHome = true;
}

// ============ IR A HOME ============
bool goToHome() {
  if (!isMoving) {
    Serial.println("Iniciando homing...");
    isMoving = true;
    expectingHome = true;
    moveStartTime = millis();
    lastCounterState = readLimitPos();  // Leer estado inicial del CONTADOR
    
    // Girar a la derecha (hacia HOME)
    motorRight(MOTOR_SPEED);
  }

  // Leer estado actual del HOME (activo en LOW)
  bool homeActive = readLimitHome();

  // Si detecta HOME (LOW), detener inmediatamente
  if (homeActive && expectingHome) {
    Serial.println("┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓");
    Serial.println("┃ ✓ HOME ENCONTRADO (Pin 15)   ┃");
    Serial.println("┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛");
    motorStop();
    currentPosition = 0;
    isHomed = true;
    isMoving = false;
    expectingHome = true;
    delay(300);  // Pequeña pausa para estabilizar
    return true;  // Llegó a HOME
  }

  // Timeout de seguridad
  if (millis() - moveStartTime > MOVE_TIMEOUT_MS) {
    Serial.println("⚠️  Timeout en homing");
    motorStop();
    isMoving = false;
    expectingHome = true;
    return true;  // Forzar salida
  }

  return false;  // Aún no llega
}

// ============ IR A POSICIÓN ============
bool goToPosition(uint8_t pos) {
  if (pos < 1 || pos > 6) {
    Serial.println("Posición inválida (1-6)");
    return true;  // Error, salir
  }

  if (!isHomed) {
    Serial.println(" Error: no se ha hecho homing");
    return true;  // No puede moverse sin homing
  }

  if (!isMoving) {
    targetPosition = pos;
    Serial.print("🎯 Moviendo a posición ");
    Serial.println(targetPosition);
    
    isMoving = true;
    expectingHome = false;  // No esperamos HOME durante movimiento normal
    moveStartTime = millis();
    lastCounterState = readLimitPos();  // Estado inicial del CONTADOR

    // Decidir dirección
    if (targetPosition > currentPosition) {
      // Ir hacia adelante (derecha) - posiciones mayores
      motorRight(MOTOR_SPEED);
      Serial.println("→ Dirección: ADELANTE (derecha)");
    } else if (targetPosition < currentPosition) {
      // Ir hacia atrás (izquierda) - posiciones menores
      motorLeft(MOTOR_SPEED);
      Serial.println("← Dirección: ATRÁS (izquierda)");
    } else {
      // Ya está en posición
      Serial.println("✓ Ya está en la posición objetivo");
      motorStop();
      isMoving = false;
      return true;
    }
  }

  // Detección de HOME (solo si pasamos por él)
  bool homeActive = readLimitHome();
  if (homeActive && !expectingHome) {
    // Pasamos por HOME durante movimiento normal
    if (currentPosition == 0) {
      currentPosition = 6;  // Si venimos de atrás, estamos en la última
    } else {
      currentPosition = 0;  // Si venimos de adelante, estamos en HOME
    }
    Serial.print("🏠 Pasando por HOME, posición actual: ");
    Serial.println(currentPosition);
    delay(100);  // Evitar múltiples detecciones
  }

  // Detección de CONTADOR (flanco de bajada)
  bool currentCounterState = readLimitPos();
  
  // Detectar flanco de bajada (HIGH -> LOW)
  if (lastCounterState == HIGH && currentCounterState == LOW) {
    // ¡Sensor CONTADOR activado!
    
    if (targetPosition > currentPosition) {
      // Moviéndose hacia adelante (derecha)
      currentPosition++;
      Serial.print("➡️  CONTADOR: posición actual = ");
      Serial.println(currentPosition);
    } else if (targetPosition < currentPosition) {
      // Moviéndose hacia atrás (izquierda)
      currentPosition--;
      Serial.print("⬅️  CONTADOR: posición actual = ");
      Serial.println(currentPosition);
    }
    
    // Si estamos en HOME (0) y vamos hacia atrás, ir a la última posición
    if (currentPosition == 0 && targetPosition < currentPosition) {
      currentPosition = 6;
      Serial.println("🔄 De HOME a posición 6 (última)");
    }
    // Si estamos en posición 6 y vamos hacia adelante, ir a HOME
    else if (currentPosition == 6 && targetPosition > currentPosition) {
      currentPosition = 0;
      Serial.println("🔄 De posición 6 a HOME");
    }
    
    delay(200);  // Debounce y tiempo para pasar el sensor
  }
  
  lastCounterState = currentCounterState;

  // ¿Llegamos al destino?
  if (currentPosition == targetPosition) {
    Serial.println("\n🎯 ¡LLEGÓ AL DESTINO!");
    Serial.print("   Posición final: ");
    Serial.println(currentPosition);
    Serial.println();
    motorStop();
    isMoving = false;
    delay(300);  // Pequeña pausa
    return true;  // Llegó
  }

  // Timeout de seguridad
  if (millis() - moveStartTime > MOVE_TIMEOUT_MS) {
    Serial.println("⚠️  Timeout en movimiento");
    motorStop();
    isMoving = false;
    return true;  // Forzar salida
  }

  return false;  // Aún no llega
}

// ============ ACTUALIZAR ============
void motorControlUpdate() {
  // Nada adicional necesario aquí
}