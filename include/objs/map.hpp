#ifndef OBJS_MAP_HPP
#define OBJS_MAP_HPP

#include "objs/furniture.hpp"
#include <string>

// Block

struct Block {
   enum Type { air, grass, dirt, solid, platform, transparent, sand, snow, ice, water, lava };

   Texture *texture = nullptr;
   Type type = Type::air;

   // Unsigned chars can only hold 256 unique IDs. Currently trying to save
   // block space, so it's a problem for later
   unsigned char id = 0;
   bool furniture = false;

   // Values used by physics updates, specific to the block type
   unsigned char value = 0, value2 = 0;

   static unsigned char getId(const std::string &name);
   static std::string getName(unsigned char id);
};

// Map

struct Map {
   std::vector<std::vector<Block>> blocks, walls;
   std::vector<Furniture> furniture;
   int sizeX = 0, sizeY = 0;

   // Set block functions

   void init();
   void setBlock(int x, int y, const std::string &name, bool isWall = false);
   void setBlock(int x, int y, unsigned char id, bool isWall = false);
   void deleteBlock(int x, int y, bool isWall = false);
   void moveBlock(int oldX, int oldY, int newX, int newY);

   // Set furniture functions

   void addFurniture(Furniture &object);
   void removeFurniture(Furniture &object);

   // Get block functions

   bool isPositionValid(int x, int y) const;
   bool is(int x, int y, Block::Type type) const;
   bool isu(int x, int y, Block::Type type) const; // Unsafe is variant
   bool empty(int x, int y) const;
   bool isTransparent(int x, int y) const; // Unsafe

   std::vector<Block>& operator[](size_t index);

   // Render map

   void render(const Rectangle &cameraBounds) const;
};

#endif
