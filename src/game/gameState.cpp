#include "game/gameState.hpp"
#include "game/menuState.hpp"
#include "mngr/resource.hpp"
#include "mngr/sound.hpp"
#include "util/config.hpp"
#include "util/fileio.hpp"
#include "util/input.hpp"
#include "util/math.hpp"
#include "util/parallax.hpp"
#include "util/position.hpp"
#include "util/random.hpp"
#include "util/render.hpp"
#include <algorithm>
#include <cmath>
#include <raymath.h>

// Constructors

GameState::GameState(const std::string &worldName)
   : inventory(map, player, droppedItems), backgroundTexture(getRandomBackground()), foregroundTexture(getRandomForeground()), worldName(worldName) {
   resetBackground();
   loadWorldData(worldName, player, camera.zoom, map, inventory);

   camera.zoom = clamp(camera.zoom, minCameraZoom, maxCameraZoom);
   camera.target = player.getCenter();
   camera.offset = getScreenCenter();
   camera.rotation = 0.0f;
   calculateCameraBounds();

   // Init UI
   continueButton.rectangle = {GetScreenWidth() / 2.f, GetScreenHeight() / 2.f, buttonWidth, buttonHeight};
   continueButton.text = "Continue";
   menuButton.rectangle = {continueButton.rectangle.x, continueButton.rectangle.y + buttonPaddingY, buttonWidth, buttonHeight};
   menuButton.text = "Save & Quit";
   pauseButton.rectangle = {GetScreenWidth() - buttonWidth / 2.f, GetScreenHeight() - buttonHeight / 2.f, buttonWidth, buttonHeight};
   pauseButton.text = "Pause";
   continueButton.texture = menuButton.texture = &getTexture("button");
}

GameState::~GameState() {
   inventory.discardItem();
   saveWorldData(worldName, player.position.x, player.position.y, camera.zoom, map, &inventory);
}

// Update functions

void GameState::update() {
   updatePauseScreen();
   updateControls();
   updatePhysics();
}

void GameState::updatePauseScreen() {
   pauseButton.update();
   if (pauseButton.clicked) {
      paused = !paused;
   }
   
   if (IsKeyReleased(pauseKey)) {
      playSound("click");
      paused = !paused;
   }

   if (!paused) {
      return;
   }
   
   continueButton.update();
   menuButton.update();

   if (continueButton.clicked) {
      paused = false;
   }
   if (menuButton.clicked) {
      fadingOut = true;
   }
}

void GameState::updateControls() {
   camera.target = lerp(camera.target, player.getCenter(), cameraFollowSpeed);
   calculateCameraBounds();

   if (paused) {
      return;
   }

   float wheel = IsKeyReleased(zoomInKey) - IsKeyReleased(zoomOutKey);
   if (wheel != 0.f) {
      camera.zoom = clamp(std::exp(std::log(camera.zoom) + wheel * 0.2f), minCameraZoom, maxCameraZoom);
   }

   player.updatePlayer(map);
   inventory.update();
}

/************************************/
// Temporary way to switch, delete and place blocks. blockMap blocks must be in the same order as
// the blockIds map in objs/block.cpp. Everything between these multi-comments is temporary.
static int index = 0;
static int size = 21;
static const char *blockMap[] {
   "grass", "dirt", "clay", "stone", "sand", "sandstone", "water", "bricks", "glass", "planks", "stone_bricks", "tiles", "obsidian",
   "lava", "platform", "snow", "ice", "mud", "jungle_grass",
   "sapling", "cactus_seed"
};
static bool drawWall = false;
static bool canDraw = false;
static Furniture obj;
inline Furniture::Type getFurnitureType() { return (index == 19 ? Furniture::sapling : (index == 20 ? Furniture::cactus_seed : Furniture::none)); }
/************************************/

