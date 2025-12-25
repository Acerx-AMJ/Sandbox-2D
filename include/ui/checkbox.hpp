#ifndef UI_CHECKBOX_HPP
#define UI_CHECKBOX_HPP

#include <raylib.h>

struct CheckBox {
   void update();
   void render() const;

   Rectangle rectangle;
   bool checked = false;
};

#endif
