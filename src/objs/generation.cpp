#include "objs/generation.hpp"
#include "util/fileio.hpp"
#include "util/random.hpp"

// Biome constants

struct BiomeData {
   int hmin[5];
   int hmax[5];
   int rng;
   float highestPoint, lowestPoint;
   int treeRate;
   std::string top;
   std::string bottom;
   MapGenerator::BiomeWarmth wamth;
};

constexpr int biomeCount = 7;
static inline std::array<BiomeData, biomeCount> biomeData {{
   {{-2, -1, 0, 0, 0},   {0, 0, 0, 1, 2}, 20, .3f,  .45f, 2,  "grass",        "dirt",  MapGenerator::BiomeWarmth::warm},
   {{-3, -2, -1, 0, 0},  {0, 0, 1, 2, 3}, 80, .2f,  .5f,  90, "grass",        "dirt",  MapGenerator::BiomeWarmth::warm},
   {{-7, -5, -3, -2, 0}, {0, 2, 3, 5, 7}, 80, .05f, .45f, 1,  "stone",        "stone", MapGenerator::BiomeWarmth::cold},
   {{-1, -1, 0, 0, 0},   {0, 0, 1, 1, 1}, 15, .3f,  .45f, 50, "sand",         "sand",  MapGenerator::BiomeWarmth::hot},
   {{-1, -1, 0, 0, 0},    {0, 0, 1, 1, 1}, 5, .3f,  .45f, 2,  "sand",         "sand",  MapGenerator::BiomeWarmth::hot},
   {{-3, -2, -1, 0, 0},  {0, 0, 1, 2, 3}, 80, .2f,  .5f,  60, "snow",         "snow",  MapGenerator::BiomeWarmth::cold},
   {{-3, -2, -1, 0, 0},  {0, 0, 1, 2, 3}, 80, .2f,  .5f,  95, "jungle_grass", "mud",   MapGenerator::BiomeWarmth::hot},
}};

// Constructors

MapGenerator::MapGenerator(const std::string &name, int sizeX, int sizeY, bool isFlat)
   : name(name), isFlat(isFlat) {
   map.sizeX = sizeX;
   map.sizeY = sizeY;
   map.init();

   if (!isFlat) {
      biomeTemperatureNoise.reseed(rand());
      biomeMoistureNoise.reseed(rand());
      heightNoise.reseed(rand());
      sandDebriNoise.reseed(rand());
      dirtDebriNoise.reseed(rand());
   }
}

// Generation functions

void MapGenerator::generate() {
   if (isFlat) {
      generateFlatWorld();
   } else {
      generateTerrain();
      generateDebri();
      generateWater();
      generateTrees();
   }
   saveWorldData(name, map.sizeX / 2.f, 0.f, 50.f, map);
}

void MapGenerator::generateTerrain() {
   Biome current = Biome::plains, last = Biome::plains;
   int y = startY * map.sizeY;
   int rockOffset = rockOffsetStart, waterLength = 0;

   for (int x = 0; x < map.sizeX; ++x) {
      float value = normalizedNoise2D(heightNoise, x, y, 0.01f);
      last = current;
      current = getBiome(x);

      // Get different height increase/decrease based on the noise and the biome,
      // normal 2D perlin noise is better for top-down generation.

      BiomeData &data = biomeData[(int)current];
      BiomeData &lastData = biomeData[(int)last];

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

      map.setBlock(x, y, (last != current and chance(50) ? lastData.top : data.top));
      for (int yy = y + 1; yy < map.sizeY; ++yy) {
         if (yy - y < rockOffset) {
            std::string &block = (last != current and chance(50) ? lastData.bottom : data.bottom);
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

void MapGenerator::generateWater() {
   int seaY = map.sizeY * seaLevel;
   for (int x = 0; x < map.sizeX; ++x) {
      for (int y = seaY; y < map.sizeY && map.isu(x, y, Block::air); ++y) {
         map.setBlock(x, y, (y == seaY && biomeData[(int)getBiome(x)].wamth == BiomeWarmth::cold ? "ice" : "water"));
      }
   }
}

void MapGenerator::generateDebri() {
   for (int x = 0; x < map.sizeX; ++x) {
      for (int y = 0; y < map.sizeY; ++y) {
         if (map.isu(x, y, Block::air) || map.isu(x, y, Block::grass) || map.isu(x, y, Block::sand) || map.isu(x, y, Block::snow)) {
            continue;
         }

         float value = normalizedNoise2D(dirtDebriNoise, x, y, 0.04f);
         if (value >= .825f) {
            map.setBlock(x, y, "clay");
         } else if (value <= .2f) {
            map.setBlock(x, y, "dirt");
         } else if (!map.isu(x, y, Block::dirt) && normalizedNoise2D(sandDebriNoise, x, y, 0.04f) <= .15f) {
            map.setBlock(x, y, "sand");
         }
      }
   }
}

void MapGenerator::generateTrees() {
   float y = startY * map.sizeY + 1;
   int counter = 0, counterThreshold = 0;
   
   for (int x = 0; x < map.sizeX; ++x) {
      while (y < map.sizeY - 1 && map.isu(x, y + 1, Block::air)) { y++; }
      while (y > 0 && !map.isu(x, y, Block::air)) { y--; }

      if (counter >= counterThreshold && chance(biomeData[(int)getBiome(x)].treeRate)) {
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

// Generation functions for flat worlds

void MapGenerator::generateFlatWorld() {
   int y = startY * map.sizeY;
   
   for (int x = 0; x < map.sizeX; ++x) {
      map.setBlock(x, y, "grass");

      for (int yy = y + 1; yy < map.sizeY; ++yy) {
         // Set both the wall and the block
         if (yy - y < rockOffsetStart) {
            map.setBlock(x, yy, "dirt");
            map.setBlock(x, yy, "dirt", true);
         } else {
            map.setBlock(x, yy, "stone");
            map.setBlock(x, yy, "stone", true);
         }
      }
   }
}

// Getter functions

MapGenerator::Biome MapGenerator::getBiome(int x) {
   float temperature = normalizedNoise1D(biomeTemperatureNoise, x, 0.004f);
   float moisture = normalizedNoise1D(biomeMoistureNoise, x, 0.004f);

   if (temperature < .35f) {
      // Cold biomes
      if (moisture < .5f) {
         return Biome::mountains;
      } else {
         return Biome::tundra;
      }
   } else if (temperature < .55f) {
      // Warm biomes
      if (moisture < .5f) {
         return Biome::plains;
      } else {
         return Biome::forest;
      }
   } else {
      // Hot biomes
      if (moisture < .35f) {
         return Biome::desert;
      } else if (moisture < .55f) {
         return Biome::desert_oasis;
      } else {
         return Biome::jungle;
      }
   }
}

float MapGenerator::normalizedNoise1D(siv::PerlinNoise &noise, int x, float amplitude) {
   return (noise.octave1D(x * amplitude, 4) + 1.f) / 2.f;
}

float MapGenerator::normalizedNoise2D(siv::PerlinNoise &noise, int x, int y, float amplitude) {
   return (noise.octave2D(x * amplitude, y * amplitude, 4) + 1.f) / 2.f;
}
