#ifndef UI_SCROLLFRAME_HPP
#define UI_SCROLLFRAME_HPP

#include <raylib.h>

struct Scrollframe {
   Rectangle rectangle;
   float scrollHeight = 0.f;
   float progress = 0.f, scrollbarHeight = 0.f, scrollbarY = 0.f;
   bool moving = false;

   void update();
   void render() const;

   bool inFrame(const Rectangle &rect) const;
   float getOffsetY() const;
};

#endif
