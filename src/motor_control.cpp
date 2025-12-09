#include "motor_control.h"
#include "hardware.h"

// ============ VARIABLES GLOBALES ============
uint8_t currentPosition = 1;    // ¡CAMBIO: Empieza en 1, no en 0!
uint8_t targetPosition = 1;
bool isHomed = false;

const uint8_t MOTOR_SPEED = 180;
const unsigned long MOVE_TIMEOUT_MS = 15000;  // 15 segundos

// Variables para detección
static bool isMoving = false;
static unsigned long moveStartTime = 0;
static bool lastCounterState = HIGH;
static bool counterDebounce = false;
static unsigned long lastCounterTime = 0;
static const unsigned long COUNTER_DEBOUNCE_MS = 300;

// ============ INICIALIZACIÓN ============
void motorControlInit() {
  currentPosition = 1;  // ¡IMPORTANTE: Posición 1, no 0!
  targetPosition = 1;
  isHomed = false;
  isMoving = false;
  lastCounterState = HIGH;
  counterDebounce = false;
  lastCounterTime = 0;
}

// ============ IR A HOME (POSICIÓN 1) ============
bool goToHome() {
  if (!isMoving) {
    Serial.println("Iniciando homing...");
    isMoving = true;
    moveStartTime = millis();
    lastCounterState = HIGH;  // Reset contador
    
    // Girar hacia HOME (derecha o izquierda según necesidad)
    // Asumimos que HOME está en un extremo, giramos derecha para buscarlo
    motorRight(MOTOR_SPEED);
    Serial.println("Motor: -> Derecha buscando HOME");
  }

  // Leer HOME (activo en LOW)
  bool homeActive = readLimitHome();

  // Si detecta HOME, ¡ESTO ES POSICIÓN 1!
  if (homeActive) {
    Serial.println("┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓");
    Serial.println("┃ ✓ HOME ENCONTRADO (Pos 1)    ┃");
    Serial.println("┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛");
    motorStop();
    currentPosition = 1;  // ¡POSICIÓN 1!
    isHomed = true;
    isMoving = false;
    delay(500);  // Pausa para estabilizar
    return true;
  }

  // Timeout
  if (millis() - moveStartTime > MOVE_TIMEOUT_MS) {
    Serial.println("⚠️  Timeout en homing");
    motorStop();
    isMoving = false;
    isHomed = true;  // Forzar homing para continuar
    currentPosition = 1;
    return true;
  }

  return false;
}

// ============ IR A POSICIÓN (1-6) ============
bool goToPosition(uint8_t pos) {
  if (pos < 1 || pos > 6) {
    Serial.println("❌ Posición inválida (1-6)");
    return true;
  }

  if (!isHomed) {
    Serial.println("❌ Error: hacer homing primero");
    return true;
  }

  if (!isMoving) {
    targetPosition = pos;
    Serial.print("\n🎯 Objetivo: posición ");
    Serial.println(targetPosition);
    Serial.print("📍 Actual: posición ");
    Serial.println(currentPosition);
    
    isMoving = true;
    moveStartTime = millis();
    lastCounterState = HIGH;
    counterDebounce = false;
    lastCounterTime = millis();

    // Decidir dirección
    if (targetPosition > currentPosition) {
      // Avanzar (derecha) - hacia posiciones mayores
      motorRight(MOTOR_SPEED);
      Serial.println("Dirección: -> DERECHA (avanzar)");
    } else if (targetPosition < currentPosition) {
      // Retroceder (izquierda) - hacia posiciones menores
      motorLeft(MOTOR_SPEED);
      Serial.println("Dirección: <- IZQUIERDA (retroceder)");
    } else {
      // Ya está en posición
      Serial.println("✓ Ya en posición objetivo");
      motorStop();
      isMoving = false;
      return true;
    }
    
    // Pequeña pausa para empezar movimiento
    delay(100);
  }

  // ============ DETECCIÓN DE SENSORES ============
  
  // 1. DETECTAR HOME (POSICIÓN 1)
  bool homeActive = readLimitHome();
  if (homeActive) {
    // ¡HOME detectado = POSICIÓN 1!
    if (currentPosition != 1) {  // Solo si cambiamos de posición
      currentPosition = 1;
      Serial.print("🏠 HOME detectado -> Posición: ");
      Serial.println(currentPosition);
      
      // ¿Llegamos al destino?
      if (currentPosition == targetPosition) {
        Serial.println("🎯 ¡LLEGÓ A POSICIÓN 1!");
        motorStop();
        isMoving = false;
        delay(300);
        return true;
      }
      
      // Esperar a que el sensor se libere
      while (readLimitHome() == LOW) {
        delay(10);
      }
      delay(200);  // Debounce
    }
  }

  // 2. DETECTAR CONTADOR (POSICIONES 2-6)
  bool counterActive = readLimitPos();
  
  // Detectar flanco de bajada (HIGH -> LOW)
  if (lastCounterState == HIGH && counterActive == LOW) {
    unsigned long now = millis();
    
    // Debounce: esperar al menos 300ms entre conteos
    if (!counterDebounce && (now - lastCounterTime) > COUNTER_DEBOUNCE_MS) {
      counterDebounce = true;
      lastCounterTime = now;
      
      // CALCULAR NUEVA POSICIÓN
      if (targetPosition > currentPosition) {
        // Avanzando: incrementar posición
        currentPosition++;
        Serial.print("🔘 CONTADOR -> Posición: ");
        Serial.println(currentPosition);
      } 
      else if (targetPosition < currentPosition) {
        // Retrocediendo: decrementar posición
        if (currentPosition > 2) {  // No bajar de posición 2 (HOME es 1)
          currentPosition--;
          Serial.print("🔘 CONTADOR <- Posición: ");
          Serial.println(currentPosition);
        }
      }
      
      // Verificar si llegamos al destino
      if (currentPosition == targetPosition) {
        Serial.println("\n🎯 ¡LLEGÓ AL DESTINO!");
        Serial.print("   Posición final: ");
        Serial.println(currentPosition);
        motorStop();
        isMoving = false;
        delay(300);
        return true;
      }
      
      // Pequeña pausa para que el sensor se libere
      delay(200);
    }
  }
  
  // Detectar flanco de subida (LOW -> HIGH) para resetear debounce
  if (lastCounterState == LOW && counterActive == HIGH) {
    if (counterDebounce && (millis() - lastCounterTime) > 100) {
      counterDebounce = false;
    }
  }
  
  lastCounterState = counterActive;

  // Timeout de seguridad
  if (millis() - moveStartTime > MOVE_TIMEOUT_MS) {
    Serial.println("⚠️  Timeout en movimiento");
    Serial.print("   Posición actual: ");
    Serial.println(currentPosition);
    motorStop();
    isMoving = false;
    return true;
  }

  return false;  // Aún no llega
}

// ============ ACTUALIZAR ============
void motorControlUpdate() {
  // Vacío por ahora
}