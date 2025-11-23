#ifndef UI_SCROLLFRAME_HPP
#define UI_SCROLLFRAME_HPP

#include <raylib.h>

// Scrollframe

struct Scrollframe {
   Rectangle rectangle;
   float scrollHeight = 0.f;
   float progress = 0.f, scrollbarHeight = 0.f, scrollbarY = 0.f;
   bool moving = false;

   // Functions

   void update();
   void render();

   bool inFrame(const Rectangle& rect);
   float getOffsetY();
};

#endif
