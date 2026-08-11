#pragma once
#include <raylib.h>
#include <string>

struct Input {
   void update(float dt);
   void render();
   Rectangle normalizeRect() const;

   // Members

   Texture *texture = nullptr;
   Rectangle rectangle;
   std::string text, fallback;

   bool hovering = false;
   bool typing = false;
   bool changed = false;
   bool wrapinput = true;
   bool rendercursor = false;

   int maxChars = 255;
   float counter = 0;
   float cursorcounter = 0;
   size_t cursor = 0;
   size_t prevsize = 0;
};
