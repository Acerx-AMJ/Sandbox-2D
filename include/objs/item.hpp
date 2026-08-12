#pragma once
#include "types.hpp"
#include <raylib.h>
#include <string>

enum class ItemActionType: unsigned char {
   none,
   placeBlock,
   placeWall,
   placeFurniture,
   placeLiquid,
};

struct ItemData {
   Texture texture {0};
   ItemActionType action = ItemActionType::none;
   blockid_t block = 0;
   blockid_t wall = 0;
   furnitureid_t furniture = 0;
   liquidid_t liquid = 0;
   unsigned stackSize = 9999;
};

bool isItemActionValid(const std::string &action);
bool isItemNameValid(const std::string &name);
bool isItemIdValid(itemid_t id);
ItemActionType getItemActionTypeFromString(const std::string &action);
itemid_t getItemIdFromName(const std::string &name);
std::string getItemNameFromId(itemid_t id);
size_t getItemCount();

void reserveItemContainers(size_t estimate);
void pushItem(const std::string &name);
void setItem(const std::string &name, ItemData data);

struct ItemNew {
   unsigned count = 0;
   itemid_t id = 0;
   bool favorite = false;
};

// old shit
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
