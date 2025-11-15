#ifndef OBJS_GENERATION_HPP
#define OBJS_GENERATION_HPP

#include <vector>
#include "objs/block.hpp"

using Map = std::vector<std::vector<Block>>;

// Generate functions

void generateMap(Map& map, int sizeX, int sizeY);
void generateTerrain(Map& map);
void generateWater(Map& map);
void generateDebri(Map& map);

#endif
