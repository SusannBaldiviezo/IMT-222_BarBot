#ifndef BUTTONS_H
#define BUTTONS_H

#include <Arduino.h>

struct Button {
  uint8_t pin;
  bool stableState;
  bool lastReading;
  bool pressedEvent;
  unsigned long lastChange;
};

extern const unsigned long DEBOUNCE_MS;

void initButton(Button &b, uint8_t pin);
void updateButton(Button &b, unsigned long now);

#endif
