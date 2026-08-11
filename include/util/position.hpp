#pragma once
#include <raylib.h>

Vector2 getScreenSize();
Vector2 getScreenCenter(const Vector2 &offset = {0, 0});

float getWidthRatio();
float getHeightRatio();
float getMinRatio();
float getFontSize(float size);

Vector2 applyResponsiveness(const Vector2 &size);
Vector2 applyCubicResponsiveness(const Vector2 &size);
