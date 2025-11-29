#ifndef RECIPES_H
#define RECIPES_H

#include <Arduino.h>

struct Recipe {
  const char *name;
  uint8_t bottlePos1;   // primera botella (ej. licor)
  uint32_t pourMs1;     // tiempo de servido 1
  uint8_t bottlePos2;   // segunda botella (ej. gaseosa)
  uint32_t pourMs2;     // tiempo de servido 2
};

extern const Recipe recipes[];
extern const uint8_t NUM_RECIPES;

#endif
