#include "mngr/resource.hpp"
#include "objs/furniture.hpp"
#include "objs/inventory.hpp"
#include "objs/map.hpp"
#include "objs/player.hpp"
#include "util/random.hpp"
#include <array>
#include <raymath.h>
#include <unordered_map>

// Constants

constexpr int cactusGrowSpeedMin  = 8 * 350;
constexpr int cactusGrowSpeedMax  = 8 * 1750;
constexpr int saplingGrowSpeedMin = 8 * 200;
constexpr int saplingGrowSpeedMax = 8 * 1500;

constexpr int cactusSizeMin = 4;
constexpr int cactusSizeMax = 9;
constexpr int palmSizeMin   = 8;
constexpr int palmSizeMax   = 22;
constexpr int treeSizeMin   = 5;
constexpr int treeSizeMax   = 18;

constexpr int treeRootChance     = 25;
constexpr int treeBranchChance   = 15;
constexpr int cactusBranchChance = 50;
constexpr int cactusFlowerChance = 10;

constexpr int sitRange = 25.0f; // Squared
constexpr int interactionRange = 64.0f; // Squared

constexpr int textureSize = 8;

// Furniture ID constants

constexpr int furnitureCount = 13;

static inline const std::unordered_map<std::string, unsigned short> furnitureTextureIds {
   {"tree", 0}, {"sapling", 1}, {"palm", 2}, {"palm_sapling", 3}, {"pine", 4},
   {"pine_sapling", 5}, {"jungle_tree", 6}, {"jungle_sapling", 7}, {"cactus", 8}, {"cactus_seed", 9},
   {"table", 10}, {"chair", 11}, {"door", 12}
};

constexpr static inline std::array<const char*, furnitureCount> furnitureTextureNames {
   "tree", "sapling", "palm", "palm_sapling", "pine",
   "pine_sapling", "jungle_tree", "jungle_sapling", "cactus", "cactus_seed",
   "table", "chair", "door"
};

constexpr static inline std::array<FurnitureType, furnitureCount> furnitureTypes {
   FurnitureType::tree, FurnitureType::sapling, FurnitureType::tree, FurnitureType::sapling, FurnitureType::tree,
   FurnitureType::sapling, FurnitureType::tree, FurnitureType::sapling, FurnitureType::cactus, FurnitureType::cactusSeed,
   FurnitureType::table, FurnitureType::chair, FurnitureType::door,
};

constexpr static inline std::array<float, furnitureCount> furnitureBreakingTimes {
   5.0f, // tree
   0.25f, // sapling
   5.0f, // palm tree
   0.25f, // palm sapling
   5.0f, // pine tree
   0.25f, // pine sapling
   5.0f, // jungle tree
   0.25f, // jungle sapling
   2.5f, // cactus
   0.25f, // cactus seed
   1.0f, // table
   1.0f, // chair
   1.0f, // door
};

// Furniture drops

struct FurnitureItem {
   const char *name;
   bool isWall = false;
   bool isFurniture = false;
   int min = 0;
   int max = 0;
};

struct FurnitureDrop {
   std::vector<FurnitureItem> items;
   int dropLevel = 0;
};

static inline const std::array<FurnitureDrop, furnitureCount> furnitureDrops {
   FurnitureDrop{{{"planks", false, false, 20, 35}, {"sapling", false, true, 0, 3}}}, // tree
   FurnitureDrop{}, // sapling
   FurnitureDrop{{{"planks", false, false, 20, 35}, {"sapling", false, true, 0, 3}}}, // palm
   FurnitureDrop{}, // palm sapling
   FurnitureDrop{{{"planks", false, false, 20, 35}, {"sapling", false, true, 0, 3}}}, // pine
   FurnitureDrop{}, // pine sapling
   FurnitureDrop{{{"planks", false, false, 20, 35}, {"sapling", false, true, 0, 3}}}, // jungle
   FurnitureDrop{}, // jungle sapling
   FurnitureDrop{{{"cactus_block", false, false, 5, 17}, {"cactus_seed", false, true, 0, 2}}}, // cactus
   FurnitureDrop{}, // cactus seed
   FurnitureDrop{{{"table", false, true, 1, 1}}}, // table
   FurnitureDrop{{{"chair", false, true, 1, 1}}}, // chair
   FurnitureDrop{{{"door", false, true, 1, 1}}}, // door
};

// Helper functions

inline void setBlock(FurniturePiece &piece, int tx, int ty) {
   piece.nil = false;
   piece.tx = tx;
   piece.ty = ty;
}

// Furniture constructors

