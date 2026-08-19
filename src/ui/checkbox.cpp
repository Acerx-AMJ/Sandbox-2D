#include "mngr/input.hpp"
#include "ui/checkbox.hpp"
#include "ui/keybindIndicator.hpp"
#include "SRU/audio.hpp"
#include "SRU/render.hpp"
#include "SRU/util.hpp"

void CheckBox::init(Font font, Vector2 origin, const std::string &keybind) {
   this->font = font;
   this->origin = origin;
   this->keybind = keybind;
}

void CheckBox::update() {
   if (CheckCollisionPointRec(GetMousePosition(), R4bounds(rect, origin))) {
      setMouseOnUI(true);

      if (isMousePressedUI(MOUSE_BUTTON_LEFT)) {
         checked = !checked;
         playSound("click");
      }
   }
}

void CheckBox::render() {
   drawTexture((checked ? "checkbox_checked" : "checkbox_unchecked"), rect, origin);
   drawKeybindIndicator(font, keybind, R4anchor(rect, origin, TOP_RIGHT));
}
