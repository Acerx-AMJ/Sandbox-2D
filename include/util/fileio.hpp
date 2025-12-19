#ifndef UTIL_FILEIO_HPP
#define UTIL_FILEIO_HPP

#include <string>
struct Map;
struct Player;
struct Inventory;

// File functions

std::string getRandomLineFromFile(const std::string &file);
void saveWorldData(const std::string &name, float playerX, float playerY, float zoom, const Map &map, const Inventory *inventory);
void loadWorldData(const std::string &name, Player &player, float &zoom, Map &map, Inventory &inventory);

#endif