Furniture::Furniture(FurnitureType type, unsigned short id, short value, short value2, int posX, int posY, short sizeX, short sizeY)
   : type(type), id(id), value(value), value2(value2), posX(posX), posY(posY), sizeX(sizeX), sizeY(sizeY) {
   pieces = std::vector<std::vector<FurniturePiece>>(sizeY, std::vector<FurniturePiece>(sizeX, FurniturePiece{}));   
}

Furniture::Furniture(const std::string &texture, int posX, int posY, short sizeX, short sizeY, FurnitureType type)
   : type(type), id(furnitureTextureIds.at(texture)), posX(posX), posY(posY), sizeX(sizeX), sizeY(sizeY) {
   pieces = std::vector<std::vector<FurniturePiece>>(sizeY, std::vector<FurniturePiece>(sizeX, FurniturePiece{}));
}

// Destroy furniture

void Furniture::destroy(Map &map, Inventory &inventory, int cursorX, int cursorY, int breakingLevel) {
   map.removeFurniture(*this);
   const FurnitureDrop &drop = furnitureDrops.at(id);

   if (drop.dropLevel > breakingLevel) {
      return;
   }

   for (const FurnitureItem &item: drop.items) {
      Item dropped {ItemType::block, (item.isFurniture ? getFurnitureIdFromName(item.name) : getBlockIdFromName(item.name)), static_cast<unsigned short>(random(item.min, item.max)), item.isFurniture, item.isWall, false};
      inventory.tryToPlaceItemOrDropAtCoordinates(dropped, cursorX, cursorY);
   }
}

// Update furniture

void Furniture::update(Map &map, Player &player, const Vector2 &mousePos) {
   // If the furniture isn't valid, just destroy it immediately
   if (!isValid(map)) {
      map.removeFurniture(*this);
      return;
   }
   
   // Update specific furniture
   switch (type) {
   case FurnitureType::sapling: {
      if (value == 0) {
         value2 = random(saplingGrowSpeedMin, saplingGrowSpeedMax);
      }

      value += 1;
      if (value >= value2) {
         map.removeFurniture(*this);
         generateFurniture(posX, posY + 1, map, FurnitureType::tree, false); // Not important for trees
      }
   } break;

   case FurnitureType::cactusSeed: {
      if (value == 0) {
         value2 = random(cactusGrowSpeedMin, cactusGrowSpeedMax);
      }

      value += 1;
      if (value >= value2) {
         map.removeFurniture(*this);
         generateFurniture(posX, posY, map, FurnitureType::cactus, false); // Same with cacti
      }
   } break;

   case FurnitureType::chair: {
      Vector2 chairpos = {(float)posX, (float)posY};

      if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) && CheckCollisionPointRec(mousePos, {(float)posX, (float)posY, (float)sizeX, (float)sizeY})) {
         float range = Vector2DistanceSqr(chairpos, player.getCenter());
         
         // Raycast to make sure the player does not teleport
         if (range > sitRange || map.raycast(chairpos, player.getCenter())) {
            return;
         }
         
         player.placedBlock = true;
         player.sitting = true;
         player.flipX = (pieces[0][0].tx == 0); // Flip player if chair is flipped
         player.position.x = posX - (player.flipX ? 0 : 1); // Adjust flipped player's position
         player.position.y = posY - 1;
      }
   } break;

   case FurnitureType::door: {
      Rectangle doorRect = {(float)posX, (float)posY, (float)sizeX, (float)sizeY};
      bool previousValue = value;
      bool previousValue2 = value2;

      value2 = CheckCollisionRecs(doorRect, player.getBounds());
      if (previousValue2 == true && value2 == false && (!value || !value2)) {
         value = false; // Close the door
      } else if (previousValue2 == false && value2 == true && (!value || !value2)) {
         value = true; // Open the door
      }

      if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) && CheckCollisionPointRec(mousePos, doorRect) && Vector2DistanceSqr({(float)posX, (float)posY}, player.getCenter()) < interactionRange) {
         player.placedBlock = true;
         value = !value;
         value2 = false;
      }

      for (int i = 0; value != previousValue && i < sizeY; ++i) {
         pieces[i][0].ty = (value ? textureSize * (i + 3) : textureSize * i);
      }
   } break;

   default: break;
   }
}

