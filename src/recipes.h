#include "recipes.h"

// Ejemplo de asignación de posiciones (0..5):
// 0 = Ron
// 1 = Fernet
// 2 = Singani
// 3 = Cola
// 4 = Sprite
// 5 = Vodka

const Recipe recipes[] = {
  { "Ron + Cola",        0, 3000, 3, 4000 },
  { "Fernet + Cola",     1, 3000, 3, 4000 },
  { "Singani + Sprite",  2, 3000, 4, 4000 },
  { "Vodka + Sprite",    5, 3000, 4, 4000 },
};

const uint8_t NUM_RECIPES = sizeof(recipes) / sizeof(recipes[0]);
