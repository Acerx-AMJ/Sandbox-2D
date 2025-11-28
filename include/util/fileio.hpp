#ifndef UTIL_FILEIO_HPP
#define UTIL_FILEIO_HPP

#include <filesystem>
#include <string>

struct Map;
struct Player;

// File functions

std::string getRandomLineFromFile(const std::filesystem::path& file);
void saveWorldData(const std::string& name, float playerX, float playerY, float zoom, const Map& map);
void loadWorldData(const std::string& name, Player& player, float& zoom, Map& map);

#endif
