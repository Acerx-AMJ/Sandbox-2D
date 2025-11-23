#include "mngr/resource.hpp"
#include "mngr/sound.hpp"
#include "ui/button.hpp"
#include "util/render.hpp"

// Update function

void Button::update(float offsetY) {
   bool was_hovering = hovering;
   hovering = CheckCollisionPointRec({GetMouseX() + rectangle.width / 2.f, GetMouseY() + rectangle.height / 2.f - offsetY}, rectangle);
   down = hovering and IsMouseButtonDown(MOUSE_LEFT_BUTTON);
   clicked = hovering and IsMouseButtonReleased(MOUSE_LEFT_BUTTON);

   if (down) {
      scale = std::max(scale * 1.f - GetFrameTime(), .9f);
   } else if (hovering) {
      scale = std::min(scale * 1.f + GetFrameTime(), 1.1f);
   } else if (scale != 1.f) {
      scale = (scale < 1.f ? std::min(1.f, scale * 1.f + GetFrameTime()) : std::max(1.f, scale * 1.f - GetFrameTime()));
   }

   if (not was_hovering and hovering) {
      SoundManager::get().play("hover");
   }

   if (clicked) {
      SoundManager::get().play("click");
   }
}

// Render function

void Button::render(float offsetY) {
   drawTexture(ResourceManager::get().getTexture("button"), {rectangle.x, rectangle.y - offsetY}, {rectangle.width * scale, rectangle.height * scale});
   drawText({rectangle.x, rectangle.y - offsetY}, text.c_str(), 35 * scale);
}

// Other functions

Rectangle Button::normalizeRect() {
   return {rectangle.x - rectangle.width / 2.f, rectangle.y - rectangle.height / 2.f, rectangle.width, rectangle.height};
}
