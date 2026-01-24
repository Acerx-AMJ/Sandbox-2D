#ifndef OBJS_ITEM_HPP
#define OBJS_ITEM_HPP

#include <raylib.h>

// Item type

enum class ItemType: unsigned char {
   block, item, equipment, potion
};

// Item

struct Item {
   ItemType type = ItemType::block;
   unsigned short id = 0;
   unsigned short count = 0;
   bool isFurniture = false;
   bool isWall = false;
   bool favorite = false;
};

// Selected item

struct SelectedItem {
   Item item;
   Item *address = nullptr;
   bool fullSelect = true;
   bool fromTrash = false;

   void reset();
};

// Dropped item

struct DroppedItem {
   ItemType type = ItemType::block;
   unsigned short id = 0;
   unsigned short count = 0;
   float lifetime = 0.f;
   int tileX = 0;
   int tileY = 0;

   bool isFurniture = false;
   bool isWall = false;
   bool inBounds = false;
   bool flagForDeletion = false;

   DroppedItem() = default;
   DroppedItem(ItemType type, unsigned short id, unsigned short count, bool isFurniture, bool isWall, int tileX, int tileY, float lifetime);
   DroppedItem(Item &item, int tileX, int tileY);

   void update(const Rectangle &cameraBounds, float dt);
   void render() const;

   Rectangle getBounds() const;
};

#endif
