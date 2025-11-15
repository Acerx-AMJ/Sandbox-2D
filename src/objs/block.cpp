#include <array>
#include <unordered_map>
#include "mngr/resource.hpp"
#include "objs/block.hpp"
#include "util/format.hpp" // IWYU pragma: export

// Constants

constexpr int idCount = 8;

static std::unordered_map<std::string, int> blockIds {
   {"air", 0}, {"grass", 1}, {"dirt", 2}, {"clay", 3}, {"stone", 4}, {"sand", 5}, {"sandstone", 6}, {"water", 7} 
};

static std::array<Block::Type, idCount> blockTypes {{
   Block::Type::air, Block::Type::grass, Block::Type::dirt, Block::Type::dirt, Block::Type::stone,
   Block::Type::sand, Block::Type::stone, Block::Type::water
}};

static std::array<Color, idCount> blockColors {{
   {0, 0, 0, 0}, {28, 152, 29, 255}, {117, 56, 19, 255}, {158, 91, 35, 255}, {102, 102, 102, 255},
   {255, 189, 40, 255}, {247, 134, 13, 255}, {8, 69, 165, 255}
}};

// Set/get block functions

void setBlock(Block& block, const std::string& name) {
   assert(blockIds.find(name) != blockIds.end(), "Block with the name '{}' does not exist.", name);
   block.tex = &ResourceManager::get().getTexture(name);
   block.id = blockIds[name];
   block.type = blockTypes[block.id];
}

Color& getBlockColor(int blockId) {
   return blockColors[blockId];
}
