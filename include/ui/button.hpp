#pragma once
#include <raylib.h>
#include <string>

constexpr inline Vector2 buttonSize = {0.194f, 0.065f};
constexpr inline Vector2 rawButtonPadding = {0.019f, 0.019f};
constexpr inline Vector2 buttonPadding = {buttonSize.x + rawButtonPadding.x, buttonSize.y + rawButtonPadding.y};

struct Button {
   void init(Font font, Texture texture, Vector2 origin, const std::string &text, const std::string &keybind = "");
   void update(float dt, float offsetY = 0.0f);
   void render(float offsetY = 0.0f);

   // Members

   Font font;
   Texture2D texture {0};
   Rectangle rect = {0, 0, 0, 0};
   Vector2 origin = {0.5f, 0.5f};
   std::string text, keybind;

   bool hovering = false;
   bool down = false;
   bool clicked = false;
   bool favorite = false;
   bool disabled = false;

   float scale = 1.0f;
};
