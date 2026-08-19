#pragma once
#include <raylib.h>

struct Bar {
   void init(Texture2D texture, Vector2 origin, Color foregroundTint, Color backgroundTint);
   void update(float alpha);
   void render();

   // members

   Texture2D texture = {0};
   Rectangle rect = {0, 0, 0, 0};
   Vector2 origin = {0.5f, 0.5f};
   Color foregroundTint = WHITE;
   Color backgroundTint = WHITE;

   float progress = 1.0f;
   float progressInterpolation = 1.0f;
};
