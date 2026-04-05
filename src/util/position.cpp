#include "mngr/resource.hpp"
#include "util/position.hpp"
#include <raymath.h>

// Responsive design

Vector2 getScreenSize() {
   return {(float)GetScreenWidth(), (float)GetScreenHeight()};
}

Vector2 getScreenCenter(const Vector2 &offset) {
   return Vector2Add(getOrigin(getScreenSize()), offset);
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

// Helper functions

Vector2 getOrigin(const Vector2 &size) {
   return Vector2Scale(size, 0.5f);
}

Vector2 getOrigin(const char *text, float fontSize, float spacing) {
   return getOrigin(MeasureTextEx(getFont("andy"), text, fontSize, spacing));
}

Rectangle getBox(const Texture &texture) {
   return {0, 0, (float)texture.width, (float)texture.height};
}

Rectangle getCameraBounds(const Camera2D &camera) {
   Vector2 pos = GetScreenToWorld2D({0, 0}, camera);
   Vector2 size = Vector2Scale(getScreenSize(), 1.f / camera.zoom);
   return {pos.x, pos.y, size.x, size.y};
}

Vector2 lerp(const Vector2 &a, const Vector2 &b, float t) {
   return Vector2Add(a, Vector2Scale(Vector2Subtract(b, a), Clamp(t, 0, 1)));
}

Color lerp(const Color &a, const Color &b, float t) {
   float i = 1.0f - t;
   return Color{
      static_cast<unsigned char>(a.r * t + b.r * i),
      static_cast<unsigned char>(a.g * t + b.g * i),
      static_cast<unsigned char>(a.b * t + b.b * i),
      a.a,
   };
}
