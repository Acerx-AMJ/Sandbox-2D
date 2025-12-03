#include "objs/furniture.hpp"
#include "mngr/resource.hpp"
#include "objs/generation.hpp"
#include "objs/map.hpp"
#include "util/random.hpp"
#include <array>
#include <unordered_map>

// Constants

constexpr int furnitureCount = 10;
static std::unordered_map<std::string, int> furnitureTextureIds {
   {"tree", 0}, {"sapling", 1}, {"palm", 2}, {"palm_sapling", 3}, {"pine", 4}, {"pine_sapling", 5},
   {"jungle_tree", 6}, {"jungle_sapling", 7}, {"cactus", 8}, {"cactus_seed", 9}
};

constexpr std::array<const char*, furnitureCount> furnitureTextureNames {
   "tree", "sapling", "palm", "palm_sapling", "pine", "pine_sapling",
   "jungle_tree", "jungle_sapling", "cactus", "cactus_seed"
};

constexpr int texSize = 8;

// Helper functions

inline void setBlock(FurniturePiece &piece, int tx, int ty) {
   piece.nil = false;
   piece.tx = tx;
   piece.ty = ty;
}

inline bool isBlockSoil(Map &map, int x, int y) {
   return map.is(x, y, Block::grass) || map.isu(x, y, Block::dirt) || map.isu(x, y, Block::sand) || map.isu(x, y, Block::snow);
}

// Furniture functions

Furniture::Furniture(Type type, id_t texId, int value, int value2, int posX, int posY, int sizeX, int sizeY)
   : type(type), texId(texId), value(value), value2(value2), posX(posX), posY(posY), sizeX(sizeX), sizeY(sizeY) {
   pieces = std::vector<std::vector<FurniturePiece>>(sizeY, std::vector<FurniturePiece>(sizeX, FurniturePiece{}));   
}

Furniture::Furniture(const std::string &texture, int posX, int posY, int sizeX, int sizeY, Type type)
   : type(type), texId(furnitureTextureIds[texture]), posX(posX), posY(posY), sizeX(sizeX), sizeY(sizeY) {
   pieces = std::vector<std::vector<FurniturePiece>>(sizeY, std::vector<FurniturePiece>(sizeX, FurniturePiece{}));
}

// Update furniture

void Furniture::update(Map &map) {
   switch (type) {

   case Type::tree: {
      if (!isBlockSoil(map, posX + 1, posY + sizeY)) {
         map.removeFurniture(*this);
         return;
      }
   } break;

   case Type::sapling: {
      if (!isBlockSoil(map, posX, posY + sizeY)) {
         map.removeFurniture(*this);
         return;
      }

      if (value == 0) {
         value2 = random(200, 1500);
      }

      value += 1;
      if (value >= value2) {
         map.removeFurniture(*this);
         Furniture::generate(posX, posY + 1, map, Type::tree);
      }
   } break;

   case Type::cactus: {
      if (!map.isu(posX + 1, posY + sizeY, Block::sand)) {
         map.removeFurniture(*this);
         return;
      }
   } break;

   case Type::cactus_seed: {
      if (!map.isu(posX, posY + sizeY, Block::sand)) {
         map.removeFurniture(*this);
         return;
      }

      if (value == 0) {
         value2 = random(350, 1750);
      }

      value += 1;
      if (value >= value2) {
         map.removeFurniture(*this);
         Furniture::generate(posX, posY, map, Type::cactus);
      }
   } break;

   default: break;
   }
}

// Get functions

