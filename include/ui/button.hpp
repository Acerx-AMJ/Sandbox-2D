#ifndef UTIL_BUTTON_HPP
#define UTIL_BUTTON_HPP

#include <string>
#include <raylib.h>

// Button

struct Button {
   Rectangle rectangle;
   std::string text;
   bool hovering = false, down = false, clicked = false;
   float scale = 1;

   // Functions

   void update();
   void render();
};

#endif
