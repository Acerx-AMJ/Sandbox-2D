#include "util/math.hpp"

// Lerp function

float lerp(float a, float b, float t) {
   return a + (b - a) * t;
}
