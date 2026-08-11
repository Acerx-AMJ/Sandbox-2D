#pragma once
#include <raylib.h>
#include <string>

struct CheckBox {
   void update();
   void render() const;

   Rectangle rectangle;
   std::string keybind;
   bool checked = false;
};
