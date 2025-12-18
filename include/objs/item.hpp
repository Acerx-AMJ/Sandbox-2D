#ifndef OBJS_ITEM_HPP
#define OBJS_ITEM_HPP

struct Map;

struct Item {
   enum Type { item, equipment, potion };

   Type type = Type::item;
   unsigned char id = 0;
   bool isFurniture = false;
   bool favorite = false;
   int count = 0;
};

struct SelectedItem {
   Item item;
   Item *address = nullptr;
   bool fullSelect = true;
   bool fromTrash = false;

   void reset();
};

struct DroppedItem {
   Item::Type type = Item::item;
   unsigned char id = 0;
   bool isFurniture = false;
   int count = 0;

   float x = 0, y = 0;
   float lifetime = 0.f;

   void update(Map &map);
   void render(float offsetY);
};

#endif
