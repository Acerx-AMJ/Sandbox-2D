#pragma once
#include <raylib.h>
#include <string>

struct CheckBox {
   void init(Font font, Vector2 origin, const std::string &keybind = "");
   void update();
   void render();

   // Members

   Font font;
   Rectangle rect = {0, 0, 0, 0};
   Vector2 origin = {0.5f, 0.5f};
   std::string keybind;
   bool checked = false;
};
