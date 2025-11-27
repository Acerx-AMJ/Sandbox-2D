#include <cmath>
#include "mngr/resource.hpp"
#include "ui/input.hpp"
#include "mngr/sound.hpp"
#include "util/render.hpp"
#include "util/text.hpp"

// Update function

void Input::update() {
   bool wasTyping = typing;
   hovering = CheckCollisionPointRec(GetMousePosition(), rectangle);

   if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
      typing = (hovering and not typing);
   }

   if ((IsKeyReleased(KEY_ENTER))) {
      typing = false;
   }

   if (typing) {
      auto previous = text.size();
      if ((IsKeyPressed(KEY_BACKSPACE) or IsKeyPressedRepeat(KEY_BACKSPACE) or IsKeyPressed(KEY_DELETE) or IsKeyPressedRepeat(KEY_DELETE)) and not text.empty()) {
         text.pop_back();
      }

      char pressed = GetCharPressed();
      while (pressed != 0 and text.size() < maxChars) {
         text += pressed;
         pressed = GetCharPressed();
      }

      if (text.size() != previous) {
         playSound("hover");
      }
   }

   if (wasTyping != typing) {
      playSound("click");
   }
   ++counter;
}

// Render function

void Input::render() {
   unsigned char value = (typing ? std::sin(counter * .3f) * 65 + 190 : 255);
   std::string wrapped = text;
   wrapText(wrapped, rectangle.width - 10.f, 35, 1);

   drawTextureNoOrigin(getTexture("button"), {rectangle.x, rectangle.y}, {rectangle.width, rectangle.height});
   drawText({rectangle.x + rectangle.width / 2.f, rectangle.y + rectangle.height / 2.f}, (text.empty() ? defaultText : wrapped).c_str(), 35, Color{value, value, value, 255});
}
