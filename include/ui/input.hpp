#ifndef UI_INPUT_HPP
#define UI_INPUT_HPP

#include <raylib.h>
#include <string>

struct Input {
   void update();
   void render(float dt) const;
   Rectangle normalizeRect() const;

   // Members

   Texture *texture = nullptr;
   Rectangle rectangle;
   std::string text, fallback;

   bool hovering = false;
   bool typing = false;
   bool changed = false;

   int maxChars = 255;
   int counter = 0;
};

#endif
