#ifndef OBJS_BLOCK_HPP
#define OBJS_BLOCK_HPP

#include <string>
#include <raylib.h>

// Block

struct Block {
   enum class Type { air, grass, dirt, stone, sand, water };
   
   Texture* tex = nullptr;
   Type type = Type::air;
   int id = 0;
};

// Set/get block function

void setBlock(Block& block, const std::string& name);
Color& getBlockColor(int blockId);

#endif