bool Furniture::isValid(const Map &map) const {
   switch (type) {
   case FurnitureType::tree:
      return map.isSoil(posX + 1, posY + sizeY);
   
   case FurnitureType::sapling:
      return map.isSoil(posX, posY + sizeY) && !map.is(posX, posY - 1, BlockType::sand) && !(map.isLiquid(posX, posY) && map.isLiquid(posX, posY + 1));
   
   case FurnitureType::cactus:
      return map.blocks[posY + sizeY][posX + 1].id == getBlockIdFromName("sand");

   case FurnitureType::cactusSeed:
      return map.blocks[posY + 1][posX].id == getBlockIdFromName("sand") && !map.is(posX, posY - 1, BlockType::sand) && !map.isLiquid(posX, posY);
   
   case FurnitureType::table:
      return map.is(posX, posY + sizeY, BlockType::solid) && map.is(posX + 2, posY + sizeY, BlockType::solid);

   case FurnitureType::chair:
      return map.is(posX, posY + sizeY, BlockType::solid);
   
   case FurnitureType::door:
      return map.is(posX, posY + sizeY, BlockType::solid) && map.is(posX, posY - 1, BlockType::solid);

   default:
      return false;
   }
}

// Get functions

Furniture getFurniture(int x, int y, const Map &map, FurnitureType type, bool playerFacingLeft, bool debug) {
   switch (type) {
   case FurnitureType::tree: {
      if (!debug && (x < 1 || x >= map.sizeX - 1 || y < 0 || !map.isSoil(x , y + 1))) {
         return {};
      }

      bool palm = map.blocks[y + 1][x].id == getBlockIdFromName("sand");
      int height = (palm ? random(palmSizeMin, palmSizeMax) : random(treeSizeMin, treeSizeMax));

      for (int i = 0; i < height && i < map.sizeY; ++i) {
         if (!map.isNotSolid(x, y - i)) {
            height = i;
            break;
         }
      }

      if (!debug && (height < (palm ? palmSizeMin : treeSizeMin))) {
         return {};
      }
      static std::unordered_map<unsigned short, std::string> textureMap {
         {getBlockIdFromName("grass"), "tree"}, {getBlockIdFromName("dirt"), "tree"},        {getBlockIdFromName("sand"),         "palm"},
         {getBlockIdFromName("snow"),  "pine"}, {getBlockIdFromName("mud"),  "jungle_tree"}, {getBlockIdFromName("jungle_grass"), "jungle_tree"}
      };

      Furniture tree (textureMap[map.blocks[y + 1][x].id], x - 1, y - height + 1, 3, height, FurnitureType::tree);
      int offsetTx = (chance(50) ? 0 : 3 * textureSize);

      // Generate the tree top
      for (int i = 0; i < (palm ? 3 : 2); ++i) {
         for (int j = 0; j < 3; ++j) {
            setBlock(tree.pieces[i][j], offsetTx + j * textureSize, i * textureSize);
         }
      }

      // Generate everything from the trunk to the sticks and roots
      for (int i = (palm ? 3 : 2); i < height; ++i) {
         FurniturePiece &piece = tree.pieces[i][1];
         if (palm) {
            setBlock(piece, random(0, 2) * textureSize, (i + 1 == height ? 4 : 3) * textureSize);
            continue;
         }

         setBlock(piece, 2 * textureSize, 3 * textureSize);

         // Trunk logic
         if (i + 1 == height) {
            bool rightFree = (map.isNotSolid(tree.posX + 2, tree.posY + i) && map.isSoil(tree.posX + 2, tree.posY + i + 1) && chance(treeRootChance));
            bool leftFree = (map.isNotSolid(tree.posX, tree.posY + i) && map.isSoil(tree.posX, tree.posY + i + 1) && chance(treeRootChance));

            if (rightFree) {
               setBlock(tree.pieces[i][2], 4 * textureSize, 4 * textureSize);
               piece.tx = 3 * textureSize;
               piece.ty = 4 * textureSize;
            }
            if (leftFree) {
               setBlock(tree.pieces[i][0], 0 * textureSize, 4 * textureSize);
               piece.tx = 1 * textureSize;
               piece.ty = 4 * textureSize;
            }
            if (rightFree && leftFree) {
               piece.tx = 5 * textureSize;
               piece.ty = 4 * textureSize;
            } else if (!rightFree && !leftFree) {
               piece.ty = 4 * textureSize;
            }

         // Branch logic
         } else {
            bool rightFree = (map.isNotSolid(tree.posX + 2, tree.posY + i) && chance(treeBranchChance));
            bool leftFree = (map.isNotSolid(tree.posX, tree.posY + i) && chance(treeBranchChance));

            if (rightFree) {
               setBlock(tree.pieces[i][2], random(3, 5) * textureSize, 2 * textureSize);
               piece.tx = 3 * textureSize;
               piece.ty = 3 * textureSize;
            }
            if (leftFree) {
               setBlock(tree.pieces[i][0], random(0, 2) * textureSize, 2 * textureSize);
               piece.tx = 1 * textureSize;
               piece.ty = 3 * textureSize;
            }
            if (rightFree && leftFree) {
               piece.tx = 5 * textureSize;
               piece.ty = 3 * textureSize;
            }
         }
      }
      return tree;
   } break;

   case FurnitureType::sapling: {
      if (!debug && (x < 0 || x >= map.sizeX || y < 0 || !map.isSoil(x , y + 2) || !map.isEmpty(x, y) || !map.isEmpty(x, y + 1))) {
         return {};
      }

      static std::unordered_map<unsigned short, std::string> textureMap {
         {getBlockIdFromName("grass"), "sapling"},      {getBlockIdFromName("dirt"), "sapling"},        {getBlockIdFromName("sand"),         "palm_sapling"},
         {getBlockIdFromName("snow"),  "pine_sapling"}, {getBlockIdFromName("mud"),  "jungle_sapling"}, {getBlockIdFromName("jungle_grass"), "jungle_sapling"}
      };

      int offsetTx = (random(0, 100) / 33) * textureSize;
      unsigned short btype = map.blocks[y + 2][x].id;
      Furniture sapling ((!textureMap.count(btype) ? "sapling" : textureMap[btype]), x, y, 1, 2, FurnitureType::sapling);

      setBlock(sapling.pieces[0][0], offsetTx, 0 * textureSize);
      setBlock(sapling.pieces[1][0], offsetTx, 1 * textureSize);
      return sapling;
   } break;

   case FurnitureType::cactus: {
      if (!debug && (x < 1 || x >= map.sizeX - 1 || y < 0 || y >= map.sizeY || map.blocks[y + 1][x].id != getBlockIdFromName("sand"))) {
         return {};
      }

      int height = random(cactusSizeMin, cactusSizeMax);
      for (int i = 0; i < height && i < map.sizeY; ++i) {
         if (!map.isNotSolid(x, y - i)) {
            height = i;
            break;
         }
      }

      if (!debug && height <= 0) {
         return {};
      }

      Furniture cactus ("cactus", x - 1, y - height + 1, 3, height, FurnitureType::cactus);

      for (int i = 0; i < cactus.sizeY; ++i) {
         for (int j = 0; j < cactus.sizeX; ++j) {
            if (!map.isNotSolid(cactus.posX + j, cactus.posY + i)) {
               continue;
            }
            
            if (j == 1) {
               cactus.pieces[i][j].nil = false;
            } else if (i != cactus.sizeY - 1 && i != 0) {
               cactus.pieces[i][j].nil = chance(cactusBranchChance);
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
                  setBlock(piece, 0 * textureSize, 3 * textureSize);
               } else if (rightStub) {
                  setBlock(piece, 0 * textureSize, 1 * textureSize);
               } else if (leftStub) {
                  setBlock(piece, 0 * textureSize, 2 * textureSize);
               } else {
                  if (i == 0) {
                     setBlock(piece, 1 * textureSize, (chance(cactusFlowerChance) ? 1 : 0) * textureSize);
                  } else if (i == cactus.sizeY - 1) {
                     setBlock(piece, 1 * textureSize, 3 * textureSize);
                  } else {
                     setBlock(piece, 0 * textureSize, 0 * textureSize);
                  }
               }
               continue;
            } else if (!piece.nil) {
               int offsetX = (j == 0 ? 2 : 3) * textureSize;

               if (cactus.pieces[i - 1][j].nil && cactus.pieces[i + 1][j].nil) {
                  setBlock(piece, offsetX, 3 * textureSize);
               } else if (cactus.pieces[i - 1][j].nil) {
                  setBlock(piece, offsetX, 0 * textureSize);
               } else if (cactus.pieces[i + 1][j].nil) {
                  setBlock(piece, offsetX, 2 * textureSize);
               } else {
                  setBlock(piece, offsetX, 1 * textureSize);
               }
            }
         }
      }
      return cactus;
   } break;

   case FurnitureType::cactusSeed: {
      if (!debug && (y < 0 || x < 0 || x >= map.sizeX || y + 1 >= map.sizeY || map.blocks[y + 1][x].id != getBlockIdFromName("sand") || !map.isEmpty(x, y))) {
         return {};
      }

      Furniture cactusSeed ("cactus_seed", x, y, 1, 1, FurnitureType::cactusSeed);
      setBlock(cactusSeed.pieces[0][0], (random(0, 100) / 33) * textureSize, 0);
      return cactusSeed;
   } break;

   case FurnitureType::table: {
      if (!debug && (!map.is(x, y + 2, BlockType::solid) || !map.is(x + 2, y + 2, BlockType::solid))) {
         return {};
      }

      Furniture table ("table", x, y, 3, 2, FurnitureType::table);
      table.isWalkable = true;

      for (int yy = 0; yy < 2; ++yy) {
         for (int xx = 0; xx < 3; ++xx) {
            // Skip the block inbetween the tables legs
            if (yy == 1 && xx == 1) {
               continue;
            }

            if (!debug && !map.isNotSolid(xx + x, yy + y)) {
               return {};
            }
            setBlock(table.pieces[yy][xx], textureSize * xx, textureSize * yy);
         }
      }
      return table;
   } break;

   case FurnitureType::chair: {
      if (!debug && !map.is(x, y + 2, BlockType::solid)) {
         return {};
      }
      Furniture chair ("chair", x, y, 1, 2, FurnitureType::chair);
      for (int yy = 0; yy < 2; ++yy) {
         if (!debug && !map.isNotSolid(x, yy + y)) {
            return {};
         }
         setBlock(chair.pieces[yy][0], (playerFacingLeft ? 0 : textureSize), yy * textureSize);
      }
      return chair;
   } break;

   case FurnitureType::door: {
      if (!debug && (!map.is(x, y + 3, BlockType::solid) || !map.is(x, y - 1, BlockType::solid))) {
         return {};
      }
      Furniture door ("door", x, y, 1, 3, FurnitureType::door);
      for (int yy = 0; yy < 3; ++yy) {
         if (!debug && !map.isNotSolid(x, yy + y)) {
            return {};
         }
         setBlock(door.pieces[yy][0], (playerFacingLeft ? 0 : textureSize), yy * textureSize);
      }
      return door;
   } break;

   default: return {};
   };
}

