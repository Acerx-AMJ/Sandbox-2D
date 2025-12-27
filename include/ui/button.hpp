#ifndef UI_BUTTON_HPP
#define UI_BUTTON_HPP

#include <raylib.h>
#include <string>

// Constants

constexpr inline float buttonWidth    = 210.0f;
constexpr inline float buttonHeight   = 70.0f;
constexpr inline float buttonPaddingX = buttonWidth + 20.0f;
constexpr inline float buttonPaddingY = buttonHeight + 20.0f;

// Button

struct Button {
   void update(float dt, float offsetY = 0.0f);
   void render(float offsetY = 0.0f) const;

   Rectangle normalizeRect() const;

   // Members

   Texture2D *texture = nullptr;
   Rectangle rectangle;
   std::string text;

   bool hovering = false;
   bool down     = false;
   bool clicked  = false;
   bool favorite = false;
   bool disabled = false;

   float scale = 1.0f;
};

#endif
