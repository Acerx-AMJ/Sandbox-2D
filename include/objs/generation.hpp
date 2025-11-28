#ifndef OBJS_GENERATION_HPP
#define OBJS_GENERATION_HPP

#include <string>
struct Map;

void generateMap(const std::string& name, int sizeX, int sizeY);
void generateTerrain(Map& map);
void generateWater(Map& map);
void generateDebri(Map& map);
void generateTrees(Map& map);

#endif
