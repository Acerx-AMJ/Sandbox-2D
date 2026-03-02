#ifndef UI_INPUT_HPP
#define UI_INPUT_HPP

#include <raylib.h>
#include <string>

struct Input {
   void update();
   void render() const;
   Rectangle normalizeRect() const;

   // Members

   Texture *texture = nullptr;
   Rectangle rectangle;
   std::string text, fallback, keybind;

   bool hovering = false;
   bool typing = false;
   bool changed = false;
   bool wrapinput = true;

   int maxChars = 255;
   int counter = 0;
   float alpha = 1.0f;
};

#endif
