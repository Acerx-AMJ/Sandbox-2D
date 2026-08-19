#pragma once
#include <raylib.h>
#include <string>

struct Input {
   void init(Font font, Texture texture, Vector2 origin, int maxChars, const std::string &fallback);
   void update(float dt);
   void render();

   // Members

   Font font;
   Texture texture = {0};
   Rectangle rect = {0, 0, 0, 0};
   Vector2 origin = {0.5f, 0.5f};
   Vector2 textOrigin = {0.5f, 0.5f};
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
