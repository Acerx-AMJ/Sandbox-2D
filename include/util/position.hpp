#ifndef UTIL_POSITION_HPP
#define UTIL_POSITION_HPP

#include <raylib.h>

Vector2 getScreenSize();
Vector2 getScreenCenter(const Vector2 &offset = {0, 0});

float getWidthRatio();
float getHeightRatio();
float getMinRatio();
float getFontSize(float size);

Vector2 applyResponsiveness(const Vector2 &size);
Vector2 applyCubicResponsiveness(const Vector2 &size);

Vector2 getOrigin(const Vector2 &size);
Vector2 getOrigin(const char *text, float fontSize, float spacing);

Rectangle getBox(const Texture &texture);
Rectangle getCameraBounds(const Camera2D &camera);

Vector2 lerp(const Vector2 &a, const Vector2 &b, float t);
Color lerp(const Color &a, const Color &b, float t);

#endif
