#include "game/gameState.hpp"
#include "game/menuState.hpp"
#include "mngr/resource.hpp"
#include "util/fileio.hpp"
#include "util/position.hpp"
#include "util/random.hpp"
#include "util/render.hpp"
#include <algorithm>
#include <cmath>

// Constants

constexpr float cameraFollowSpeed = 17.5f;
constexpr float maxCameraZoom = 1.f;
constexpr float minCameraZoom = 100.f;

// Constructors

GameState::GameState(const std::string& worldName)
   : worldName(worldName) {
   loadWorldData(worldName, player, camera.zoom, map);
   camera.target = player.getCenter();
   camera.offset = getScreenCenter();
   camera.rotation = 0.0f;
}

GameState::~GameState() {
   saveWorldData(worldName, player.pos.x, player.pos.y, camera.zoom, map);
}

// Update functions

void GameState::update() {
   updateControls();
   updatePhysics();
}

void GameState::updateControls() {
   player.updatePlayer(map);
   camera.target = lerp(camera.target, player.getCenter(), 25.f * GetFrameTime());

   float wheel = GetMouseWheelMove();
   if (wheel != 0.f) {
      camera.zoom = std::clamp(std::exp(std::log(camera.zoom) + wheel * 0.2f), maxCameraZoom, minCameraZoom);
   }

   if (IsKeyReleased(KEY_ESCAPE)) {
      fadingOut = true;
   }
}

/************************************/
// Temporary way to switch, delete and place blocks. blockMap blocks must be in the same order as
// the blockIds map in objs/block.cpp.
static int index = 0;
static int size = 18;
static const char* blockMap[] {
   "grass", "dirt", "clay", "stone", "sand", "sandstone", "water", "bricks", "glass", "planks", "stone_bricks", "tiles", "obsidian",
   "lava", "platform", "snow", "ice",
   "sapling"
};
static bool drawWall = false;
static bool canDraw = false;
static Furniture obj;
inline Furniture::Type getFurnitureType() { return (index == 17 ? Furniture::sapling : Furniture::none); }
/************************************/

