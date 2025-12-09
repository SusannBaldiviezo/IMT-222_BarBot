#include "buttons.h"

const unsigned long DEBOUNCE_MS = 30;

void initButton(Button &b, uint8_t pin) {
  b.pin = pin;
  pinMode(pin, INPUT_PULLUP);  // Activo en LOW
  b.stableState = HIGH;
  b.lastReading = HIGH;
  b.pressedEvent = false;
  b.lastChange = 0;
}

void updateButton(Button &b, unsigned long now) {
  bool reading = digitalRead(b.pin);

  if (reading != b.lastReading) {
    b.lastChange = now;
    b.lastReading = reading;
  }

  b.pressedEvent = false;

  if ((now - b.lastChange) > DEBOUNCE_MS) {
    if (reading != b.stableState) {
      b.stableState = reading;
      // Flanco de bajada = pulsaciónnn
      if (b.stableState == LOW) {
        b.pressedEvent = true;
      }
    }
  }
}