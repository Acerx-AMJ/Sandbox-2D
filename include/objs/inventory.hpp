#ifndef OBJS_INVENTORY_HPP
#define OBJS_INVENTORY_HPP

#include "objs/item.hpp"
#include "util/config.hpp"

struct Inventory {
   Item items[inventoryHeight][inventoryWidth];
   bool open = false;

   void render();
};

#endif
