#include "mngr/input.hpp"
#include "mngr/sound.hpp"
#include <raylib.h>
#include <array>

// Key state enum

enum class KeyState: char {
   none,
   down = 1,
   pressed = 2,
   released = 4,
   repeated = 8
};

constexpr KeyState operator|(KeyState a, KeyState b) {
   return static_cast<KeyState>(static_cast<char>(a) | static_cast<char>(b));
}

constexpr KeyState operator-(KeyState a, KeyState b) {
   return static_cast<KeyState>(static_cast<char>(a) - static_cast<char>(b));
}

constexpr char operator&(KeyState a, KeyState b) {
   return static_cast<char>(a) & static_cast<char>(b);
}

// Mouse state enum

enum class MouseState: char {
   none,
   down = 1,
   pressed = 2,
   released = 4
};

constexpr MouseState operator|(MouseState a, MouseState b) {
   return static_cast<MouseState>(static_cast<char>(a) | static_cast<char>(b));
}

constexpr MouseState operator-(MouseState a, MouseState b) {
   return static_cast<MouseState>(static_cast<char>(a) - static_cast<char>(b));
}

constexpr char operator&(MouseState a, MouseState b) {
   return static_cast<char>(a) & static_cast<char>(b);
}

// Globals

static inline std::array<KeyState, KEY_KB_MENU + 1> keys;
static inline std::array<MouseState, MOUSE_BUTTON_BACK + 1> mouse;
static inline bool mouseOnUI = false;

// Update input

void updateInput() {
   mouseOnUI = false;
   for (int key = 0; key <= KEY_KB_MENU; ++key) {
      KeyState &state = keys[key];
      state = KeyState::none;

      if (IsKeyDown(key))          state = state | KeyState::down;
      if (IsKeyPressed(key))       state = state | KeyState::pressed;
      if (IsKeyReleased(key))      state = state | KeyState::released;
      if (IsKeyPressedRepeat(key)) state = state | KeyState::repeated;
   }

   for (int button = 0; button <= MOUSE_BUTTON_BACK; ++button) {
      MouseState &state = mouse[button];
      state = MouseState::none;

      if (IsMouseButtonDown(button))     state = state | MouseState::down;
      if (IsMouseButtonPressed(button))  state = state | MouseState::pressed;
      if (IsMouseButtonReleased(button)) state = state | MouseState::released;
   }
}

// Get key functions

bool isKeyAState(int key, KeyState state) {
   bool isState = keys[key] & state;
   if (isState) {
      keys[key] = keys[key] - state;
   }
   return isState;
}

bool isKeyDown(int key) {
   return isKeyAState(key, KeyState::down);
}

bool isKeyPressed(int key) {
   return isKeyAState(key, KeyState::pressed);
}

bool isKeyReleased(int key) {
   return isKeyAState(key, KeyState::released);
}

bool isKeyRepeated(int key) {
   return isKeyAState(key, KeyState::repeated) || isKeyAState(key, KeyState::pressed);
}

bool handleKeyPressWithSound(int key) {
   bool pressed = isKeyPressed(key);
   if (pressed) {
      playSound("click");
   }
   return pressed;
}

bool handleKeyReleaseWithSound(int key) {
   bool released = IsKeyReleased(key);
   if (released) {
      playSound("click");
   }
   return released;
}

// Get mouse functions

void setMouseOnUI(bool onUI) {
   mouseOnUI = onUI;
}

void resetMousePress(int button) {
   mouse[button] = mouse[button] | MouseState::pressed;
}

bool isMouseAState(int button, MouseState state, bool onUI) {
   if (onUI != mouseOnUI) {
      return false;
   }

   bool isState = mouse[button] & state;
   if (isState) {
      mouse[button] = mouse[button] - state;
   }
   return isState;
}

bool isMouseDownUI(int button) {
   return isMouseAState(button, MouseState::down, true);
}

bool isMousePressedUI(int button) {
   return isMouseAState(button, MouseState::pressed, true);
}

bool isMouseReleasedUI(int button) {
   return isMouseAState(button, MouseState::released, true);
}

bool isMouseDownOutsideUI(int button) {
   return isMouseAState(button, MouseState::down, false);
}

bool isMousePressedOutsideUI(int button) {
   return isMouseAState(button, MouseState::pressed, false);
}

bool isMouseReleasedOutsideUI(int button) {
   return isMouseAState(button, MouseState::released, false);
}

bool isMouseDown(int button) {
   return isMouseAState(button, MouseState::down, mouseOnUI);
}

bool isMousePressed(int button) {
   return isMouseAState(button, MouseState::pressed, mouseOnUI);
}

bool isMouseReleased(int button) {
   return isMouseAState(button, MouseState::released, mouseOnUI);
}
