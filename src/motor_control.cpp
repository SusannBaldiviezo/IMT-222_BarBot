#include "motor_control.h"
#include "hardware.h"

//  NOMBRES DE BEBIDAS 
const char* bebidasNombres[] = {
  "",
  "COCA",     // 1
  "RON",      // 2
  "FERNET",   // 3
  "SPRITE",   // 4
  "SINGANI",  // 5
  "VODKA"     // 6
};

//  VARIABLES 
uint8_t currentPosition = 1;
uint8_t targetPosition = 1;
bool isHomed = false;

// Variables de selección
uint8_t selectedPosition = 1;
uint8_t selectedCombo = 0;

const uint8_t MOTOR_SPEED = 180;
const unsigned long MOVE_TIMEOUT_MS = 15000;

// Variables de control
static bool isMoving = false;
static unsigned long moveStartTime = 0;
static bool sensorActivated = false;
static unsigned long sensorActivationTime = 0;
static bool currentDirectionForward = true;

//  RESET 
void resetSystem() {
  motorStop();
  currentPosition = 1;
  targetPosition = 1;
  isHomed = false;
  isMoving = false;
  sensorActivated = false;
  Serial.println("Sistema reseteado");
}

//  INICIALIZACIÓN 
void motorControlInit() {
  resetSystem();
}

//  CALCULAR MEJOR DIRECCIÓN PARA SISTEMA CIRCULAR 
bool calculateBestDirection(uint8_t target, uint8_t current) {
  if (target == current) return true;
  
  Serial.print("CALCULAR DIRECCION: ");
  Serial.print(current);
  Serial.print(" -> ");
  Serial.println(target);
  
  // Sistema circular: 1-2-3-4-5-6-(vuelve a 1)
  
  // Distancia hacia adelante (derecha)
  int forwardDistance = 0;
  if (target > current) {
    forwardDistance = target - current;
  } else {
    forwardDistance = (6 - current) + target;
  }
  
  // Distancia hacia atrás (izquierda)
  int backwardDistance = 0;
  if (target < current) {
    backwardDistance = current - target;
  } else {
    backwardDistance = current + (6 - target);
  }
  
  // Si distancias iguales, elegir según posición actual
  if (forwardDistance == backwardDistance) {
    if (current >= 5) return false; // atrás si cerca del final
    if (current <= 2) return true;  // adelante si cerca del inicio
    return true; // default: adelante
  }
  
  Serial.print("  -> Adelante: ");
  Serial.print(forwardDistance);
  Serial.print(" | <- Atrás: ");
  Serial.println(backwardDistance);
  
  return (forwardDistance < backwardDistance);
}

//  IR A HOME 
bool goToHome() {
  if (!isMoving) {
    Serial.println("BUSCANDO HOME...");
    isMoving = true;
    moveStartTime = millis();
    sensorActivated = false;
    currentDirectionForward = false; // HOME generalmente está atrás
    
    motorLeft(MOTOR_SPEED);
  }
  
  // DETECCIÓN ÚNICA DE HOME
  if (readHomeSinglePulse()) {
    if (!sensorActivated) {
      sensorActivated = true;
      sensorActivationTime = millis();
      Serial.println("  PULSO HOME detectado!");
    }
    
    if (millis() - sensorActivationTime > 10) {
      Serial.println("✓ HOME ENCONTRADO");
      motorStop();
      delay(50);
      
      currentPosition = 1;
      isHomed = true;
      isMoving = false;
      
      // Retroceder para liberar sensor
      motorRight(100);
      delay(30);
      motorStop();
      
      delay(300);
      return true;
    }
  }

  if (millis() - moveStartTime > MOVE_TIMEOUT_MS) {
    Serial.println(" Timeout en homing");
    motorStop();
    isMoving = false;
    return true;
  }

  return false;
}

