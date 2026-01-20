#ifndef UTIL_FILEIO_HPP
#define UTIL_FILEIO_HPP

#include <string>
#include <vector>

// File functions

std::string getRandomLineFromFile(const std::string &path);
std::vector<std::string> getAllLinesFromFile(const std::string &path);
void saveLinesToFile(const std::string &path, const std::vector<std::string> &lines);

void saveWorldData(const std::string &name, float playerX, float playerY, int breath, int hearts, int maxHearts, float zoom, const struct Map &map, const struct Inventory *inventory, const std::vector<struct DroppedItem> *droppedItems);
void loadWorldData(const std::string &name, struct Player &player, float &zoom, struct Map &map, struct Inventory &inventory, std::vector<struct DroppedItem> &droppedItems);

int getFileVersion(const std::string &name);
int getLatestVersion();

#endif
