#ifndef UI_CHECKBOX_HPP
#define UI_CHECKBOX_HPP

#include <raylib.h>

struct CheckBox {
   Rectangle rectangle;
   bool checked = false;

   void update();
   void render() const;
};

#endif
