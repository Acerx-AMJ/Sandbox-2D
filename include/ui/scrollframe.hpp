#ifndef UI_SCROLLFRAME_HPP
#define UI_SCROLLFRAME_HPP

#include <raylib.h>

// Constants

constexpr inline float scrollBarWidth = 56.667f;

// Scrollframe

struct Scrollframe {
   void update(float dt);
   void render() const;

   // Helper functions

   void setProgressBasedOnPosition(float positionY);
   float getOffsetY() const;
   bool inFrame(const Rectangle &rect) const;

   // Members

   Rectangle rectangle;
   bool moving = false;

   float scrollHeight = 0.f;
   float progress = 0.f;
   float scrollbarY = 0.f;
   float scrollbarHeight = 0.f;
};

#endif
