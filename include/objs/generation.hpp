#ifndef OBJS_GENERATION_HPP
#define OBJS_GENERATION_HPP

#include "objs/map.hpp"
#include "PerlinNoise.hpp"
#include <atomic>
#include <mutex>
#include <string>

struct MapGenerator {
   enum class Biome { plains, forest, mountains, desert_oasis, desert, tundra, jungle };
   enum class BiomeWarmth { cold, warm, hot };

   std::mutex &infoTextMutex;
   std::string &infoText;
   float &progress;

   std::vector<int> rockStartHeights;
   std::string name;
   Map map;

   std::atomic<bool> isCompleted = false;
   bool isFlat = false;

   siv::PerlinNoise biomeTemperatureNoise;
   siv::PerlinNoise biomeMoistureNoise;
   siv::PerlinNoise heightNoise;
   siv::PerlinNoise sandDebriNoise;
   siv::PerlinNoise dirtDebriNoise;
   siv::PerlinNoise oreNoise1;
   siv::PerlinNoise oreNoise2;

   // Constructors

   MapGenerator(const std::string &name, int sizeX, int sizeY, bool isFlat, std::mutex &infoTextMutex, std::string &infoText, float &progress);

   // Generation functions

   void generate();
   void generateTerrain();
   void generateWater();
   void generateDebri();
   void generateTrees();

   // Generation functions for flat worlds

   void generateFlatWorld();

   // Find a perfect spawn location for the player

   Vector2 findPlayerSpawnLocation();

   // Getter functions

   Biome getBiome(int x);
   float normalizedNoise1D(siv::PerlinNoise &noise, int x, float amplitude);
   float normalizedNoise2D(siv::PerlinNoise &noise, int x, int y, float amplitude);

   // Other functions

   void setInfo(const std::string &text, float progress);
};

#endif