Furniture Furniture::get(int x, int y, Map &map, Type type, bool debug) {
   switch (type) {

   case Type::tree: {
      if (!debug && (x < 1 || x >= map.sizeX - 1 || y < 0 || !isBlockSoil(map, x , y + 1))) {
         return {};
      }
      
      bool palm = (map.isu(x, y + 1, Block::sand));
      int height = (palm ? random(8, 22) : random(5, 18));
      for (int i = 0; i < height && i < map.sizeY; ++i) {
         if (!map.empty(x, y - i)) {
            height = i;
            break;
         }
      }

      if (!debug && (height < (palm ? 8 : 5))) {
         return {};
      }
      static std::unordered_map<Block::id_t, std::string> textureMap {
         {Block::getId("grass"), "tree"}, {Block::getId("dirt"), "tree"}, {Block::getId("sand"), "palm"},
         {Block::getId("snow"), "pine"}, {Block::getId("mud"), "jungle_tree"}, {Block::getId("jungle_grass"), "jungle_tree"}
      };

      Furniture tree (textureMap[map.blocks[y + 1][x].id], x - 1, y - height + 1, 3, height, Furniture::tree);
      int offsetTx = (chance(50) ? 0 : 3 * texSize);

      for (int i = 0; i < (palm ? 3 : 2); ++i) {
         for (int j = 0; j < 3; ++j) {
            setBlock(tree.pieces[i][j], offsetTx + j * texSize, i * texSize);
         }
      }

      for (int i = (palm ? 3 : 2); i < height; ++i) {
         FurniturePiece &piece = tree.pieces[i][1];
         if (palm) {
            setBlock(piece, random(0, 2) * texSize, (i + 1 == height ? 4 : 3) * texSize);
            continue;
         }

         setBlock(piece, 2 * texSize, 3 * texSize);

         if (i + 1 == height) {
            bool rightFree = (map.empty(tree.posX + 2, tree.posY + i) && isBlockSoil(map, tree.posX + 2, tree.posY + i + 1) && chance(25));
            bool leftFree = (map.empty(tree.posX, tree.posY + i) && isBlockSoil(map, tree.posX, tree.posY + i + 1) && chance(25));

            if (rightFree) {
               setBlock(tree.pieces[i][2], 4 * texSize, 4 * texSize);
               piece.tx = 3 * texSize;
               piece.ty = 4 * texSize;
            }
            if (leftFree) {
               setBlock(tree.pieces[i][0], 0 * texSize, 4 * texSize);
               piece.tx = 1 * texSize;
               piece.ty = 4 * texSize;
            }
            if (rightFree && leftFree) {
               piece.tx = 5 * texSize;
               piece.ty = 4 * texSize;
            } else if (!rightFree && !leftFree) {
               piece.ty = 4 * texSize;
            }
         } else {
            bool rightFree = (map.empty(tree.posX + 2, tree.posY + i) && chance(15));
            bool leftFree = (map.empty(tree.posX, tree.posY + i) && chance(15));

            if (rightFree) {
               setBlock(tree.pieces[i][2], random(3, 5) * texSize, 2 * texSize);
               piece.tx = 3 * texSize;
               piece.ty = 3 * texSize;
            }
            if (leftFree) {
               setBlock(tree.pieces[i][0], random(0, 2) * texSize, 2 * texSize);
               piece.tx = 1 * texSize;
               piece.ty = 3 * texSize;
            }
            if (rightFree && leftFree) {
               piece.tx = 5 * texSize;
               piece.ty = 3 * texSize;
            }
         }
      }
      return tree;
   } break;

   case Type::sapling: {
      if (!debug && (x < 0 || x >= map.sizeX || y < 0 || !isBlockSoil(map, x , y + 2) || !map.empty(x, y) || !map.empty(x, y + 1))) {
         return {};
      }

      static std::unordered_map<Block::id_t, std::string> textureMap {
         {Block::getId("grass"), "sapling"}, {Block::getId("dirt"), "sapling"}, {Block::getId("sand"), "palm_sapling"},
         {Block::getId("snow"), "pine_sapling"}, {Block::getId("mud"), "jungle_sapling"}, {Block::getId("jungle_grass"), "jungle_sapling"}
      };
      id_t btype = map.blocks[y + 2][x].id;
      Furniture sapling ((!textureMap.count(btype) ? "sapling" : textureMap[btype]), x, y, 1, 2, Furniture::sapling);

      int value = random(0, 100);
      int offsetTx = (value < 33 ? 0 : (value < 66 ? texSize : 2 * texSize));

      setBlock(sapling.pieces[0][0], offsetTx, 0 * texSize);
      setBlock(sapling.pieces[1][0], offsetTx, 1 * texSize);
      return sapling;
   } break;

   case Type::cactus: {
      if (!debug && (x < 1 || x >= map.sizeX - 1 || y < 0 || !map.isu(x , y + 1, Block::sand))) {
         return {};
      }

      int height = random(4, 9);
      for (int i = 0; i < height && i < map.sizeY; ++i) {
         if (!map.empty(x, y - i)) {
            height = i;
            break;
         }
      }

      if (!debug && height <= 0) {
         return {};
      }

      Furniture cactus ("cactus", x - 1, y - height + 1, 3, height, Furniture::cactus);
      cactus.value = height;
      
      for (int i = 0; i < cactus.sizeY; ++i) {
         for (int j = 0; j < cactus.sizeX; ++j) {
            if (!map.empty(cactus.posX + j, cactus.posY + i)) {
               continue;
            }
            
            if (j == 1) {
               cactus.pieces[i][j].nil = false;
            } else if (i != cactus.sizeY - 1 && i != 0) {
               cactus.pieces[i][j].nil = chance(50);
            }
         }
      }

      for (int i = 0; i < cactus.sizeY; ++i) {
         for (int j = 0; j < cactus.sizeX; ++j) {
            FurniturePiece &piece = cactus.pieces[i][j];
            
            if (j == 1) {
               bool rightStub = (!cactus.pieces[i][2].nil && i < cactus.sizeY + 1 && cactus.pieces[i + 1][2].nil);
               bool leftStub = (!cactus.pieces[i][0].nil && i < cactus.sizeY + 1 && cactus.pieces[i + 1][0].nil);

               if (rightStub && leftStub) {
                  setBlock(piece, 0 * texSize, 3 * texSize);
               } else if (rightStub) {
                  setBlock(piece, 0 * texSize, 1 * texSize);
               } else if (leftStub) {
                  setBlock(piece, 0 * texSize, 2 * texSize);
               } else {
                  if (i == 0) {
                     setBlock(piece, 1 * texSize, (chance(10) ? 1 : 0) * texSize);
                  } else if (i == cactus.sizeY - 1) {
                     setBlock(piece, 1 * texSize, 3 * texSize);
                  } else {
                     setBlock(piece, 0 * texSize, 0 * texSize);
                  }
               }
               continue;
            } else if (!piece.nil) {
               int offsetX = (j == 0 ? 2 : 3) * texSize;

               if (cactus.pieces[i - 1][j].nil && cactus.pieces[i + 1][j].nil) {
                  setBlock(piece, offsetX, 3 * texSize);
               } else if (cactus.pieces[i - 1][j].nil) {
                  setBlock(piece, offsetX, 0 * texSize);
               } else if (cactus.pieces[i + 1][j].nil) {
                  setBlock(piece, offsetX, 2 * texSize);
               } else {
                  setBlock(piece, offsetX, 1 * texSize);
               }
            }
         }
      }
      return cactus;
   } break;

   case Type::cactus_seed: {
      printf(
         "GENERATING CACTUS SEED\n"
      );
      if (!debug && (!map.is(x, y + 1, Block::sand) or !map.empty(x, y))) {
         return {};
      }

      Furniture cactus_seed ("cactus_seed", x, y, 1, 1, Furniture::cactus_seed);
      int value = random(0, 100);
      int offsetTx = (value < 33 ? 0 : (value < 66 ? texSize : 2 * texSize));

      setBlock(cactus_seed.pieces[0][0], offsetTx * texSize, 0 * texSize);
      return cactus_seed;
   } break;

   default: return {};
   };
}