//  IR A POSICIÓN CON DETECCIÓN ÚNICA 
bool goToPosition(uint8_t pos) {
  if (pos < 1 || pos > 6) return true;
  if (!isHomed) return true;
  
  if (pos == currentPosition) {
    Serial.println("✓ Ya en posición");
    return true;
  }

  if (!isMoving) {
    targetPosition = pos;
    Serial.print("IR A POSICION: ");
    Serial.print(pos);
    Serial.print(" (");
    Serial.print(bebidasNombres[pos]);
    Serial.println(")");
    
    // Calcular dirección óptima
    currentDirectionForward = calculateBestDirection(pos, currentPosition);
    
    isMoving = true;
    moveStartTime = millis();
    sensorActivated = false;
    
    if (currentDirectionForward) {
      motorRight(MOTOR_SPEED);
      Serial.println("DIRECCIÓN: -> ADELANTE");
    } else {
      motorLeft(MOTOR_SPEED);
      Serial.println("DIRECCIÓN: <- ATRÁS");
    }
    
    delay(100);
  }
  
  //  DETECCIÓN ÚNICA DE SENSORES 
  unsigned long now = millis();
  
  // 1. DETECTAR HOME (posición 1) - PULSO ÚNICO
  if (readHomeSinglePulse()) {
    if (!sensorActivated) {
      Serial.println("PULSO HOME detectado!");
      motorStop();
      delay(30);
      
      currentPosition = 1;
      sensorActivated = true;
      sensorActivationTime = now;
      
      // Verificar si llegó a destino
      if (targetPosition == 1) {
        Serial.println(" LLEGÓ A HOME!");
        isMoving = false;
        delay(200);
        return true;
      }
      
      // Si no era HOME el objetivo, continuar
      // Retroceder un poco para liberar sensor
      motorRight(100);
      delay(30);
      motorStop();
      delay(100);
      
      // Continuar en la dirección original
      if (currentDirectionForward) {
        motorRight(MOTOR_SPEED);
      } else {
        motorLeft(MOTOR_SPEED);
      }
    }
  }
  
  // 2. DETECTAR CONTADOR - PULSO ÚNICO
  else if (readCounterSinglePulse()) {
    if (!sensorActivated) {
      uint8_t newPosition = currentPosition;
      
      // Calcular nueva posición según dirección
      if (currentDirectionForward) {
        // Vamos ADELANTE (derecha)
        newPosition = (currentPosition < 6) ? currentPosition + 1 : 1;
        Serial.print(" CONTADOR (Adelante): ");
      } else {
        // Vamos ATRÁS (izquierda)
        newPosition = (currentPosition > 1) ? currentPosition - 1 : 6;
        Serial.print(" CONTADOR (Atrás): ");
      }
      
      Serial.print(currentPosition);
      Serial.print(" -> ");
      Serial.println(newPosition);
      
      motorStop();
      delay(30);
      
      currentPosition = newPosition;
      sensorActivated = true;
      sensorActivationTime = now;
      
      // Verificar si llegó al destino
      if (currentPosition == targetPosition) {
        Serial.println(" LLEGÓ AL DESTINO!");
        isMoving = false;
        delay(200);
        return true;
      }
      
      // Retroceder un poco y continuar
      if (currentDirectionForward) {
        motorLeft(100);  // Retroceder
        delay(30);
        motorStop();
        delay(100);
        motorRight(MOTOR_SPEED);  // Seguir adelante
      } else {
        motorRight(100);  // Retroceder
        delay(30);
        motorStop();
        delay(100);
        motorLeft(MOTOR_SPEED);  // Seguir atrás
      }
    }
  }
  
  // Reset flag de sensor después de 200ms
  if (sensorActivated && (now - sensorActivationTime > 200)) {
    sensorActivated = false;
  }
  
  //  VERIFICAR LLEGADA 
  if (currentPosition == targetPosition) {
    Serial.print("LLEGÓ A POS ");
    Serial.println(currentPosition);
    motorStop();
    isMoving = false;
    delay(200);
    return true;
  }
  
  //  TIMEOUT 
  if (millis() - moveStartTime > MOVE_TIMEOUT_MS) {
    Serial.print("  TIMEOUT en pos ");
    Serial.println(currentPosition);
    motorStop();
    isMoving = false;
    return true;
  }
  
  return false;
}

//  OBTENER DIRECCIÓN ACTUAL 
bool getCurrentDirection() {
  return currentDirectionForward;
}