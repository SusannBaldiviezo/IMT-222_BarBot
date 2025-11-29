#include "hardware.h"
#include <Arduino.h>
#include <Wire.h>

// ---- Definición de pines ----
const uint8_t PIN_SERVO         = 18;
const uint8_t PIN_MOTOR         = 19;
const uint8_t PIN_HOME          = 4;
const uint8_t PIN_SLOT          = 5;
const uint8_t PIN_BTN_UP        = 12;
const uint8_t PIN_BTN_DOWN      = 14;
const uint8_t PIN_BTN_OK        = 27;
const uint8_t PIN_LED_HEARTBEAT = 2;   // LED onboard o LED externo

// Ángulos del servo “lógico”
const int SERVO_REST_ANGLE  = 20;
const int SERVO_PRESS_ANGLE = 110;

// LCD I2C
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Botones globales
Button btnUp;
Button btnDown;
Button btnOk;

// =================== SERVO (pulsos manuales tipo RC) ===================
//
// El servo de Wokwi (y uno real) espera pulsos de:
//   - ~500 us  → 0°
//   - ~1500 us → 90°
//   - ~2400 us → 180°
// repetidos cada 20 ms.
// Aquí generamos ~25 pulsos cada vez que cambiamos de ángulo,
// suficiente para que el servo se mueva en la simulación.
//

void servoInit() {
  pinMode(PIN_SERVO, OUTPUT);
  servoSetAngle(SERVO_REST_ANGLE);  // posición inicial de reposo
}

void servoSetAngle(int angle) {
  if (angle < 0)   angle = 0;
  if (angle > 180) angle = 180;

  // 0° -> 500 us, 180° -> 2400 us
  uint16_t pulseUs = map(angle, 0, 180, 500, 2400);

  // Enviar ~25 pulsos (≈0.5 s) para que el servo llegue a la posición
  for (int i = 0; i < 25; i++) {
    digitalWrite(PIN_SERVO, HIGH);
    delayMicroseconds(pulseUs);
    digitalWrite(PIN_SERVO, LOW);
    delayMicroseconds(20000 - pulseUs);  // periodo total 20 ms
  }
}

// =================== INICIALIZACIÓN DE HARDWARE ===================

void hardwareInit() {
  Wire.begin();

  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("BarBot IMT");
  lcd.setCursor(0, 1);
  lcd.print("Iniciando...");

  // Servo
  servoInit();

  // Motor del carrusel
  pinMode(PIN_MOTOR, OUTPUT);
  digitalWrite(PIN_MOTOR, LOW);

  // Finales de carrera
  pinMode(PIN_HOME, INPUT_PULLUP);
  pinMode(PIN_SLOT, INPUT_PULLUP);

  // Botones con antirrebote
  initButton(btnUp,   PIN_BTN_UP);
  initButton(btnDown, PIN_BTN_DOWN);
  initButton(btnOk,   PIN_BTN_OK);

  // LED heartbeat (tarea FreeRTOS)
  pinMode(PIN_LED_HEARTBEAT, OUTPUT);
}

// =================== MOTOR ===================

void motorOn() {
  digitalWrite(PIN_MOTOR, HIGH);
}

void motorOff() {
  digitalWrite(PIN_MOTOR, LOW);
}

// =================== FINALES DE CARRERA ===================

bool readLimitHome() {
  // Activo en LOW (por INPUT_PULLUP)
  return digitalRead(PIN_HOME) == LOW;
}

bool readLimitSlot() {
  // Activo en LOW (por INPUT_PULLUP)
  return digitalRead(PIN_SLOT) == LOW;
}
