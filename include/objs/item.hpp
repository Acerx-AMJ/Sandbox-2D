#pragma once
#include <raylib.h>

enum class ItemType: unsigned char {
   block, item, equipment, potion
};

struct Item {
   ItemType type = ItemType::block;
   unsigned short id = 0;
   unsigned short count = 0;
   bool isFurniture = false;
   bool isWall = false;
   bool favorite = false;
};

struct SelectedItem {
   Item item;
   Item *address = nullptr;
   bool fullSelect = true;
   bool fromTrash = false;

   void reset();
};

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