void generateFurniture(int x, int y, Map &map, FurnitureType type, bool playerFacingleft) {
   Furniture furniture = getFurniture(x, y, map, type, playerFacingleft);
   if (furniture.type != FurnitureType::none) {
      map.addFurniture(furniture);
   }
}

// Render furniture

void Furniture::preview(const Map &map) const {
   bool valid = isValid(map);
   
   for (int y = posY; y - posY < sizeY; ++y) {
      for (int x = posX; x - posX < sizeX; ++x) {
         const FurniturePiece &piece = pieces[y - posY][x - posX];
         if (piece.nil) {
            continue;
         }
         Color color = Fade((map.isNotSolid(x, y) && valid ? WHITE : RED), previewAlpha);
         DrawTexturePro(getTexture(furnitureTextureNames[id]), {(float)piece.tx, (float)piece.ty, textureSize, textureSize}, {(float)x, (float)y, 1.f, 1.f}, {0, 0}, 0, color);
      }
   }
}

void Furniture::render(const Rectangle &cameraBounds) const {
   for (int y = posY; y <= cameraBounds.height && y - posY < sizeY; ++y) {
      for (int x = posX; x <= cameraBounds.width && x - posX < sizeX; ++x) {
         const FurniturePiece &piece = pieces[y - posY][x - posX];
         if (y < cameraBounds.y || x < cameraBounds.x || piece.nil) {
            continue;
         }
         Color color = (type == FurnitureType::door && value ? wallTint : WHITE);
         DrawTexturePro(getTexture(furnitureTextureNames[id]), {(float)piece.tx, (float)piece.ty, textureSize, textureSize}, {(float)x, (float)y, 1.f, 1.f}, {0, 0}, 0, color);
      }
   }
}

// Id functions

float getFurnitureBreakingTime(unsigned short id) {
   return furnitureBreakingTimes.at(id);
}

unsigned short getFurnitureIdFromName(const std::string &name) {
   return furnitureTextureIds.at(name);
}

std::string getFurnitureNameFromId(unsigned short id) {
   return furnitureTextureNames.at(id);
}

FurnitureType getFurnitureType(unsigned short id) {
   return furnitureTypes.at(id);
}

FurnitureTexture getFurnitureIcon(unsigned short id) {
   constexpr std::array<Vector2, furnitureCount> textureSizes {{
      {}, {textureSize, textureSize * 2}, {}, {}, {},
      {}, {}, {}, {}, {textureSize, textureSize},
      {textureSize * 3, textureSize * 2}, {textureSize, textureSize * 2}, {textureSize, textureSize * 3}
   }};
   return {getTexture(getFurnitureNameFromId(id)), textureSizes[id].x, textureSizes[id].y};
}
