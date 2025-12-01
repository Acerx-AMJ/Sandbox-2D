#include "objs/generation.hpp"
#include "objs/map.hpp"
#include "util/fileio.hpp"
#include "util/random.hpp"
#include "PerlinNoise.hpp"
#include <unordered_map>

// Constants

constexpr float startY = .5f;
constexpr float seaLevel = .4f;
constexpr float highThreshold = .25f;
constexpr float lowThreshold = .5f;
constexpr float highestPoint = .2f;
constexpr float lowestPoint = .55f;

constexpr int rockOffsetStart = 12;
constexpr int rockOffsetMin = 5;
constexpr int rockOffsetMax = 25;

// Private functions

inline float normalizedNoise2D(siv::PerlinNoise& noise, int x, int y, float amplitude) {
   return (noise.octave2D(x * amplitude, y * amplitude, 4) + 1.f) / 2.f;
}

inline float normalizedNoise1D(siv::PerlinNoise& noise, int x, float amplitude) {
   return (noise.octave1D(x * amplitude, 4) + 1.f) / 2.f;
}

// Biome functions

constexpr int biomeCount = 6;
static std::unordered_map<Biome, int> treeRates {
   {Biome::plains, 2}, {Biome::forest, 90}, {Biome::mountains, 1}, {Biome::desert, 40}, {Biome::tundra, 60}, {Biome::jungle, 95}
};

constexpr std::array<std::pair<const char*, const char*>, biomeCount> biomeBlocks {{
   {"grass", "dirt"}, {"grass", "dirt"}, {"stone", "stone"}, {"sand", "sand"}, {"snow", "snow"}, {"jungle_grass", "mud"}
}};

// A single noise object can do two things, one for the 0.0-0.1 and one for 0.9-1.0
static siv::PerlinNoise forestPlainNoise;
static siv::PerlinNoise mountainDesertNoise;
static siv::PerlinNoise tundraJungleNoise;

void initBiomeNoise() {
   forestPlainNoise.reseed(rand());
   mountainDesertNoise.reseed(rand());
   tundraJungleNoise.reseed(rand());
}

Biome getBiome(int x) {
   if (normalizedNoise1D(forestPlainNoise, x, 0.004f) <= .2f) {
      return Biome::forest;
   }

   auto value1 = normalizedNoise1D(mountainDesertNoise , x, 0.004f);
   if (value1 <= .4f) {
      return Biome::desert;
   } else if (value1 >= .75f) {
      return Biome::mountains;
   }

   auto value2 = normalizedNoise1D(tundraJungleNoise, x, 0.004f);
   if (value2 <= .3f) {
      return Biome::tundra;
   } else if (value2 >= .75f) {
      return Biome::jungle;
   }
   return Biome::plains;
}

// Generate functions

void generateMap(const std::string& name, int sizeX, int sizeY) {
   Map map;
   map.sizeX = sizeX;
   map.sizeY = sizeY;
   map.init();
   initBiomeNoise();

   generateTerrain(map);
   generateDebri(map);
   generateWater(map);
   generateTrees(map);
   saveWorldData(name, sizeX / 2.f, 0.f, 50.f, map);
}

void generateTerrain(Map& map) {
   siv::PerlinNoise noise (rand());
   int y = startY * map.sizeY;
   int rockOffset = rockOffsetStart;

   for (int x = 0; x < map.sizeX; ++x) {
      float value = normalizedNoise2D(noise, x, y, 0.01f);

      // Get different height increase/decrease based on the noise, normal 2D
      // perlin noise is better for top-down generation

      if (value < .2f) {
         y += random(-3, 0);
         rockOffset += random(-2, 0);
      } else if (value < .4f) {
         y += random(-2, 0);
         rockOffset += random(-1, 0);
      } else if (value < .65f) {
         y += (chance(50) ? random(-1, 1) : 0);
         rockOffset += (chance(20) ? random(-1, 1) : 0);
      } else if (value < .85f) {
         y += random(0, 2);
         rockOffset += random(0, 1);
      } else {
         y += random(0, 3);
         rockOffset += random(0, 2);
      }

      // Don't let the height get too low or too high

      if (y < map.sizeY * highThreshold) {
         ++y;
         ++rockOffset;
      }

      if (y > map.sizeY * lowThreshold) {
         --y;
         --rockOffset;
      }
      y = std::clamp<int>(y, map.sizeY * highestPoint, map.sizeY * lowestPoint);
      rockOffset = std::clamp(rockOffset, rockOffsetMin, rockOffsetMax);

      // Generate grass, dirt and stone

      auto [top_block, bottom_block] = biomeBlocks[(int)getBiome(x)];
      map.setBlock(x, y, top_block);

      for (int yy = y + 1; yy < map.sizeY; ++yy) {
         if (yy - y < rockOffset) {
            map.setBlock(x, yy, bottom_block);
            map.setBlock(x, yy, bottom_block, true);
         } else {
            map.setBlock(x, yy, "stone");
            map.setBlock(x, yy, "stone", true);
         }
      }
   }
}

void generateWater(Map& map) {
   int seaY = map.sizeY * seaLevel;
   for (int x = 0; x < map.sizeX; ++x) {
      for (int y = seaY; y < map.sizeY and map.isu(x, y, Block::air); ++y) {
         map.setBlock(x, y, (getBiome(x) == Biome::tundra ? "ice" : "water"));
      }
   }
}

void generateDebri(Map& map) {
   siv::PerlinNoise sandNoise (rand());
   siv::PerlinNoise dirtNoise (rand());

   for (int x = 0; x < map.sizeX; ++x) {
      for (int y = 0; y < map.sizeY; ++y) {
         if (map.isu(x, y, Block::air) or map.isu(x, y, Block::grass)) {
            continue;
         }

         float value = normalizedNoise2D(dirtNoise, x, y, 0.04f);
         if (value >= .825f) {
            map.setBlock(x, y, "clay");
         } else if (value <= .2f) {
            map.setBlock(x, y, "dirt");
         } else if (not map.isu(x, y, Block::dirt) and normalizedNoise2D(sandNoise, x, y, 0.04f) <= .15f) {
            map.setBlock(x, y, "sand");
         }
      }
   }
}

void generateTrees(Map& map) {
   float y = startY * map.sizeY + 1;
   int counter = 0, counterThreshold = 0;
   
   for (int x = 0; x < map.sizeX; ++x) {
      while (y < map.sizeY - 1 and map.isu(x, y + 1, Block::air)) { y++; }
      while (y > 0 and not map.isu(x, y, Block::air)) { y--; }

      if (counter >= counterThreshold and chance(treeRates[getBiome(x)])) {
         bool sapling = chance(5);
         Furniture::generate(x, (sapling ? y - 1 : y), map, (sapling ? Furniture::sapling : Furniture::tree));
         counter = 0;
         counterThreshold = random(1, 4);
      } else {
         counter++;
      }
   }
}
