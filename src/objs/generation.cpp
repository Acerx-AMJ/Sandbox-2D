#include "objs/generation.hpp"
#include "objs/map.hpp"
#include "util/fileio.hpp"
#include "util/random.hpp"
#include "PerlinNoise.hpp"
#include <unordered_map>

// Constants

constexpr float startY = .5f;
constexpr float seaLevel = .4f;
constexpr int rockOffsetStart = 12;
constexpr int rockOffsetMin = 5;
constexpr int rockOffsetMax = 25;
constexpr int maxWaterLength = 100;

// Private functions

inline float normalizedNoise2D(siv::PerlinNoise &noise, int x, int y, float amplitude) {
   return (noise.octave2D(x * amplitude, y * amplitude, 4) + 1.f) / 2.f;
}

inline float normalizedNoise1D(siv::PerlinNoise &noise, int x, float amplitude) {
   return (noise.octave1D(x * amplitude, 4) + 1.f) / 2.f;
}

// Biome functions

constexpr int biomeCount = 6;
static std::unordered_map<Biome, int> treeRates {
   {Biome::plains, 2}, {Biome::forest, 90}, {Biome::mountains, 1}, {Biome::desert, 50}, {Biome::tundra, 60}, {Biome::jungle, 95}
};

constexpr std::array<std::pair<const char*, const char*>, biomeCount> biomeBlocks {{
   {"grass", "dirt"}, {"grass", "dirt"}, {"stone", "stone"}, {"sand", "sand"}, {"snow", "snow"}, {"jungle_grass", "mud"}
}};

struct BiomeHeightData {
   int hmin[5];
   int hmax[5];
   int rng;
   float highestPoint, lowestPoint;
};

constexpr std::array<BiomeHeightData, biomeCount> biomeHeightData {{
   {{-2, -1, 0, 0, 0},   {0, 0, 0, 1, 2}, 20, .3f,  .45f},
   {{-3, -2, -1, 0, 0},  {0, 0, 1, 2, 3}, 80, .2f,  .5f},
   {{-7, -5, -3, -2, 0}, {0, 2, 3, 5, 7}, 80, .05f, .45f},
   {{-1, -1, 0, 0, 0},   {0, 0, 1, 1, 1}, 15, .3f,  .45f},
   {{-3, -2, -1, 0, 0},  {0, 0, 1, 2, 3}, 80, .2f,  .5f},
   {{-3, -2, -1, 0, 0},  {0, 0, 1, 2, 3}, 80, .2f,  .5f},
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

   float value1 = normalizedNoise1D(mountainDesertNoise , x, 0.004f);
   if (value1 <= .4f) {
      return Biome::desert;
   } else if (value1 >= .75f) {
      return Biome::mountains;
   }

   float value2 = normalizedNoise1D(tundraJungleNoise, x, 0.004f);
   if (value2 <= .3f) {
      return Biome::tundra;
   } else if (value2 >= .75f) {
      return Biome::jungle;
   }
   return Biome::plains;
}

// Generate functions

void generateMap(const std::string &name, int sizeX, int sizeY) {
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

void generateTerrain(Map &map) {
   Biome current = Biome::plains, last = Biome::plains;
   siv::PerlinNoise noise (rand());
   int y = startY * map.sizeY;
   int rockOffset = rockOffsetStart, waterLength = 0;

   for (int x = 0; x < map.sizeX; ++x) {
      float value = normalizedNoise2D(noise, x, y, 0.01f);
      last = current;
      current = getBiome(x);

      // Get different height increase/decrease based on the noise, normal 2D
      // perlin noise is better for top-down generation

      BiomeHeightData data = biomeHeightData[(int)current];
      int height = std::floor(value * 5.f);
      y += random(data.hmin[height], data.hmax[height]);

      // 2 is the middle point in our data
      if ((height == 2 and chance(data.rng)) or height != 2) {
         y += random(-1, 1);
      }

      if (y > map.sizeY * seaLevel) {
         waterLength += 1;
      } else {
         waterLength = 0;
      }

      if (y < map.sizeY * data.highestPoint) {
         y += data.hmax[4];
      }

      if (y > map.sizeY * data.lowestPoint or waterLength >= maxWaterLength) {
         y += data.hmin[random(0, 2)];
      }

      y = std::clamp(y, 0, map.sizeY - 1);
      rockOffset = std::clamp(rockOffset, rockOffsetMin, rockOffsetMax);

      // Generate grass, dirt and stone

      auto [last_top_block, last_bottom_block] = biomeBlocks[(int)last];
      auto [top_block, bottom_block] = biomeBlocks[(int)current];

      map.setBlock(x, y, (last != current and chance(50) ? last_top_block : top_block));
      for (int yy = y + 1; yy < map.sizeY; ++yy) {
         if (yy - y < rockOffset) {
            const auto *block = (last != current and chance(50) ? last_bottom_block : bottom_block);
            map.setBlock(x, yy, block);

            if (std::string(block) != "sand") {
               map.setBlock(x, yy, block, true);
            }
         } else {
            map.setBlock(x, yy, "stone");
            map.setBlock(x, yy, "stone", true);
         }
      }
   }
}

void generateWater(Map &map) {
   int seaY = map.sizeY * seaLevel;
   for (int x = 0; x < map.sizeX; ++x) {
      for (int y = seaY; y < map.sizeY && map.isu(x, y, Block::air); ++y) {
         map.setBlock(x, y, (y == seaY && getBiome(x) == Biome::tundra ? "ice" : "water"));
      }
   }
}

void generateDebri(Map &map) {
   siv::PerlinNoise sandNoise (rand());
   siv::PerlinNoise dirtNoise (rand());

   for (int x = 0; x < map.sizeX; ++x) {
      for (int y = 0; y < map.sizeY; ++y) {
         if (map.isu(x, y, Block::air) || map.isu(x, y, Block::grass) || map.isu(x, y, Block::sand) || map.isu(x, y, Block::snow)) {
            continue;
         }

         float value = normalizedNoise2D(dirtNoise, x, y, 0.04f);
         if (value >= .825f) {
            map.setBlock(x, y, "clay");
         } else if (value <= .2f) {
            map.setBlock(x, y, "dirt");
         } else if (!map.isu(x, y, Block::dirt) && normalizedNoise2D(sandNoise, x, y, 0.04f) <= .15f) {
            map.setBlock(x, y, "sand");
         }
      }
   }
}

void generateTrees(Map &map) {
   float y = startY * map.sizeY + 1;
   int counter = 0, counterThreshold = 0;
   
   for (int x = 0; x < map.sizeX; ++x) {
      while (y < map.sizeY - 1 && map.isu(x, y + 1, Block::air)) { y++; }
      while (y > 0 && !map.isu(x, y, Block::air)) { y--; }

      if (counter >= counterThreshold && chance(treeRates[getBiome(x)])) {
         bool sapling = chance(5);

         if (map.isu(x, y + 1, Block::sand) and chance(60)) {
            Furniture::generate(x, y, map, (sapling ? Furniture::cactus_seed : Furniture::cactus));
         } else {
            Furniture::generate(x, (sapling ? y - 1 : y), map, (sapling ? Furniture::sapling : Furniture::tree));
         }
         counter = 0;
         counterThreshold = random(1, 4);
      } else {
         counter++;
      }
   }
}
