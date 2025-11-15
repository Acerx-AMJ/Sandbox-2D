#ifndef UTIL_POSITION_HPP
#define UTIL_POSITION_HPP

#include <raylib.h>

// Screen position functions

Vector2 getScreenSize(float offsetX = 0, float offsetY = 0);
Vector2 getScreenCenter(float offsetX = 0, float offsetY = 0);

// Origin functions

Vector2 getOrigin(const Vector2& size);
Vector2 getOrigin(float x, float y);
Vector2 getOrigin(const char* text, float fontSize, float spacing);

// Texture functions

Vector2 getSize(const Texture& texture);
Rectangle getBox(const Texture& texture);

// Camera functions

Rectangle getCameraBounds(const Camera2D& camera);

#endif
