#include "hardware.h"

// DEFINICIÓN DE PINES
const uint8_t PIN_MOTOR_REN  = 18;
const uint8_t PIN_MOTOR_LEN  = 17;
const uint8_t PIN_MOTOR_RPWM = 15;
const uint8_t PIN_MOTOR_LPWM = 16;

// 2 SENSORES
const uint8_t PIN_HOME    = 25;  // HOME sensor
const uint8_t PIN_COUNTER = 26;  // ÚNICO sensor contador

// SERVO
const uint8_t PIN_SERVO  = 27;
const int SERVO_REST_ANGLE     = 0;
const int SERVO_DISPENSE_ANGLE = 180;

// 4 BOTONES
const uint8_t PIN_BTN_UP     = 32;
const uint8_t PIN_BTN_DOWN   = 33;
const uint8_t PIN_BTN_OK     = 12;
const uint8_t PIN_BTN_RESET  = 13;

// RELÉ (FOCO 220V)
const uint8_t PIN_RELE_FOCO = 19;

// LCD I2C
const uint8_t LCD_SDA = 21;
const uint8_t LCD_SCL = 22;

// BOTONES GLOBALES
Button btnUp;
Button btnDown;
Button btnOk;
Button btnReset;

// VARIABLES PARA DETECCIÓN ÚNICA DE PULSOS
static bool sensorHomeLastState = true;    // HIGH por PULLUP
static bool sensorCounterLastState = true; // HIGH por PULLUP

// INICIALIZACIÓN
void hardwareInit() {
  Serial.begin(115200);
  Serial.println("Inicializando hardware...");
  
  Wire.begin(LCD_SDA, LCD_SCL);
  delay(100);

  // BTS7960
  pinMode(PIN_MOTOR_REN, OUTPUT);
  pinMode(PIN_MOTOR_LEN, OUTPUT);
  digitalWrite(PIN_MOTOR_REN, HIGH);
  digitalWrite(PIN_MOTOR_LEN, HIGH);

  ledcAttach(PIN_MOTOR_RPWM, 1000, 8);
  ledcAttach(PIN_MOTOR_LPWM, 1000, 8);
  motorStop();

  // 2 SENSORES
  pinMode(PIN_HOME, INPUT_PULLUP);
  pinMode(PIN_COUNTER, INPUT_PULLUP);

  // Servo
  servoInit();

  // 4 BOTONES
  initButton(btnUp, PIN_BTN_UP);
  initButton(btnDown, PIN_BTN_DOWN);
  initButton(btnOk, PIN_BTN_OK);
  initButton(btnReset, PIN_BTN_RESET);

  // RELÉ - Iniciar apagado
  pinMode(PIN_RELE_FOCO, OUTPUT);
  digitalWrite(PIN_RELE_FOCO, LOW);

  Serial.println("Hardware inicializado OK");
  Serial.println("Sistema con 2 sensores: HOME + CONTADOR");
}

// MOTOR
void motorRight(uint8_t speed) {
  speed = constrain(speed, 0, 255);
  ledcWrite(PIN_MOTOR_RPWM, speed);
  ledcWrite(PIN_MOTOR_LPWM, 0);
}

void motorLeft(uint8_t speed) {
  speed = constrain(speed, 0, 255);
  ledcWrite(PIN_MOTOR_RPWM, 0);
  ledcWrite(PIN_MOTOR_LPWM, speed);
}

void motorStop() {
  ledcWrite(PIN_MOTOR_RPWM, 0);
  ledcWrite(PIN_MOTOR_LPWM, 0);
}

// FINALES DE CARRERA - LECTURA DIRECTA
bool readLimitHome() {
  return digitalRead(PIN_HOME) == LOW;
}

bool readLimitCounter() {
  return digitalRead(PIN_COUNTER) == LOW;
}

// DETECCIÓN DE PULSO ÚNICO
bool readHomeSinglePulse() {
  bool currentState = digitalRead(PIN_HOME);
  bool detected = false;
  
  // Detectar flanco de bajada (HIGH -> LOW)
  if (currentState == LOW && sensorHomeLastState == HIGH) {
    detected = true;
    Serial.println("🏠 PULSO HOME DETECTADO (flanco bajada)");
  }
  
  sensorHomeLastState = currentState;
  return detected;
}

bool readCounterSinglePulse() {
  bool currentState = digitalRead(PIN_COUNTER);
  bool detected = false;
  
  // Detectar flanco de bajada (HIGH -> LOW)
  if (currentState == LOW && sensorCounterLastState == HIGH) {
    detected = true;
    Serial.println("🔢 PULSO COUNTER DETECTADO (flanco bajada)");
  }
  
  sensorCounterLastState = currentState;
  return detected;
}

// SERVO
void servoInit() {
  pinMode(PIN_SERVO, OUTPUT);
  
  // Inicializar servo en posición de reposo
  servoSetAngle(SERVO_REST_ANGLE);
  
  Serial.println("Servo inicializado");
}

void servoSetAngle(int angle) {
  angle = constrain(angle, 0, 180);
  uint16_t pulseUs = map(angle, 0, 180, 500, 2400);

  // 50 pulsos para MÁS FUERZA
  for (int i = 0; i < 50; i++) {
    digitalWrite(PIN_SERVO, HIGH);
    delayMicroseconds(pulseUs);
    digitalWrite(PIN_SERVO, LOW);
    delayMicroseconds(20000 - pulseUs);
  }
}

// SERVO CON PRESIÓN
void servoPress() {
  Serial.println("🔘 SERVO PRESIONANDO...");
  
  // 1. Ir a posición de dispensación
  servoSetAngle(SERVO_DISPENSE_ANGLE);
  delay(500);
  
  // 2. Mantener presión por 1 segundo
  delay(1000);
  
  // 3. Volver a reposo
  servoSetAngle(SERVO_REST_ANGLE);
  delay(300);
  
  Serial.println("🔘 SERVO LIBERADO");
}

// RELÉ
void releOn() {
  digitalWrite(PIN_RELE_FOCO, HIGH);
  Serial.println("💡 FOCO ENCENDIDO");
}

void releOff() {
  digitalWrite(PIN_RELE_FOCO, LOW);
  Serial.println("💡 FOCO APAGADO");
}