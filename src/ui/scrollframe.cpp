#include "mngr/resource.hpp"
#include "ui/scrollframe.hpp"
#include "util/config.hpp"
#include "util/input.hpp"
#include "util/math.hpp"
#include "util/render.hpp"
#include <cmath>

void Scrollframe::update() {
   float scrollFactor = GetMouseWheelMove();

   if (CheckCollisionPointRec(GetMousePosition(), rectangle) && scrollFactor != 0.f) {
      progress = clamp(progress + scrollFactor * scrollSpeed * GetFrameTime() * (rectangle.height / scrollHeight), 0.f, 1.f);
   } else if (CheckCollisionPointRec(GetMousePosition(), {rectangle.x + rectangle.width - scrollBarWidth, scrollbarY, scrollBarWidth, scrollbarHeight})) {
      setMouseOnUI(true);
      moving = IsMouseButtonDown(MOUSE_BUTTON_LEFT);
   }
   
   if (!IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
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

void Scrollframe::render() const {
   drawTextureNoOrigin(getTexture("scrollframe"), {rectangle.x, rectangle.y}, {rectangle.width, rectangle.height});
   drawTextureNoOrigin(getTexture("scrollbar"), {rectangle.x + rectangle.width - scrollBarWidth, scrollbarY}, {scrollBarWidth, scrollbarHeight});
}

// Other functions

bool Scrollframe::inFrame(const Rectangle &rect) const {
   float top = rectangle.y + getOffsetY();
   return rectangle.x <= rect.x && rectangle.x + rectangle.width >= rect.x + rect.width && top <= rect.y && top + rectangle.height >= rect.y + rect.height;
}

float Scrollframe::getOffsetY() const {
   return (scrollHeight - rectangle.height) * progress;
}
