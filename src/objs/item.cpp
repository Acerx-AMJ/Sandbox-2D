#include "objs/item.hpp"
#include "objs/inventory.hpp"
#include <raylib.h>
#include <raymath.h>
#include <cmath>

// constants

static const std::unordered_map<std::string, ItemActionType> itemActionTypeStrings {{
   {"none", ItemActionType::none}, {"place_block", ItemActionType::placeBlock}, {"place_wall", ItemActionType::placeWall},
   {"place_furniture", ItemActionType::placeFurniture}, {"place_liquid", ItemActionType::placeLiquid}
}};

static size_t itemCount = 1; // 0 - nil
static std::vector<std::string> itemNames {""};
static std::vector<ItemData> itemData {{}};
static std::unordered_map<std::string, itemid_t> itemIds;

// item getter functions

bool isItemActionValid(const std::string &action) {
   return itemActionTypeStrings.find(action) != itemActionTypeStrings.end();
}

bool isItemNameValid(const std::string &name) {
   return itemIds.find(name) != itemIds.end();
}

bool isItemIdValid(itemid_t id) {
   return id >= 0 && id < itemCount;
}

ItemActionType getItemActionTypeFromString(const std::string &action) {
   if (auto it = itemActionTypeStrings.find(action); it != itemActionTypeStrings.end()) {
      return it->second;
   }
   return ItemActionType::none;
}

itemid_t getItemIdFromName(const std::string &name) {
   return itemIds.at(name);
}

std::string getItemNameFromId(itemid_t id) {
   return itemNames[id];
}

size_t getItemCount() {
   return itemCount;
}

void reserveItemContainers(size_t estimate) {
   itemNames.reserve(estimate + 1);
   itemData.reserve(estimate + 1);
   itemIds.reserve(estimate + 1);
}

void pushItem(const std::string &name) {
   itemData.push_back({});
   itemNames.push_back(name);
   itemIds[name] = itemCount;
   itemCount += 1;
}

void setItem(const std::string &name, ItemData data) {
   itemid_t id = itemIds[name];
   itemData[id] = data;
}

// Constants

constexpr int droppedItemLifetime      = 60.0f * 15.0f;
constexpr float droppedItemFloatSpeed  = 1.5f;
constexpr float droppedItemFloatHeight = 0.25f;
constexpr Vector2 droppedItemSize      = {0.8f, 0.8f};

// Selected item functions

void SelectedItem::reset()  {
   item = Item{};
   address = nullptr;
   fullSelect = true;
   fromTrash = false;
}

// Dropped item functions

DroppedItem::DroppedItem(ItemType type, unsigned short id, unsigned short count, bool isFurniture, bool isWall, int tileX, int tileY, float lifetime)
   : type(type), id(id), count(count), lifetime(lifetime), tileX(tileX), tileY(tileY), isFurniture(isFurniture), isWall(isWall) {}

DroppedItem::DroppedItem(Item &item, int tileX, int tileY)
   : type(item.type), id(item.id), count(item.count), lifetime(0.0f), tileX(tileX), tileY(tileY), isFurniture(item.isFurniture), isWall(item.isWall) {}

void DroppedItem::update(const Rectangle &cameraBounds, float dt) {
   lifetime += dt;
   inBounds = (tileX >= cameraBounds.x && tileX <= cameraBounds.width && tileY >= cameraBounds.y && tileY <= cameraBounds.height);

   if (lifetime >= droppedItemLifetime) {
      flagForDeletion = true;
   }
}

void DroppedItem::render() const {
   if (!inBounds) {
      return;
   }

   float offsetY = std::sin(lifetime * droppedItemFloatSpeed) * droppedItemFloatHeight;
   Vector2 position = {tileX + 0.5f, (tileY + 0.5f) - offsetY};
   Vector2 size = droppedItemSize;
   drawItem(type, id, count, isFurniture, isWall, position, size, false, true);
}

Rectangle DroppedItem::getBounds() const {
   return {(float)tileX, (float)tileY, 1.0f, 1.0f};
}