void Furniture::generate(int x, int y, Map &map, Type type) {
   Furniture furniture = Furniture::get(x, y, map, type);
   if (furniture.type != Furniture::none) {
      map.addFurniture(furniture);
   }
}

// Render furniture

void Furniture::preview(Map &map) {
   for (int y = posY; y - posY < sizeY; ++y) {
      for (int x = posX; x - posX < sizeX; ++x) {
         FurniturePiece &piece = pieces[y - posY][x - posX];
         if (piece.nil) {
            continue;
         }
         Color color = Fade((map.empty(x, y) ? WHITE : RED), .75f);
         DrawTexturePro(getTexture(furnitureTextureNames[texId]), {(float)piece.tx, (float)piece.ty, texSize, texSize}, {(float)x, (float)y, 1.f, 1.f}, {0, 0}, 0, color);
      }
   }
}

void Furniture::render(int minX, int minY, int maxX, int maxY) {
   for (int y = posY; y < maxY && y - posY < sizeY; ++y) {
      for (int x = posX; x < maxX && x - posX < sizeX; ++x) {
         FurniturePiece &piece = pieces[y - posY][x - posX];
         if (y < minY || x < minX || piece.nil) {
            continue;
         }
         DrawTexturePro(getTexture(furnitureTextureNames[texId]), {(float)piece.tx, (float)piece.ty, texSize, texSize}, {(float)x, (float)y, 1.f, 1.f}, {0, 0}, 0, WHITE);
      }
   }
}

// Id functions

Furniture::id_t Furniture::getId(const std::string &name) {
   return furnitureTextureIds[name];
}

std::string Furniture::getName(id_t id) {
   return furnitureTextureNames[id];
}
