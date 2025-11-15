#include "objs/generation.hpp"

// Includes

#include "PerlinNoise.hpp"
#include "util/random.hpp"

using namespace std::string_literals;
using namespace siv;

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

// Globals

int sizeX = 0;
int sizeY = 0;

// Private functions

inline float normalizedNoise2D(PerlinNoise& noise, int x, int y, float amplitude) {
   return (noise.octave2D(x * amplitude, y * amplitude, 4) + 1.f) / 2.f;
}

// Generate functions

void generateMap(Map& map, int sizeX, int sizeY) {
   map = Map(sizeY, std::vector<Block>(sizeX, Block{}));
   ::sizeX = sizeX;
   ::sizeY = sizeY;

   generateTerrain(map);
   generateDebri(map);
   generateWater(map);
}

void generateTerrain(Map& map) {
   PerlinNoise noise (rand());
   int y = startY * sizeY;
   int rockOffset = rockOffsetStart;

   for (int x = 0; x < sizeX; ++x) {
      float value = normalizedNoise2D(noise, x, y, 0.01f);

      // Get different height increase/decrease based on the noise

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

      if (y < sizeY * highThreshold) {
         ++y;
         ++rockOffset;
      }

      if (y > sizeY * lowThreshold) {
         --y;
         --rockOffset;
      }
      y = std::clamp<int>(y, sizeY * highestPoint, sizeY * lowestPoint);
      rockOffset = std::clamp(rockOffset, rockOffsetMin, rockOffsetMax);

      // Generate grass, dirt and stone

      setBlock(map[y][x], "grass"s);
      for (int yy = y + 1; yy < sizeY; ++yy) {
         setBlock(map[yy][x], (yy - y < rockOffset ? "dirt"s : "stone"s));
      }
   }
}

void generateWater(Map& map) {
   int seaY = sizeY * seaLevel;
   for (int x = 0; x < sizeX; ++x) {
      for (int y = seaY; y < sizeY and map[y][x].type == Block::Type::air; ++y) {
         setBlock(map[y][x], "water"s);
      }
   }
}

void generateDebri(Map& map) {
   PerlinNoise sandNoise (rand());
   PerlinNoise dirtNoise (rand());

   for (int x = 0; x < sizeX; ++x) {
      for (int y = 0; y < sizeY; ++y) {
         if (map[y][x].type == Block::Type::air or map[y][x].type == Block::Type::grass) {
            continue;
         }

         float value = normalizedNoise2D(dirtNoise, x, y, 0.04f);
         if (value >= .825f) {
            setBlock(map[y][x], "clay"s);
         } else if (value <= .2f) {
            setBlock(map[y][x], "dirt"s);
         } else if (map[y][x].type != Block::Type::dirt and normalizedNoise2D(sandNoise, x, y, 0.04f) <= .15f) {
            setBlock(map[y][x], "sand"s);
         }
      }
   }
}
