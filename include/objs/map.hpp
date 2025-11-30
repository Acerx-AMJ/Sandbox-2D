#ifndef OBJS_BLOCK_HPP
#define OBJS_BLOCK_HPP

#include "objs/furniture.hpp"
#include <string>

// Block

struct Block {
   enum Type { air, grass, dirt, solid, platform, transparent, sand, snow, ice, water, lava };
   using id_t = unsigned char;
   
   Texture* tex = nullptr;
   Type type = Type::air;

   // Unsigned chars can only hold 256 unique IDs. Currently trying to save
   // block space, so it's a problem for later
   id_t id = 0;
   bool furniture = false;

   // Values used by physics updates, specific to the block type
   unsigned char value = 0, value2 = 0;

   Color& getColor();
   Color& getWallColor();
   static void initializeColors();
   static id_t getId(const std::string& name);
   static Color& getColorFromId(Block::id_t id);
};

// Map

struct Map {
   std::vector<std::vector<Block>> blocks;
   std::vector<std::vector<Block>> walls;
   std::vector<Furniture> furniture;
   int sizeX = 0;
   int sizeY = 0;

   // Set block functions

   void init();
   void setBlock(int x, int y, const std::string& name, bool walls = false);
   void setBlock(int x, int y, Block::id_t id, bool walls = false);
   void deleteBlock(int x, int y, bool walls = false);
   void moveBlock(int ox, int oy, int nx, int ny);

   // Set furniture functions

   void addFurniture(Furniture& furniture);
   void removeFurniture(Furniture& furniture);

   // Get block functions

   bool isPositionValid(int x, int y);
   bool is(int x, int y, Block::Type type);
   bool isu(int x, int y, Block::Type type); // Unsafe is variant
   bool empty(int x, int y); // Unsafe too
   bool isTransparent(int x, int y);

   std::vector<Block>& operator[](size_t index);

   // Render map

   void render(Camera2D& camera);
};

#endif
