#include "mngr/input.hpp"
#include "SRU/audio.hpp"
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

constexpr bool operator&(KeyState a, KeyState b) {
   return (static_cast<char>(a) & static_cast<char>(b)) != 0;
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

constexpr bool operator&(MouseState a, MouseState b) {
   return (static_cast<char>(a) & static_cast<char>(b)) != 0;
}

// Globals

static inline std::array<KeyState, KEY_KB_MENU + 1> keys;
static inline std::array<MouseState, MOUSE_BUTTON_BACK + 1> mouse;
static inline bool mouseOnUI = false;
static inline bool shouldBlock = false;

// Update input

void updateInput() {
   mouseOnUI = false;
   keys.fill(KeyState::none);
   mouse.fill(MouseState::none);
}

void setInputBlocking(bool block) {
   shouldBlock = block;
}

// Get key functions

bool isKeyAState(int key, KeyState state) {
   if (shouldBlock) return false;
   
   bool isState = !(keys[key] & state);
   keys[key] = keys[key] | state;
   return isState;
}

bool isKeyDown(int key) {
   return IsKeyDown(key) && isKeyAState(key, KeyState::down);
}

bool isKeyPressed(int key) {
   return IsKeyPressed(key) && isKeyAState(key, KeyState::pressed);
}

bool isKeyReleased(int key) {
   return IsKeyReleased(key) && isKeyAState(key, KeyState::released);
}

bool isKeyRepeated(int key) {
   return (IsKeyPressed(key) && isKeyAState(key, KeyState::pressed)) || (IsKeyPressedRepeat(key) && isKeyAState(key, KeyState::repeated));
}

bool handleKeyPressWithSound(int key) {
   bool pressed = isKeyPressed(key);
   if (pressed) {
      playSound("click");
   }
   return pressed;
}

// Get mouse functions

void setMouseOnUI(bool onUI) {
   mouseOnUI = onUI;
}

void resetMousePress(int button) {
   mouse[button] = MouseState(char(mouse[button]) & ~char(MouseState::pressed));
}

bool isMouseAState(int button, MouseState state) {
   bool isState = !(mouse[button] & state);
   mouse[button] = mouse[button] | state;
   return isState;
}

bool isMouseDownUI(int button) {
   return mouseOnUI && isMouseDown(button);
}

bool isMousePressedUI(int button) {
   return mouseOnUI && isMousePressed(button);
}

bool isMouseReleasedUI(int button) {
   return mouseOnUI && isMouseReleased(button);
}

bool isMouseDownOutsideUI(int button) {
   return !mouseOnUI && isMouseDown(button);
}

bool isMousePressedOutsideUI(int button) {
   return !mouseOnUI && isMousePressed(button);
}

bool isMouseReleasedOutsideUI(int button) {
   return !mouseOnUI && isMouseReleased(button);
}

bool isMouseDown(int button) {
   return IsMouseButtonDown(button) && isMouseAState(button, MouseState::down);
}

bool isMousePressed(int button) {
   return IsMouseButtonPressed(button) && isMouseAState(button, MouseState::pressed);
}

bool isMouseReleased(int button) {
   return IsMouseButtonReleased(button) && isMouseAState(button, MouseState::released);
}
