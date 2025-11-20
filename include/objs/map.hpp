#ifndef OBJS_BLOCK_HPP
#define OBJS_BLOCK_HPP

#include <string>
#include <vector>
#include <raylib.h>

// Block

struct Block {
   enum Type { air, grass, dirt, solid, transparent, sand, water };
   
   Texture* tex = nullptr;
   Type type = Type::air;
   int id = 0;

   Color& getColor();
   Color& getWallColor();
   static void initializeWallColors();
};

// Map

struct Map {
   std::vector<std::vector<Block>> blocks;
   std::vector<std::vector<Block>> walls;
   int sizeX = 0;
   int sizeY = 0;

   // Set block functions

   void setSize(int x, int y);
   void setBlock(int x, int y, const std::string& name, bool walls = false);
   void deleteBlock(int x, int y, bool walls = false);
   void moveBlock(int ox, int oy, int nx, int ny);

   // Get block functions

   bool isPositionValid(int x, int y);
   bool is(int x, int y, Block::Type type);
   bool isTransparent(int x, int y);

   std::vector<Block>& operator[](size_t index);

   // Render map

   void render(Camera2D& camera);
};

#endif