void GameState::updatePhysics() {
   /************************************/
   auto mousePos = GetScreenToWorld2D(GetMousePosition(), camera);

   if (IsKeyPressed(KEY_E)) {
      index = (index + 1) % size;
   }

   if (IsKeyPressed(KEY_Q)) {
      index = (index == 0 ? size - 1 : index - 1);
   }

   if (IsKeyPressed(KEY_R)) {
      drawWall =! drawWall;
   }

   if (map.isPositionValid(mousePos.x, mousePos.y)) {
      auto ftype = getFurnitureType();
      canDraw = (drawWall or ftype != Furniture::none or not CheckCollisionRecs(player.getBounds(), {(float)(int)mousePos.x, (float)(int)mousePos.y, 1.f, 1.f}));

      if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
         map.deleteBlock(mousePos.x, mousePos.y, drawWall);
      } else if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT) and canDraw and not map.blocks[mousePos.y][mousePos.x].furniture) {
         if (ftype != Furniture::none) {
            Furniture::generate(mousePos.x, mousePos.y, map, ftype);
         } else {
            map.setBlock(mousePos.x, mousePos.y, blockMap[index], drawWall);
         }
      } else if (IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE) and (drawWall ? map.walls : map.blocks)[mousePos.y][mousePos.x].type != Block::air) {
         index = (drawWall ? map.walls : map.blocks)[mousePos.y][mousePos.x].id - 1;
      }
   }
   /************************************/

   physicsTimer += GetFrameTime();
   if (physicsTimer >= .1f) {
      physicsTimer -= .1f;
   } else {
      return;
   }
   
   for (int y = map.sizeY - 1; y >= 0; --y) {
      for (int x = map.sizeX - 1; x >= 0; --x) {
         auto& block = map[y][x];

         if (block.type == Block::water or block.type == Block::lava) {
            if (block.type == Block::lava) {
               // Turn into obsidian if water is found adjacently
               if (map.is(x, y + 1, Block::water)) {
                  map.setBlock(x, y, "obsidian");
                  map.deleteBlock(x, y + 1);
               } else if (map.is(x, y - 1, Block::water)) {
                  map.setBlock(x, y, "obsidian");
                  map.deleteBlock(x, y - 1);
               } else if (map.is(x + 1, y, Block::water)) {
                  map.setBlock(x, y, "obsidian");
                  map.deleteBlock(x + 1, y);
               } else if (map.is(x - 1, y, Block::water)) {
                  map.setBlock(x, y, "obsidian");
                  map.deleteBlock(x - 1, y);
               }

               // Update lava slower than water
               ++map[y][x].value;
               if (map[y][x].value >= 6) {
                  map[y][x].value = 0;
               } else {
                  continue;
               }
            }

            if (map.is(x, y + 1, Block::air)) {
               map.moveBlock(x, y, x, y + 1);
            } else if (map.is(x - 1, y, Block::air) and map.is(x + 1, y, Block::air)) {
               if (chance(50)) {
                  goto moveWaterLeft;
               } else {
                  goto moveWaterRight;
               }
            } else if (map.is(x - 1, y, Block::air)) {
            moveWaterLeft:
               map.moveBlock(x, y, x - 1, y);
               --x; // Prevent the water tile from updating twice
            } else if (map.is(x + 1, y, Block::air)) {
            moveWaterRight:
               map.moveBlock(x, y, x + 1, y);
            }
         }

         if (block.type == Block::sand and (map.is(x, y + 1, Block::air) or map.is(x, y + 1, Block::water))) {
            map.moveBlock(x, y, x, y + 1);
         }

         if (block.type == Block::dirt and (map.is(x, y - 1, Block::air) or map.is(x, y - 1, Block::water) or map.is(x, y - 1, Block::platform))) {
            if (map[y][x].value2 == 0) {
               map[y][x].value2 = random(100, 255);
            }

            ++map[y][x].value;
            if (map[y][x].value >= map[y][x].value2) {
               map[y][x].value = 0;
               map.setBlock(x, y, "grass");
            }
         }

         if (block.type == Block::grass and not map.is(x, y - 1, Block::air) and not map.is(x, y - 1, Block::water) and not map.is(x, y - 1, Block::platform)) {
            if (map[y][x].value2 == 0) {
               map[y][x].value2 = random(100, 255);
            }

            ++map[y][x].value;
            if (map[y][x].value >= map[y][x].value2) {
               map[y][x].value = 0;
               map.setBlock(x, y, "dirt");
            }
         }
      }
   }
   
   for (auto& obj: map.furniture) {
      obj.update(map);
   }

   // Remove deleted furniture
   map.furniture.erase(std::remove_if(map.furniture.begin(), map.furniture.end(), [this](Furniture& f) -> bool {
      return f.deleted;
   }), map.furniture.end());
}

// Other functions

void GameState::render() {
   drawRect(BLUE);
   BeginMode2D(camera);
   map.render(camera);

   /************************************/
   auto mousePos = GetScreenToWorld2D(GetMousePosition(), camera);
   if (canDraw and camera.zoom > 12.5f and map.isPositionValid(mousePos.x, mousePos.y)) {
      auto ftype = getFurnitureType();
      if (ftype != Furniture::none) {
         static auto oldBelow = Block::air;
         auto below = (map.isPositionValid(mousePos.x, mousePos.y + obj.sizeY) ? map.blocks[mousePos.y + obj.sizeY][mousePos.x].type : Block::air);
         
         if (ftype != obj.type or oldBelow != below) {
            obj = Furniture::get(mousePos.x, mousePos.y, map, ftype, true);
         }
         oldBelow = below;
         obj.posX = mousePos.x;
         obj.posY = mousePos.y;
         obj.preview(map);
      } else {
         drawTextureBlock(getTexture(blockMap[index]), {(float)(int)mousePos.x, (float)(int)mousePos.y, 1.f, 1.f}, Fade((drawWall ? Color{120, 120, 120, 255} : WHITE), .75f));
      }
   }
   /************************************/

   player.render();
   EndMode2D();
}

State* GameState::change() {
   return new MenuState();
}
