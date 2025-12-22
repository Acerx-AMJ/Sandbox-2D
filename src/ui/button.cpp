#include "mngr/sound.hpp"
#include "ui/button.hpp"
#include "util/config.hpp"
#include "util/input.hpp"
#include "util/render.hpp"
#include <raymath.h>

void Button::update(float offsetY) {
   bool was_hovering = hovering;
   hovering = CheckCollisionPointRec({GetMouseX() + rectangle.width / 2.f, GetMouseY() + rectangle.height / 2.f + offsetY}, rectangle);
   if (!disabled) {
      down = hovering && IsMouseButtonDown(MOUSE_LEFT_BUTTON);
      clicked = hovering && isMousePressedUI(MOUSE_LEFT_BUTTON);
   } else {
      down = clicked = false;
   }

   if (hovering) {
      setMouseOnUI(true);
   }

   if (down) {
      scale = std::max(scale * 1.f - GetFrameTime(), buttonScaleMin);
   } else if (hovering) {
      scale = std::min(scale * 1.f + GetFrameTime(), buttonScaleMax);
   } else if (scale != 1.f) {
      scale = (scale < 1.f ? std::min(1.f, scale * 1.f + GetFrameTime()) : std::max(1.f, scale * 1.f - GetFrameTime()));
   }

   if (!was_hovering && hovering) {
      playSound("hover");
   }

   if (clicked) {
      playSound("click");
   }
}

void Button::render(float offsetY) const {
   if (texture) {
      drawTexture(*texture, {rectangle.x, rectangle.y - offsetY}, Vector2Scale({rectangle.width, rectangle.height}, scale), 0, (disabled ? buttonDisabledColor : WHITE));
   }
   drawText({rectangle.x, rectangle.y - offsetY}, text.c_str(), 35 * scale, (disabled ? buttonDisabledColor : WHITE));
}

Rectangle Button::normalizeRect() const {
   return {rectangle.x - rectangle.width / 2.f, rectangle.y - rectangle.height / 2.f, rectangle.width, rectangle.height};
}
