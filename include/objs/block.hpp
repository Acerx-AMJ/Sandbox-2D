#ifndef OBJS_BLOCK_HPP
#define OBJS_BLOCK_HPP

#include <string>
#include <vector>
#include <raylib.h>

// Block

struct Block {
   enum class Type { air, grass, dirt, solid, sand, water };
   
   Texture* tex = nullptr;
   Type type = Type::air;
   int id = 0;
};

using Row = std::vector<Block>;
using Map = std::vector<Row>;

// Set block functions

void setBlock(Block& block, const std::string& name);
void deleteBlock(Block& block);
void moveBlock(Block& oldBlock, Block& newBlock);

// Get block functions

bool isPositionValid(Map &map, int x, int y);
bool leftIs(Map& map, int x, int y, Block::Type type = Block::Type::air);
bool rightIs(Map& map, int x, int y, Block::Type type = Block::Type::air);
bool topIs(Map& map, int x, int y, Block::Type type = Block::Type::air);
bool bottomIs(Map& map, int x, int y, Block::Type type = Block::Type::air);
Color& getBlockColor(int blockId);

#endif
