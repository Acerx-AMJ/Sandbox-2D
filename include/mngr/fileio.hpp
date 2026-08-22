#pragma once
#include <string>
#include <vector>

void saveWorldData(const std::string &name, const struct Vector2 &playerSpawnPosition, const struct Vector2 &position, bool creative, int breath, int hearts, int maxHearts, float zoom, const struct Map &map, const struct Console *console, const struct Inventory *inventory, const std::vector<struct DroppedItem> *droppedItems);
void loadWorldData(const std::string &name, struct Player &player, float &zoom, struct Map &map, struct Console &console, struct Inventory &inventory, std::vector<struct DroppedItem> &droppedItems);
bool deleteWorld(const std::string &name);

int getFileVersion(const std::string &name);
int getLatestVersion();
