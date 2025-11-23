#include <algorithm>
#include <cmath>
#include <fstream>
#include "game/gameState.hpp"
#include "game/menuState.hpp"
#include "mngr/resource.hpp"
#include "util/format.hpp"
#include "util/position.hpp"
#include "util/random.hpp"
#include "util/render.hpp"

// Constants

constexpr int mapSizeX = 2500;
constexpr int mapSizeY = 300;

constexpr float cameraFollowSpeed = 17.5f;
constexpr float maxCameraZoom = 1.f;
constexpr float minCameraZoom = 100.f;

// Constructors

GameState::GameState(const std::string& worldName)
   : worldName(worldName) {
   std::ifstream file (format("data/worlds/{}.txt", worldName));
   assert(file.is_open(), "Failed to load world 'data/worlds/{}.txt'.", worldName);

   float playerX = 0, playerY = 0;
   int mapSizeX = 0, mapSizeY = 0;
   
   file >> std::ws >> playerX >> std::ws >> playerY;
   file >> std::ws >> mapSizeX >> std::ws >> mapSizeY;
   map.setSize(mapSizeX, mapSizeY);

   for (int y = 0; y < mapSizeY; ++y) {
      for (int x = 0; x < mapSizeX; ++x) {
         int id = 0;
         file >> id;
         map.setBlock(x, y, (Block::id_t)id);
      }
   }

   // Do the same for background walls
   for (int y = 0; y < mapSizeY; ++y) {
      for (int x = 0; x < mapSizeX; ++x) {
         int id = 0;
         file >> id;
         map.setBlock(x, y, (Block::id_t)id, true);
      }
   }

   player.init({playerX, playerY});
   camera.target = player.getCenter();
   camera.offset = getScreenCenter();
   camera.rotation = 0.0f;
   camera.zoom = 50.f;
}

GameState::~GameState() {
   std::fstream file (format("data/worlds/{}.txt", worldName));
   assert(file.is_open(), "Failed to save world 'data/worlds/{}.txt'.", worldName);
   file << player.pos.x << '\n' << player.pos.y << '\n';
   file << map.sizeX << '\n' << map.sizeY << '\n';

   for (const auto& row: map.blocks) {
      for (const auto& tile: row) {
         file << (int)tile.id << ' ';
      }
      file << '\n';
   }

   for (const auto& row: map.walls) {
      for (const auto& tile: row) {
         file << (int)tile.id << ' ';
      }
      file << '\n';
   }
   file.close();
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

// Temporary way to switch, delete and place blocks. blockMap blocks must be in the same order as
// the blockIds map in objs/block.cpp.
static int index = 0;
static int size = 14;
static const char* blockMap[] {
   "grass", "dirt", "clay", "stone", "sand", "sandstone", "water", "bricks", "glass", "planks", "stone_bricks", "tiles", "obsidian", "lava"
};
static bool drawWall = false;

void GameState::updatePhysics() {
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
      if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
         map.deleteBlock(mousePos.x, mousePos.y, drawWall);
      } else if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT) and (drawWall or not CheckCollisionRecs(player.getBounds(), {(float)(int)mousePos.x, (float)(int)mousePos.y, 1.f, 1.f}))) {
         map.setBlock(mousePos.x, mousePos.y, blockMap[index], drawWall);
      } else if (IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE) and (drawWall ? map.walls : map.blocks)[mousePos.y][mousePos.x].type != Block::air) {
         index = (drawWall ? map.walls : map.blocks)[mousePos.y][mousePos.x].id - 1;
      }
   }

   physicsTimer += GetFrameTime();
   if (physicsTimer >= .1f) {
      physicsTimer -= .1f;
   } else {
      return;
   }

   
   for (int y = mapSizeY - 1; y >= 0; --y) {
      for (int x = mapSizeX - 1; x >= 0; --x) {
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

         if (block.type == Block::dirt and (map.is(x, y - 1, Block::air) or map.is(x, y - 1, Block::water))) {
            if (map[y][x].value2 == 0) {
               map[y][x].value2 = random(100, 255);
            }

            ++map[y][x].value;
            if (map[y][x].value >= map[y][x].value2) {
               map[y][x].value = 0;
               map.setBlock(x, y, "grass");
            }
         }

         if (block.type == Block::grass and not map.is(x, y - 1, Block::air) and not map.is(x, y - 1, Block::water)) {
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
}

// Other functions

void GameState::render() {
   drawRect(BLUE);
   BeginMode2D(camera);
   map.render(camera);
   player.render();
   EndMode2D();

   drawTexture(ResourceManager::get().getTexture(blockMap[index]), {GetScreenWidth() - 75.f, GetScreenHeight() - 75.f}, {50.f, 50.f}, 0.f, (drawWall ? Color{120, 120, 120, 255} : WHITE));
}

State* GameState::change() {
   return new MenuState();
}
