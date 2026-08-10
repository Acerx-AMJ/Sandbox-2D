#include "mngr/input.hpp"
#include "ui/checkbox.hpp"
#include "ui/keybindIndicator.hpp"
#include "util/render.hpp"
#include "SRU/audio.hpp"
#include "SRU/assets.hpp"

void CheckBox::update() {
   if (CheckCollisionPointRec(GetMousePosition(), rectangle)) {
      setMouseOnUI(true);

      if (isMousePressedUI(MOUSE_BUTTON_LEFT)) {
         checked = !checked;
         playSound("click");
      }
   }
}

void CheckBox::render() const {
   Texture &texture = getTexture(checked ? "checkbox_checked" : "checkbox_unchecked");
   drawTextureNoOrigin(texture, {rectangle.x, rectangle.y}, {rectangle.width, rectangle.height});
   drawKeybindIndicator(keybind, {rectangle.x + rectangle.width, rectangle.y});
}
