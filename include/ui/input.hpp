#ifndef UI_INPUT_HPP
#define UI_INPUT_HPP

#include <raylib.h>
#include <string>

struct Input {
   Rectangle rectangle;
   std::string text, fallback;
   bool hovering = false, typing = false;
   int maxChars = 255, counter = 0;

   void update();
   void render() const;
};

#endif
