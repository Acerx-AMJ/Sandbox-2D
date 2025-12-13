#ifndef OBJS_INVENTORY_HPP
#define OBJS_INVENTORY_HPP

#include "objs/item.hpp"
#include "util/config.hpp"

struct Inventory {
   Item items[inventoryHeight][inventoryWidth];
   int selectedX = 0, selectedY = 0;
   bool open = false;

   bool anySelected = false;
   Item *selectedItem = nullptr;

   void update();
   void render();
   void renderItem(Item &item, const Vector2 &position);
};

#endif
