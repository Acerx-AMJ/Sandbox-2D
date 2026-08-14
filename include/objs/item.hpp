#pragma once
#include "config.hpp"
#include <raylib.h>
#include <string>
#include <vector>

// item data

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
   int stackSize = 9999;
};

bool isItemActionValid(const std::string &action);
bool isItemNameValid(const std::string &name);
bool isItemIdValid(itemid_t id);
ItemActionType getItemActionTypeFromString(const std::string &action);
ItemData &getItemData(itemid_t id);
itemid_t getItemIdFromName(const std::string &name);
std::string getItemNameFromId(itemid_t id);
size_t getItemCount();

Texture getItemTexture(itemid_t id);
Vector2 getItemSize(itemid_t id, Vector2 size);

void reserveItemContainers(size_t estimate);
void pushItem(const std::string &name);
void setItem(const std::string &name, ItemData data);

// items

struct Item {
   int count = 0;
   itemid_t id = 0;
   bool favorite = false;
};

struct SelectedItem {
   Item item {};
   int lastSelection = 0;
   bool selected = false;
};

struct DroppedItem {
   int count = 0;
   itemid_t id = 0;
   float lifetime = 0.0f;
   int tileX = 0;
   int tileY = 0;

   bool inBounds = false;
   bool flagForDeletion = false;

   DroppedItem() = default;
   DroppedItem(int count, itemid_t id, float lifetime, int tileX, int tileY);
   DroppedItem(Item &item, int tileX, int tileY);

   void update(const Rectangle &cameraBounds, float dt);
   void render() const;

   Rectangle getBounds() const;
};

// drop data

enum class ToolType: unsigned char {
   pickaxe,
   axe,
   hammer,
};

struct Drop {
   Drop(itemid_t item, float dropChance, int dropMin, int dropMax)
      : item(item), dropChance(dropChance), dropMin(dropMin), dropMax(dropMax) {}

   itemid_t item = 0;
   float dropChance = 1.0f;
   int dropMin = 1;
   int dropMax = 1;
};

struct DropTable {
   std::vector<Drop> drops;
};

bool isToolTypeValid(const std::string &type);
bool isDropTableNameValid(const std::string &name);
bool isDropTableIdValid(droptableid_t id);
ToolType getToolTypeFromString(const std::string &type);
DropTable &getDropTable(droptableid_t id);
droptableid_t getDropTableIdFromName(const std::string &name);
std::string getDropTableNameFromId(droptableid_t id);
size_t getDropTableCount();

void reserveDropTableContainers(size_t estimate);
void pushDropTable(const std::string &name);
void setDropTable(const std::string &name, DropTable &table);
