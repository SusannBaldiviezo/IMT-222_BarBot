#include "fsm.h"
#include "hardware.h"
#include "buttons.h"

// ----------------- Variables de estado global -----------------
BarbotState currentState;
uint8_t currentPos       = 0;   // posición actual 0..5
uint8_t targetPos        = 0;   // posición destino
int8_t  selectedRecipe   = 0;   // índice de receta seleccionada

// Tiempo de inicio de cada estado
static unsigned long stateStart = 0;

// Para detectar “entry” en cada estado
static BarbotState lastStateForEntry = ST_IDLE;

// Para detectar flancos en los finales de carrera
static bool lastSlotState = false;
static bool lastHomeState = false;

static const uint8_t NUM_POS = 6;
static const unsigned long MOVE_TIMEOUT_MS = 10000; // 10 s de seguridad

void goToState(BarbotState newState) {
  currentState = newState;
  stateStart   = millis();
}

void fsmInit() {
  currentPos        = 0;
  targetPos         = 0;
  selectedRecipe    = 0;
  currentState      = ST_IDLE;
  stateStart        = millis();
  lastStateForEntry = (BarbotState)255;  // valor imposible para forzar primer entry
  lastSlotState     = false;
  lastHomeState     = false;
}

void lcdShowMenu() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("BarBot IMT");
  lcd.setCursor(0, 1);
  lcd.print(selectedRecipe + 1);
  lcd.print(") ");
  lcd.print(recipes[selectedRecipe].name);
}

void fsmStep() {
  unsigned long now = millis();

  // Actualizamos botones (antirrebote)
  updateButton(btnUp,   now);
  updateButton(btnDown, now);
  updateButton(btnOk,   now);

  // Leemos finales de carrera
  bool homeState = readLimitHome();
  bool slotState = readLimitSlot();

  // Entry de estado
  bool entering = (currentState != lastStateForEntry);
  if (entering) {
    lastStateForEntry = currentState;
  }

  // ----------------- Máquina de estados -----------------
  switch (currentState) {

    case ST_IDLE: {
      if (entering) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("BarBot listo");
        lcd.setCursor(0, 1);
        lcd.print("Iniciando...");
      }

      // Podríamos esperar botón, pero aquí pasamos solos tras 1 s
      if (now - stateStart > 1000) {
        goToState(ST_BOOT);
      }
      break;
    }

    case ST_BOOT: {
      if (entering) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("BarBot IMT");
        lcd.setCursor(0, 1);
        lcd.print("Inicializando");
      }
      // Después de 1s pasamos a homing
      if (now - stateStart > 1000) {
        goToState(ST_HOME);
      }
      break;
    }

    case ST_HOME: {
      if (entering) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Homing carrusel");
        motorOn();
      }

      // Flanco del HOME → llegó a 0
      if (homeState && !lastHomeState) {
        motorOff();
        currentPos = 0;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Pos 0 lista");
        selectedRecipe = 0;
        goToState(ST_MENU);
      }
      break;
    }

    case ST_MENU: {
      if (entering) {
        lcdShowMenu();
      }

      // Navegar recetas
      if (btnUp.pressedEvent) {
        selectedRecipe--;
        if (selectedRecipe < 0) {
          selectedRecipe = NUM_RECIPES - 1;
        }
        lcdShowMenu();
      }

      if (btnDown.pressedEvent) {
        selectedRecipe++;
        if (selectedRecipe >= (int8_t)NUM_RECIPES) {
          selectedRecipe = 0;
        }
        lcdShowMenu();
      }

      if (btnOk.pressedEvent) {
        // Comenzar por la primera botella
        targetPos = recipes[selectedRecipe].bottlePos1;
        goToState(ST_MOVE1);
      }
      break;
    }

    case ST_MOVE1: {
      if (entering) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Moviendo a pos");
        lcd.setCursor(0, 1);
        lcd.print(targetPos);
        motorOn();
      }

      // Flanco en SLOT → avanzó una posición
      if (slotState && !lastSlotState) {
        currentPos = (currentPos + 1) % NUM_POS;
        Serial.print("Posicion (1): ");
        Serial.println(currentPos);
      }

      if (currentPos == targetPos) {
        motorOff();
        goToState(ST_DISPENSE1);
      }

      // Timeout de seguridad (simulación)
      if (now - stateStart > MOVE_TIMEOUT_MS) {
        motorOff();
        Serial.println("Timeout en ST_MOVE1, pasando a ST_DISPENSE1");
        goToState(ST_DISPENSE1);
      }
      break;
    }

    case ST_DISPENSE1: {
      if (entering) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Sirviendo 1/2");
        servoSetAngle(SERVO_PRESS_ANGLE);
      }

      uint32_t pourMs = recipes[selectedRecipe].pourMs1;
      if (now - stateStart >= pourMs) {
        servoSetAngle(SERVO_REST_ANGLE);
        // Preparar segunda botella
        targetPos = recipes[selectedRecipe].bottlePos2;
        goToState(ST_MOVE2);
      }
      break;
    }

    case ST_MOVE2: {
      if (entering) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Moviendo a pos");
        lcd.setCursor(0, 1);
        lcd.print(targetPos);
        motorOn();
      }

      // Flanco en SLOT → avanzó una posición
      if (slotState && !lastSlotState) {
        currentPos = (currentPos + 1) % NUM_POS;
        Serial.print("Posicion (2): ");
        Serial.println(currentPos);
      }

      if (currentPos == targetPos) {
        motorOff();
        goToState(ST_DISPENSE2);
      }

      // Timeout de seguridad
      if (now - stateStart > MOVE_TIMEOUT_MS) {
        motorOff();
        Serial.println("Timeout en ST_MOVE2, pasando a ST_DISPENSE2");
        goToState(ST_DISPENSE2);
      }
      break;
    }

    case ST_DISPENSE2: {
      if (entering) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Sirviendo 2/2");
        servoSetAngle(SERVO_PRESS_ANGLE);
      }

      uint32_t pourMs = recipes[selectedRecipe].pourMs2;
      if (now - stateStart >= pourMs) {
        servoSetAngle(SERVO_REST_ANGLE);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Tu trago esta");
        lcd.setCursor(0, 1);
        lcd.print("listo!");
        goToState(ST_DONE);
      }
      break;
    }

    case ST_DONE: {
      if (entering) {
        // Mensaje ya se escribió en ST_DISPENSE2
      }

      // Después de 3s volvemos al menú
      if (now - stateStart > 3000) {
        goToState(ST_MENU);
      }
      break;
    }

    default:
      goToState(ST_HOME);
      break;
  }

  // Actualizamos memoria para flancos
  lastSlotState = slotState;
  lastHomeState = homeState;
}
