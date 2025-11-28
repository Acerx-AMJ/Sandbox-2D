#ifndef UI_INPUT_HPP
#define UI_INPUT_HPP

#include <raylib.h>
#include <string>

// Input

struct Input {
   Rectangle rectangle;
   std::string text, defaultText;
   bool hovering = false, typing = false;
   int maxChars = 255, counter = 0;

   // Functions

   void update();
   void render();
};

#endif
