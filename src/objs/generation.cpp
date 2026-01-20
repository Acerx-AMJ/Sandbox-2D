#include "objs/generation.hpp"
#include "mngr/sound.hpp"
#include "util/fileio.hpp"
#include "util/random.hpp"
#include <thread>

// Constants

constexpr float startY        = 0.5f;
constexpr float seaLevel      = 0.4f;
constexpr int rockOffsetStart = 12;
constexpr int rockOffsetMin   = 5;
constexpr int rockOffsetMax   = 25;
constexpr int maxWaterLength  = 100;

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

static inline const std::array<BiomeData, biomeCount> biomeData {{
   {{-2, -1, 0, 0, 0},   {0, 0, 0, 1, 2}, 20, .3f,  .45f, 2,  "grass",        "dirt",  MapGenerator::BiomeWarmth::warm}, // Plains
   {{-3, -2, -1, 0, 0},  {0, 0, 1, 2, 3}, 80, .2f,  .5f,  90, "grass",        "dirt",  MapGenerator::BiomeWarmth::warm}, // Forest
   {{-7, -5, -3, -2, 0}, {0, 2, 3, 5, 7}, 80, .05f, .45f, 1,  "stone",        "stone", MapGenerator::BiomeWarmth::cold}, // Mountains
   {{-1, -1, 0, 0, 0},   {0, 0, 1, 1, 1}, 15, .3f,  .45f, 50, "sand",         "sand",  MapGenerator::BiomeWarmth::hot},  // Desert oasis
   {{-1, -1, 0, 0, 0},   {0, 0, 1, 1, 1},  5, .3f,  .45f, 2,  "sand",         "sand",  MapGenerator::BiomeWarmth::hot},  // Desert
   {{-3, -2, -1, 0, 0},  {0, 0, 1, 2, 3}, 80, .2f,  .5f,  60, "snow",         "snow",  MapGenerator::BiomeWarmth::cold}, // Tundra
   {{-3, -2, -1, 0, 0},  {0, 0, 1, 2, 3}, 80, .2f,  .5f,  95, "jungle_grass", "mud",   MapGenerator::BiomeWarmth::hot},  // Jungle
}};

// Constructors

MapGenerator::MapGenerator(const std::string &name, int sizeX, int sizeY, bool isFlat, std::mutex &infoTextMutex, std::string &infoText, float &progress)
   : infoTextMutex(infoTextMutex), infoText(infoText), progress(progress), name(name), isFlat(isFlat) {
   map.sizeX = sizeX;
   map.sizeY = sizeY;
   setInfo("Initializing...", 0.0f);
}

// Generation functions

void MapGenerator::generate() {
   // Takes too long, better to put it in a thread!
   map.init();

   setInfo("Seeding Noise...", 0.0f);
   if (!isFlat) {
      biomeTemperatureNoise.reseed(rand());
      biomeMoistureNoise.reseed(rand());
      heightNoise.reseed(rand());
      sandDebriNoise.reseed(rand());
      dirtDebriNoise.reseed(rand());
   }

   if (isFlat) {
      generateFlatWorld();
   } else {
      generateTerrain();
      generateDebri();
      generateWater();
      generateTrees();
   }

   const Vector2 spawnLocation = findPlayerSpawnLocation();

   setInfo("Saving to File...", 0.95f);
   saveWorldData(name, spawnLocation.x, spawnLocation.y, 100, 100, 100, 50.f, map, nullptr, nullptr);

   setInfo("Generating Completed!", 1.0f);
   playSound("load");
   std::this_thread::sleep_for(std::chrono::milliseconds(500));
   isCompleted = true;
}

