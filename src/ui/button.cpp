#include "mngr/resource.hpp"
#include "mngr/sound.hpp"
#include "ui/button.hpp"
#include "util/render.hpp"

// Update function

void Button::update() {
   bool was_hovering = hovering;
   hovering = CheckCollisionPointRec({GetMouseX() + rectangle.width / 2.f, GetMouseY() + rectangle.height / 2.f}, rectangle);
   down = hovering and IsMouseButtonDown(MOUSE_LEFT_BUTTON);
   clicked = hovering and IsMouseButtonReleased(MOUSE_LEFT_BUTTON);

   if (down) {
      scale = std::max(scale * .975f, .9f);
   } else if (hovering) {
      scale = std::min(scale * 1.025f, 1.1f);
   } else if (scale != 1.f) {
      scale = (scale < 1.f ? std::min(1.f, scale * 1.025f) : std::max(1.f, scale * .975f));
   }

   if (not was_hovering and hovering) {
      SoundManager::get().play("hover");
   }

   if (clicked) {
      SoundManager::get().play("click");
   }
}

// Render function

void Button::render() {
   drawTexture(ResourceManager::get().getTexture("button"), {rectangle.x, rectangle.y}, {rectangle.width * scale, rectangle.height * scale});
   drawText({rectangle.x, rectangle.y}, text.c_str(), 35 * scale);
}
