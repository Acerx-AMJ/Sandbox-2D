#include "mngr/resource.hpp"
#include "mngr/sound.hpp"
#include "ui/input.hpp"
#include "util/config.hpp"
#include "util/format.hpp"
#include "util/position.hpp"
#include "util/render.hpp"
#include <raymath.h>
#include <cmath>

void Input::update() {
   bool wasTyping = typing;
   hovering = CheckCollisionPointRec(GetMousePosition(), rectangle);

   if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
      typing = (hovering && !typing);
   }

   if ((IsKeyReleased(KEY_ENTER))) {
      typing = false;
   }

   if (typing) {
      std::size_t previous = text.size();
      if ((IsKeyPressed(KEY_BACKSPACE) || IsKeyPressedRepeat(KEY_BACKSPACE) || IsKeyPressed(KEY_DELETE) || IsKeyPressedRepeat(KEY_DELETE)) && !text.empty()) {
         text.pop_back();
      }

      char pressed = GetCharPressed();
      while (pressed != 0 && text.size() < (std::size_t)maxChars) {
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
   unsigned char value = (typing ? std::sin(counter * inputTextFadeSpeed) * inputTextFadeValue + inputTextFadeMin : 255);
   std::string wrapped = text;
   wrapText(wrapped, rectangle.width - inputTextWrapPadding, 35, 1);

   drawTextureNoOrigin(getTexture("button"), {rectangle.x, rectangle.y}, {rectangle.width, rectangle.height});
   drawText(Vector2Add({rectangle.x, rectangle.y}, getOrigin({rectangle.width, rectangle.height})), (text.empty() ? fallback : wrapped).c_str(), 35, Color{value, value, value, 255});
}