void MapGenerator::generateTerrain() {
   setInfo("Generating Terrain...", 0.1f);

   Biome current = Biome::plains, last = Biome::plains;
   int y = startY * map.sizeY;
   int rockOffset = rockOffsetStart, waterLength = 0;

   for (int x = 0; x < map.sizeX; ++x) {
      float value = normalizedNoise2D(heightNoise, x, y, 0.01f);
      last = current;
      current = getBiome(x);

      // Get different height increase/decrease based on the noise and the biome,
      // normal 2D perlin noise is better for top-down generation.

      const BiomeData &data = biomeData.at((int)current);
      const BiomeData &lastData = biomeData.at((int)last);

      int height = std::floor(value * 5.f);
      y += random(data.hmin[height], data.hmax[height]);

      // 2 is the middle point in our data
      if ((height == 2 && chance(data.rng)) || height != 2) {
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

      if (y > map.sizeY * data.lowestPoint || waterLength >= maxWaterLength) {
         y += data.hmin[random(0, 2)];
      }

      y = std::clamp(y, 0, map.sizeY - 1);
      rockOffset = std::clamp(rockOffset, rockOffsetMin, rockOffsetMax);

      // Generate grass, dirt and stone

      map.setBlock(x, y, (last != current && chance(50) ? lastData.top : data.top));
      for (int yy = y + 1; yy < map.sizeY; ++yy) {
         if (yy - y < rockOffset) {
            const std::string &block = (last != current && chance(50) ? lastData.bottom : data.bottom);
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
   setInfo("Filling Water...", 0.75f);
   int seaY = map.sizeY * seaLevel;

   for (int x = 0; x < map.sizeX; ++x) {
      for (int y = seaY; y < map.sizeY && map.isu(x, y, BlockType::empty); ++y) {
         if (y == seaY && biomeData[(int)getBiome(x)].wamth == BiomeWarmth::cold) {
            map.setBlock(x, y, "ice");
         } else {
            map.liquidsHeights[y][x] = maxLiquidLayers;
            map.liquidTypes[y][x] = LiquidType::water;
         }
      }
   }
}

void MapGenerator::generateDebri() {
   setInfo("Generating Debris...", 0.5f);

   for (int x = 0; x < map.sizeX; ++x) {
      for (int y = 0; y < map.sizeY; ++y) {
         if (map.isu(x, y, BlockType::empty) || map.isu(x, y, BlockType::grass) || map.isu(x, y, BlockType::sand)) {
            continue;
         }

         float value = normalizedNoise2D(dirtDebriNoise, x, y, 0.04f);
         if (value >= .825f) {
            map.setBlock(x, y, "clay");
         } else if (value <= .2f) {
            map.setBlock(x, y, "dirt");
         } else if (!map.isu(x, y, BlockType::dirt) && normalizedNoise2D(sandDebriNoise, x, y, 0.04f) <= .15f) {
            map.setBlock(x, y, "sand");
         }
      }
   }
}

void MapGenerator::generateTrees() {
   setInfo("Growing Trees...", 0.85f);

   float y = startY * map.sizeY + 1;
   int counter = 0, counterThreshold = 0;
   
   for (int x = 0; x < map.sizeX; ++x) {
      while (y < map.sizeY - 1 && map.isu(x, y + 1, BlockType::empty)) { y++; }
      while (y > 0 && !map.isu(x, y, BlockType::empty)) { y--; }

      if (counter >= counterThreshold && chance(biomeData[(int)getBiome(x)].treeRate)) {
         bool sapling = chance(5);

         if (map.blocks[y + 1][x].id != getBlockIdFromName("sand") && chance(60)) {
            generateFurniture(x, y, map, (sapling ? FurnitureType::cactusSeed : FurnitureType::cactus), false);
         } else {
            generateFurniture(x, (sapling ? y - 1 : y), map, (sapling ? FurnitureType::sapling : FurnitureType::tree), false);
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
   setInfo("Generating Flat Terrain...", 0.1f);
   int startingPointY = startY * map.sizeY;
   int rockStart = rockOffsetStart + startingPointY;

   map.setRow(startingPointY, "grass");
   for (int y = startingPointY + 1; y < map.sizeY; ++y) {
      if (y < rockStart) {
         map.setRow(y, "dirt");
         map.setRow(y, "dirt", true);
      } else {
         map.setRow(y, "stone");
         map.setRow(y, "stone", true);
      }
   }
}

// Find a perfect spawn location for the player

Vector2 MapGenerator::findPlayerSpawnLocation() {
   setInfo("Finding Player Spawn Location...", 0.9f);
   float y = startY * map.sizeY + 1;
   int offset = 0;

   for (int x = map.sizeX / 2; x < map.sizeX && x >= 0; x = map.sizeX / 2 + offset) {
      while (y < map.sizeY - 1 && map.isu(x, y + 1, BlockType::empty)) { y++; }
      while (y > 0 && !map.isu(x, y, BlockType::empty)) { y--; }

      bool valid = true;
      for (int yy = 0; yy < 3; ++yy) {
         for (int xx = 0; xx < 2; ++xx) {
            if (!map.is(x + xx, y - yy, BlockType::empty)) {
               valid = false;
               goto breakOut;
            }
         }
      }
   breakOut:

      if (map.isLiquid(x, y) || map.isLiquid(x, y)) {
         valid = false;
      }

      if (valid) {
         return {(float)x, y - 2}; // To avoid spawning the player in ground
      }
      offset = -offset + (offset > 0 ? -1 : 1); // Search for the closest spawn point to center
   }
   return {map.sizeX / 2.0f, 0.0f}; // Just drop the player down from the sky if no spawn points were found
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

// Other functions

void MapGenerator::setInfo(const std::string &text, float progress) {
   std::lock_guard<std::mutex> lock(infoTextMutex);
   infoText = text;
   this->progress = progress;
}
