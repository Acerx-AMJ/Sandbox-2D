#include "ui/checkbox.hpp"
#include "util/input.hpp"
#include "util/render.hpp"
#include "mngr/resource.hpp"
#include "mngr/sound.hpp"

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
}
