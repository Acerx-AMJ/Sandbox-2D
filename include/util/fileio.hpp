#ifndef UTIL_FILEIO_HPP
#define UTIL_FILEIO_HPP

#include <string>
#include <vector>

struct Map;
struct Player;
struct Inventory;
struct DroppedItem;

// File functions

std::string getRandomLineFromFile(const std::string &path);
std::vector<std::string> getAllLinesFromFile(const std::string &path);
void saveLinesToFile(const std::string &path, const std::vector<std::string> &lines);

void saveWorldData(const std::string &name, float playerX, float playerY, float zoom, const Map &map, const Inventory *inventory, const std::vector<DroppedItem> *droppedItems);
void loadWorldData(const std::string &name, Player &player, float &zoom, Map &map, Inventory &inventory, std::vector<DroppedItem> &droppedItems);

#endif
