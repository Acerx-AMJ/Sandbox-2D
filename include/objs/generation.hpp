#ifndef OBJS_GENERATION_HPP
#define OBJS_GENERATION_HPP

#include "objs/map.hpp"
#include "PerlinNoise.hpp"
#include <string>

struct MapGenerator {
   enum class Biome { plains, forest, mountains, desert_oasis, desert, tundra, jungle };
   enum class BiomeWarmth { cold, warm, hot };

   std::string name;
   Map map;

   siv::PerlinNoise biomeTemperatureNoise;
   siv::PerlinNoise biomeMoistureNoise;
   siv::PerlinNoise heightNoise;
   siv::PerlinNoise sandDebriNoise;
   siv::PerlinNoise dirtDebriNoise;

   // Constructors

   MapGenerator(const std::string &name, int sizeX, int sizeY);

   // Generation functions

   void generate();
   void generateTerrain();
   void generateWater();
   void generateDebri();
   void generateTrees();

   // Getter functions

   Biome getBiome(int x);
   float normalizedNoise1D(siv::PerlinNoise &noise, int x, float amplitude);
   float normalizedNoise2D(siv::PerlinNoise &noise, int x, int y, float amplitude);
};

#endif
