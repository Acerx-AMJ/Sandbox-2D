#include "util/input.hpp"
#include "mngr/sound.hpp"
#include <raylib.h>
#include <array>

static inline std::array<int, MOUSE_BUTTON_BACK + 1> mouseAlreadyPressed;
static inline std::array<int, MOUSE_BUTTON_BACK + 1> mouseDownOnUI;

void resetInput() {
   mouseAlreadyPressed.fill(false);
   mouseDownOnUI.fill(false);
}

void resetMouseUIInput(int mouse) {
   mouseAlreadyPressed[mouse] = false;
}

void setMouseOnUI(int mouse) {
   mouseDownOnUI[mouse] = true;
}

bool isMousePressedUI(int mouse) {
   bool isPressed = IsMouseButtonPressed(mouse);
   bool alreadyPressed = mouseAlreadyPressed[mouse];

   if (isPressed) {
      mouseAlreadyPressed[mouse] = true;
   }
   return !alreadyPressed && isPressed;
}

bool isMousePressedOutsideUI(int mouse) {
   return !mouseAlreadyPressed[mouse] && IsMouseButtonPressed(mouse);
}

bool isMouseDownOutsideUI(int mouse) {
   return !mouseDownOnUI[mouse] && IsMouseButtonDown(mouse);
}

bool handleKeyPressWithSound(int key) {
   bool isPressed = IsKeyPressed(key);
   if (isPressed) {
      playSound("click");
   }
   return isPressed;
}

bool handleKeyReleaseWithSound(int key) {
   bool isReleased = IsKeyReleased(key);
   if (isReleased) {
      playSound("click");
   }
   return isReleased;
}
