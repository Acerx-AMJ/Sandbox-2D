#ifndef OBJS_GENERATION_HPP
#define OBJS_GENERATION_HPP

#include "objs/block.hpp"

// Generate functions

void generateMap(Map& map, int sizeX, int sizeY);
void generateTerrain(Map& map);
void generateWater(Map& map);
void generateDebri(Map& map);

#endif
