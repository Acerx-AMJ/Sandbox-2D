#ifndef OBJS_ITEM_HPP
#define OBJS_ITEM_HPP

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

   int tileX = 0, tileY = 0;
   float lifetime = 0.f;

   DroppedItem(Item &item, int tileX, int tileY);

   void update();
   void render(float offsetY);
};

#endif
