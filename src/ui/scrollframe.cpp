#include "ui/scrollframe.hpp"
#include "mngr/resource.hpp"
#include "util/math.hpp"
#include "util/render.hpp"
#include <cmath>

// Update function

void Scrollframe::update() {
   float scrollFactor = GetMouseWheelMove();

   if (CheckCollisionPointRec(GetMousePosition(), rectangle) and not floatIsZero(scrollFactor)) {
      progress = clamp(progress + scrollFactor * 15.f * GetFrameTime() * (rectangle.height / scrollHeight), 0.f, 1.f);
   } else if (CheckCollisionPointRec(GetMousePosition(), {rectangle.x + rectangle.width - 50.f, scrollbarY, 50.f, scrollbarHeight})) {
      moving = IsMouseButtonDown(MOUSE_BUTTON_LEFT);
   }
   
   if (not IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
      moving = false;
   }
   
   if (moving) {
      float local = GetMouseY() - rectangle.y;
      progress = clamp(local / rectangle.height, 0.f, 1.f);
   }

   scrollbarHeight = rectangle.height * (rectangle.height / scrollHeight);
   scrollbarY = rectangle.y + (rectangle.height - scrollbarHeight) * progress;
}

// Render function

void Scrollframe::render() {
   drawTextureNoOrigin(getTexture("scrollframe"), {rectangle.x, rectangle.y}, {rectangle.width, rectangle.height});
   drawTextureNoOrigin(getTexture("scrollbar"), {rectangle.x + rectangle.width - 56.666f, scrollbarY}, {56.666f, scrollbarHeight});
}

// Other functions

bool Scrollframe::inFrame(const Rectangle& rect) {
   float top = rectangle.y + getOffsetY();
   return rectangle.x <= rect.x and rectangle.x + rectangle.width >= rect.x + rect.width
      and top <= rect.y and top + rectangle.height >= rect.y + rect.height;
}

float Scrollframe::getOffsetY() {
   return (scrollHeight - rectangle.height) * progress;
}
