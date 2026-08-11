#include "util/position.hpp"
#include <raymath.h>

// Responsive design

Vector2 getScreenSize() {
   return {(float)GetScreenWidth(), (float)GetScreenHeight()};
}

Vector2 getScreenCenter(const Vector2 &offset) {
   return Vector2Add(getScreenSize() / 2, offset);
}

float getWidthRatio() {
   return GetScreenWidth() / 1920.0f; // target resolution AKA my resolution so I can style the UI on
                                      // my machine and it looks the same everywhere
}

float getHeightRatio() {
   return GetScreenHeight() / 1080.0f;
}

// for cubic things
float getMinRatio() {
   return fmin(getWidthRatio(), getHeightRatio());
}

float getFontSize(float size) {
   return fmin(getWidthRatio(), getHeightRatio()) * size;
}

Vector2 applyResponsiveness(const Vector2 &size) {
   return {size.x * getWidthRatio(), size.y * getHeightRatio()};
}

Vector2 applyCubicResponsiveness(const Vector2 &size) {
   float cubicRatio = getMinRatio();
   return {size.x * cubicRatio, size.y * cubicRatio};
}
