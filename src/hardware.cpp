#include "hardware.h"

// ============ DEFINICIÓN DE PINES ============
const uint8_t PIN_MOTOR_REN  = 25;
const uint8_t PIN_MOTOR_LEN  = 26;
const uint8_t PIN_MOTOR_RPWM = 27;
const uint8_t PIN_MOTOR_LPWM = 14;

// SOLO 2 SENSORES
const uint8_t PIN_HOME    = 15;  // Sensor HOME
const uint8_t PIN_COUNTER = 32;  // Sensor CONTADOR

const uint8_t PIN_SERVO  = 5;
const int SERVO_REST_ANGLE     = 0;
const int SERVO_DISPENSE_ANGLE = 90;

const uint8_t PIN_BTN_UP   = 12;
const uint8_t PIN_BTN_DOWN = 13;
const uint8_t PIN_BTN_OK   = 4;

const uint8_t PIN_LED_HEARTBEAT = 19;
// LCD I2C
const uint8_t LCD_SDA = 21;  // GPIO 21 para SDA (común)
const uint8_t LCD_SCL = 22;  // GPIO 22 para SCL (común)

// ============ BOTONES GLOBALES ============
Button btnUp;
Button btnDown;
Button btnOk;

// ============ INICIALIZACIÓN ============
void hardwareInit() {
  Serial.begin(115200);
  Serial.println("Inicializando hardware ");
  // Inicializar I2C para LCD
  Wire.begin(LCD_SDA, LCD_SCL);
  delay(100);

  // BTS7960 - Motor
  pinMode(PIN_MOTOR_REN, OUTPUT);
  pinMode(PIN_MOTOR_LEN, OUTPUT);
  digitalWrite(PIN_MOTOR_REN, HIGH);  // Habilitar derecha
  digitalWrite(PIN_MOTOR_LEN, HIGH);  // Habilitar izquierda

  ledcAttach(PIN_MOTOR_RPWM, 1000, 8);  // 1kHz, 8 bits
  ledcAttach(PIN_MOTOR_LPWM, 1000, 8);
  motorStop();

  // FINALES DE CARRERA (SOLO 2)
  // INPUT_PULLUP: Activo en LOW (sensor presionado)
  pinMode(PIN_HOME, INPUT_PULLUP);     // HOME en GPIO 15
  pinMode(PIN_COUNTER, INPUT_PULLUP);  // CONTADOR en GPIO 32

  // Servo
  servoInit();

  // Botones
  initButton(btnUp,   PIN_BTN_UP);
  initButton(btnDown, PIN_BTN_DOWN);
  initButton(btnOk,   PIN_BTN_OK);

  // LED
  pinMode(PIN_LED_HEARTBEAT, OUTPUT);

  Serial.println("Hardware inicializado OK (2 sensores)");
}

// ============ MOTOR ============
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

// ============ FINALES DE CARRERA (2 SENSORES) ============
bool readLimitHome() {
  return digitalRead(PIN_HOME) == LOW;  // Activo en LOW
}

bool readLimitPos() {
  // Lee el sensor CONTADOR (único para todas las posiciones)
  return digitalRead(PIN_COUNTER) == LOW;  // Activo en LOW
}




// ============ SERVO ============
void servoInit() {
  pinMode(PIN_SERVO, OUTPUT);
  servoSetAngle(SERVO_REST_ANGLE);
}

void servoSetAngle(int angle) {
  angle = constrain(angle, 0, 180);
  
  // Mapeo: 0° = 500us, 90° = 1500us, 180° = 2400us
  uint16_t pulseUs = map(angle, 0, 180, 500, 2400);

  // Enviar 25 pulsos para que el servo se posicione
  for (int i = 0; i < 25; i++) {
    digitalWrite(PIN_SERVO, HIGH);
    delayMicroseconds(pulseUs);
    digitalWrite(PIN_SERVO, LOW);
    delayMicroseconds(20000 - pulseUs);
  }
}