#include "mngr/fileio.hpp"
#include "objs/generation.hpp"
#include "SRU/audio.hpp"
#include "SRU/random.hpp"
#include <thread>

// Constants

constexpr float startY         = 0.5f;
constexpr float seaLevel       = 0.475f;
constexpr float tier2OreStartY = 0.45f;

constexpr int rockOffsetStart = 12;
constexpr int rockOffsetMin   = 5;
constexpr int rockOffsetMax   = 25;
constexpr int maxWaterLength  = 60;

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
   map.initThreadSafe();
   setInfo("Initializing...", 0.0f);
}

// Generation functions

void MapGenerator::generate() {
   map.initContainers(); // Do expensive initialization in a thread
   rockStartHeights.resize(map.sizeX);

   setInfo("Seeding Noise...", 0.0f);
   if (!isFlat) {
      biomeTemperatureNoise.reseed(rand());
      biomeMoistureNoise.reseed(rand());
      heightNoise.reseed(rand());
      sandDebriNoise.reseed(rand());
      dirtDebriNoise.reseed(rand());
      oreNoise1.reseed(rand());
      oreNoise2.reseed(rand());
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
   saveWorldData(name, spawnLocation, spawnLocation, false, 100, 100, 100, 50.f, map, nullptr, nullptr, nullptr);

   setInfo("Generating Completed!", 1.0f);
   playSound("success");
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
      y += randomInt(data.hmin[height], data.hmax[height]);

      // 2 is the middle point in our data
      if ((height == 2 && chance(data.rng)) || height != 2) {
         y += randomInt(-1, 1);
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
         y += data.hmin[randomInt(0, 2)];
      }

      y = std::clamp(y, 0, map.sizeY - 1);
      rockOffset = std::clamp(rockOffset, rockOffsetMin, rockOffsetMax);

      // Generate grass, dirt and stone

      map.setBlock(x, y, (last != current && chance(50) ? lastData.top : data.top));
      for (int yy = y + 1; yy < map.sizeY; ++yy) {
         if (yy - y < rockOffset) {
            const std::string &block = (last != current && chance(50) ? lastData.bottom : data.bottom);
            map.setBlock(x, yy, block);

            if (block != "sand" && block != "snow") {
               map.setWall(x, yy, block);
            }
         } else {
            rockStartHeights[x] = yy;
            map.setColumnFromPoint(x, yy, "stone");
            break;
         }
      }
   }
}

void MapGenerator::generateWater() {
   setInfo("Filling Water...", 0.75f);

   int seaY = map.sizeY * seaLevel;
   blockid_t iceid = getBlockIdFromName("ice");
   liquidid_t waterid = getLiquidIdFromName("water");

   for (int x = 0; x < map.sizeX; ++x) {
      for (int y = seaY; y < map.sizeY && map.isEmpty(x, y); ++y) {
         if (y == seaY && biomeData[(int)getBiome(x)].wamth == BiomeWarmth::cold) {
            map.setBlock(x, y, iceid);
         } else {
            map.setLiquid(x, y, waterid, maxLiquidLayers);
         }
      }
   }
}

void MapGenerator::generateDebri() {
   setInfo("Generating Debri and Ores...", 0.5f);

   blockid_t clayid = getBlockIdFromName("clay");
   blockid_t dirtid = getBlockIdFromName("dirt");
   blockid_t sandid = getBlockIdFromName("sand");
   blockid_t coalid = getBlockIdFromName("coal_ore");
   blockid_t ironid = getBlockIdFromName("iron_ore");
   blockid_t goldid = getBlockIdFromName("gold_ore");
   blockid_t mythid = getBlockIdFromName("mythril_ore");

   int tier2OreY = tier2OreStartY * map.sizeY;

   for (int x = 0; x < map.sizeX; ++x) {
      for (int y = rockStartHeights[x]; y < map.sizeY; ++y) {
         float value = dirtDebriNoise.octave2D(x * 0.05f, y * 0.05f, 3);

         // Debris
         if (value >= 0.6125f) {
            map.setBlock(x, y, clayid);
         } else if (value <= -0.6f) {
            map.setBlock(x, y, dirtid);
         } else if (sandDebriNoise.octave2D(x * 0.05f, y * 0.05f, 3) <= -0.7f) {
            map.setBlock(x, y, sandid);

         // Tier 1 ores (coal, iron)
         } else {

         float ovalue1 = oreNoise1.octave2D(x * 0.1f, y * 0.1f, 3);
         if (ovalue1 >= 0.65f) {
            map.setBlock(x, y, coalid);
         } else if (ovalue1 <= -0.7f) {
            map.setBlock(x, y, ironid);

         // Tier 2 ores (gold, mythril)
         } else if (y >= tier2OreY) {

         float ovalue2 = oreNoise2.octave2D(x * 0.125f, y * 0.125f, 3);
         if (ovalue2 >= 0.725f) {
            map.setBlock(x, y, goldid);
         } else if (ovalue2 <= -0.75f) {
            map.setBlock(x, y, mythid);
         }

         }
         }
      }
   }
   rockStartHeights.clear();
}

void MapGenerator::generateTrees() {
   setInfo("Growing Trees...", 0.85f);

   float y = startY * map.sizeY + 1;
   int counter = 0, counterThreshold = 0;
   
   for (int x = 0; x < map.sizeX; ++x) {
      while (y < map.sizeY - 1 && map.isEmpty(x, y + 1)) { y++; }
      while (y > 0 && !map.isEmpty(x, y)) { y--; }

      if (counter < counterThreshold || !chance(biomeData[(int)getBiome(x)].treeRate)) {
         counter++;
         continue;
      }
      blockid_t soilId = map.getBlock(x, y + 1).id;
      bool sapling = (isSaplingSoil(soilId) && chance(5)) || (isSaplingSoil(soilId) && !isTreeSoil(soilId));
   
      if (sapling || isTreeSoil(soilId)) {
         furnitureid_t id = randomElement(sapling ? getSaplingsFromSoil(soilId) : getTreesFromSoil(soilId));
         generateFurniture(x, y - sapling * (getFurnitureData(id).furnitureSize.y - 1), map, id, false);
      }
      counter = 0;
      counterThreshold = randomInt(1, 4);
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
         map.setWallRow(y, "dirt");
      } else {
         map.setRow(y, "stone");
         map.setWallRow(y, "stone");
      }
   }
}

// Find a perfect spawn location for the player

Vector2 MapGenerator::findPlayerSpawnLocation() {
   setInfo("Finding Player Spawn Location...", 0.9f);
   float y = startY * map.sizeY + 1;
   int offset = 0;

   for (int x = map.sizeX / 2; x < map.sizeX && x >= 0; x = map.sizeX / 2 + offset) {
      while (y < map.sizeY - 1 && map.isEmpty(x, y + 1)) { y++; }
      while (y > 0 && !map.isEmpty(x, y)) { y--; }

      bool valid = true;
      for (int yy = 0; yy < 3; ++yy) {
         for (int xx = 0; xx < 2; ++xx) {
            if (!map.isEmpty(x + xx, y - yy)) {
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