void GameState::updatePhysics() {
   if (paused) {
      return;
   }
   
   /************************************/
   Vector2 mousePos = GetScreenToWorld2D(GetMousePosition(), camera);

   if (IsKeyPressed(tempSwitchForward)) {
      index = (index + 1) % size;
   }

   if (IsKeyPressed(tempSwitchBackward)) {
      index = (index == 0 ? size - 1 : index - 1);
   }

   if (IsKeyPressed(tempSwitchWall)) {
      drawWall =! drawWall;
   }

   if (map.isPositionValid(mousePos.x, mousePos.y)) {
      Furniture::Type ftype = getFurnitureType();
      canDraw = (drawWall || ftype != Furniture::none || !CheckCollisionRecs(player.getBounds(), {(float)(int)mousePos.x, (float)(int)mousePos.y, 1.f, 1.f}));

      if (isMouseDownOutsideUI(MOUSE_BUTTON_LEFT)) {
         map.deleteBlock(mousePos.x, mousePos.y, drawWall);
      } else if (isMouseDownOutsideUI(MOUSE_BUTTON_RIGHT) && canDraw && !map.blocks[mousePos.y][mousePos.x].furniture) {
         if (ftype != Furniture::none) {
            Furniture::generate(mousePos.x, mousePos.y, map, ftype);
         } else {
            map.setBlock(mousePos.x, mousePos.y, blockMap[index], drawWall);
         }
      } else if (isMousePressedOutsideUI(MOUSE_BUTTON_MIDDLE) && (drawWall ? map.walls : map.blocks)[mousePos.y][mousePos.x].type != Block::air) {
         index = (drawWall ? map.walls : map.blocks)[mousePos.y][mousePos.x].id - 1;
      }
   }
   /************************************/

   // Update every frame (DT-dependant)
   if (!droppedItems.empty()) {
      for (auto &droppedItem: droppedItems) {
         droppedItem.update();
      }

      droppedItems.erase(std::remove_if(droppedItems.begin(), droppedItems.end(), [](DroppedItem &i) -> bool {
         return i.lifetime >= droppedItemLifetime;
      }), droppedItems.end());
   }

   // Update in specific intervals (DT-independant)
   physicsTimer += GetFrameTime();
   if (physicsTimer >= physicsUpdateTime) {
      physicsTimer -= physicsUpdateTime;
   } else {
      return;
   }

   for (int y = cameraBounds.height; y >= cameraBounds.y; --y) {
      for (int x = cameraBounds.width; x >= cameraBounds.x; --x) {
         Block &block = map[y][x];

         if (block.type == Block::water || block.type == Block::lava) {
            if (block.type == Block::lava) {
               // Turn into obsidian if water is in a 1 tile radius
               for (int yy = y - 1; yy <= y + 1 && yy >= 0 && yy < map.sizeY; ++yy) {
                  for (int xx = x - 1; xx <= x + 1 && xx >= 0 && xx < map.sizeX; ++xx) {
                     if (map.isu(xx, yy, Block::water)) {
                        if (map.blocks[y][x].furniture) {
                           map.deleteBlock(x, y);
                        } else {
                           map.setBlock(x, y, "obsidian");
                        }
                        map.deleteBlock(xx, yy);
                     }
                  }
               }

               if (block.type != Block::lava) {
                  continue;
               }

               // Update lava slower than water
               ++map[y][x].value;
               if (map[y][x].value >= lavaUpdateSpeed) {
                  map[y][x].value = 0;
               } else {
                  continue;
               }
            }

            if (map.is(x, y + 1, Block::air)) {
               map.moveBlock(x, y, x, y + 1);
            } else if (map.is(x - 1, y, Block::air) && map.is(x + 1, y, Block::air)) {
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

         // Update sand
         if (block.type == Block::sand) {
            if (map.empty(x, y + 1) || map.is(x, y + 1, Block::water)) {
               map.moveBlock(x, y, x, y + 1);
            } else if ((map.empty(x - 1, y + 1) || map.is(x - 1, y + 1, Block::water)) && (map.empty(x + 1, y + 1) || map.is(x + 1, y + 1, Block::water))) {
               if (chance(50)) {
                  goto moveSandRight;
               } else {
                  goto moveSandLeft;
               }
            } else if (map.empty(x - 1, y + 1) || map.is(x - 1, y + 1, Block::water)) {
            moveSandLeft:
               map.moveBlock(x, y, x - 1, y + 1);
            } else if (map.empty(x + 1, y + 1) || map.is(x + 1, y + 1, Block::water)) {
            moveSandRight:
               map.moveBlock(x, y, x + 1, y + 1);
            }

            if (map[y][x].value2 == 0) {
               map[y][x].value2 = random(cactusGrowSpeedMin, cactusGrowSpeedMax);
            }

            map[y][x].value += chance(1);
            if (map[y][x].value >= map[y][x].value2 && map[y][x].value2 != 0 && map.empty(x, y - 1) && map.empty(x, y - 2)) {
               map[y][x].value = map[y][x].value2 = 0;
               Furniture::generate(x, y - 1, map, Furniture::cactus_seed);
            }
         }

         // Update grass and dirt
         if (block.type == Block::dirt && (map.is(x, y - 1, Block::air) || map.is(x, y - 1, Block::water) || map.is(x, y - 1, Block::platform))) {
            if (map[y][x].value2 == 0) {
               map[y][x].value2 = random(grassGrowSpeedMin, grassGrowSpeedMax);
            }

            ++map[y][x].value;
            if (map[y][x].value >= map[y][x].value2) {
               map[y][x].value = map[y][x].value2 = 0;
               map.setBlock(x, y, (block.id == Block::getId("dirt") ? "grass" : "jungle_grass"));
            }
         }

         if (block.type == Block::grass && !map.is(x, y - 1, Block::air) && !map.is(x, y - 1, Block::water) && !map.is(x, y - 1, Block::platform)) {
            if (map[y][x].value2 == 0) {
               map[y][x].value2 = random(grassGrowSpeedMin, grassGrowSpeedMax);
            }

            ++map[y][x].value;
            if (map[y][x].value >= map[y][x].value2) {
               map[y][x].value = map[y][x].value2 = 0;
               map.setBlock(x, y, (block.id == Block::getId("grass") ? "dirt" : "mud"));
            }
         }
      }
   }
   
   for (Furniture &obj: map.furniture) {
      obj.update(map);
   }

   // Remove deleted furniture
   map.furniture.erase(std::remove_if(map.furniture.begin(), map.furniture.end(), [](Furniture &f) -> bool {
      return f.deleted;
   }), map.furniture.end());
}

// Other functions

void GameState::render() {
   // Draw parallax background
   float delta = (paused ? 0 : player.delta.x / GetFrameTime() / 60.0f); // To avoid delta time clash
   drawBackground(foregroundTexture, backgroundTexture, delta * parallaxBgSpeed, delta * parallaxFgSpeed, (paused ? 0 : gameSunSpeed));

   BeginMode2D(camera);
   map.render(cameraBounds);

   /************************************/
   // Scary method of rendering furniture and block preview correctly
   Vector2 mousePos = GetScreenToWorld2D(GetMousePosition(), camera);
   if (canDraw && map.isPositionValid(mousePos.x, mousePos.y)) {
      Furniture::Type ftype = getFurnitureType();
      if (ftype != Furniture::none) {
         static Block::Type oldBelow = Block::air;
         Block::Type below = (map.isPositionValid(mousePos.x, mousePos.y + obj.sizeY) ? map.blocks[mousePos.y + obj.sizeY][mousePos.x].type : Block::air);
         
         if (ftype != obj.type || oldBelow != below) {
            obj = Furniture::get(mousePos.x, mousePos.y, map, ftype, true);
         }
         oldBelow = below;
         obj.posX = mousePos.x;
         obj.posY = mousePos.y;
         obj.preview(map);
      } else {
         drawTextureBlock(getTexture(blockMap[index]), {(float)(int)mousePos.x, (float)(int)mousePos.y, 1.f, 1.f}, Fade((drawWall ? wallTint : (map.blocks[mousePos.y][mousePos.x].furniture ? RED : WHITE)), previewAlpha));
      }
   }
   /************************************/

   if (!droppedItems.empty()) {
      updateTimer += GetFrameTime();
      float offset = std::sin(updateTimer * droppedItemFloatSpeed) * droppedItemFloatHeight;

      for (auto &droppedItem : droppedItems) {
         if (droppedItem.tileX >= cameraBounds.x && droppedItem.tileX <= cameraBounds.width
          && droppedItem.tileY >= cameraBounds.y && droppedItem.tileY <= cameraBounds.height) {
            droppedItem.render(offset);
         }
      }
   }

   player.render();
   EndMode2D();

   // Draw the UI
   inventory.render();

   if (paused) {
      continueButton.render();
      menuButton.render();
   }
   pauseButton.render();
}

State* GameState::change() {
   return new MenuState();
}

void GameState::calculateCameraBounds() {
   cameraBounds = getCameraBounds(camera);

   cameraBounds.x = max(0, int(cameraBounds.x));
   cameraBounds.y = max(0, int(cameraBounds.y));
   cameraBounds.width = min(map.sizeX - 1, int(cameraBounds.x + cameraBounds.width) + 1);
   cameraBounds.height = min(map.sizeY - 1, int(cameraBounds.y + cameraBounds.height) + 1);
}
