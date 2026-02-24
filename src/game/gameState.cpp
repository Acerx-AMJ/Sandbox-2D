#include "game/gameState.hpp"
#include "game/menuState.hpp"
#include "mngr/particle.hpp"
#include "mngr/resource.hpp"
#include "mngr/input.hpp"
#include "mngr/sound.hpp"
#include "util/fileio.hpp"
#include "util/format.hpp"
#include "util/math.hpp"
#include "util/parallax.hpp"
#include "util/position.hpp"
#include "util/random.hpp"
#include "util/render.hpp"
#include <raymath.h>
#include <algorithm>
#include <cmath>

// Constants

constexpr float cameraFollowSpeed = 0.416f;
constexpr float minCameraZoom     = 12.5f;
constexpr float maxCameraZoom     = 200.0f;

constexpr int physicsTicks      = 8;
constexpr int lavaUpdateSpeed   = 2; // Lava updates 2x slower than water
constexpr int honeyUpdateSpeed  = 4; // Honey updates 2x slower than lava
constexpr int grassGrowSpeedMin = 100;
constexpr int grassGrowSpeedMax = 255;

constexpr float maxPickupRange = 4.0f; // Squared
constexpr float maxToolRange = 100.0f; // Squared
constexpr float timeToRespawn = 10.0f;

// Constructors

GameState::GameState(const std::string &worldName)
: backgroundTexture(getRandomBackground()), foregroundTexture(getRandomForeground()), inventory(map, player, droppedItems), worldName(worldName) {
   const Vector2 center = getScreenCenter();
   
   // Init world and camera
   loadWorldData(worldName, playerSpawnPosition, player, camera.zoom, map, inventory, droppedItems);

   camera.zoom = clamp(camera.zoom, minCameraZoom, maxCameraZoom);
   camera.target = player.getCenter();
   camera.offset = center;
   camera.rotation = 0.0f;
   calculateCameraBounds();

   inventory.items[0][0] = Item{ItemType::equipment, 5, 1};

   // Init UI
   continueButton.rectangle = {center.x, center.y, buttonWidth, buttonHeight};
   continueButton.text = "Continue";
   menuButton.rectangle = {continueButton.rectangle.x, continueButton.rectangle.y + buttonPaddingY, buttonWidth, buttonHeight};
   menuButton.text = "Save & Quit";
   pauseButton.rectangle = {GetScreenWidth() - buttonWidth / 2.f + 10.0f, GetScreenHeight() - buttonHeight / 2.f, buttonWidth, buttonHeight};
   pauseButton.text = "Pause";
   continueButton.texture = menuButton.texture = &getTexture("button");
}

GameState::~GameState() {
   inventory.discardItem();
   saveWorldData(worldName, playerSpawnPosition, player.position, player.breath, player.hearts, player.maxHearts, camera.zoom, map, &inventory, &droppedItems);
   resetBackground();
}

// Update

void GameState::update() {
   if (phase != Phase::died) {
      pauseButton.update(dt);
      if (pauseButton.clicked || handleKeyPressWithSound(KEY_ESCAPE)) {
         phase = (phase == Phase::paused ? Phase::playing : Phase::paused);
         calculateCameraBounds();
      }
   }

   switch (phase) {
   case Phase::playing: updatePlaying(); break;
   case Phase::paused:  updatePausing(); break;
   case Phase::died:    updateDying();   break;
   }
}

void GameState::fixedUpdate() {
   camera.target = lerp(camera.target, player.getCenter(), cameraFollowSpeed);
   if (phase == Phase::paused) {
      calculateCameraBounds(); // Make sure the camera does not go out of bounds
      return;
   }

   if (player.hearts == 0) {
      Phase lastPhase = phase;
      phase = Phase::died;

      if (lastPhase != phase) {
         spawnDeathParticles(player.getCenter());
      }
      calculateCameraBounds(); // Make sure the camera does not go out of bounds
   } else {
      player.updatePlayer(map);
   }

   for (DamageIndicator &indicator: map.damageIndicators) {
      indicator.velocity.y += 0.25f * fixedUpdateDT;
      indicator.velocity.x *= 0.9f; // Drag

      indicator.position.x += indicator.velocity.x;
      indicator.position.y += indicator.velocity.y;
      indicator.lifetime += fixedUpdateDT;
   }

   map.damageIndicators.erase(std::remove_if(map.damageIndicators.begin(), map.damageIndicators.end(), [](DamageIndicator &i) -> bool {
      return i.lifetime >= damageIndicatorLifetime || i.damage <= 0.0f;
   }), map.damageIndicators.end());   

   // Update physics
   physicsCounter = (physicsCounter + 1) % physicsTicks;
   if (physicsCounter != 0) {
      return;
   }

   lavaCounter = (lavaCounter + 1) % lavaUpdateSpeed;
   honeyCounter = (honeyCounter + 1) % honeyUpdateSpeed;

   Rectangle physicsBounds = cameraBounds;
   Vector2 halfSize = {(cameraBounds.width - cameraBounds.x) / 2.0f, (cameraBounds.height - cameraBounds.y) / 2.0f};
   physicsBounds.x = max<int>(0, cameraBounds.x - halfSize.x);
   physicsBounds.y = max<int>(0, cameraBounds.y - halfSize.y);
   physicsBounds.width = min<int>(map.sizeX - 1, cameraBounds.width + halfSize.x);
   physicsBounds.height = min<int>(map.sizeY - 1, cameraBounds.height + halfSize.y);

   // Loop backwards to avoid updating most of the moving blocks twice
   for (int y = physicsBounds.height; y >= physicsBounds.y; --y) {
      for (int x = physicsBounds.width; x >= physicsBounds.x; --x) {
         if (map.isLiquidAtAll(x, y)) {
            if (map.isLiquidOfType(x, y, LiquidType::water)) {
               updateWaterPhysics(x, y);
            } else if (lavaCounter == 0 && map.isLiquidOfType(x, y, LiquidType::lava)) {
               updateLavaPhysics(x, y);
            } else if (honeyCounter == 0 && map.isLiquidOfType(x, y, LiquidType::honey)) {
               updateHoneyPhysics(x, y);
            }
         }

         BlockType type = map.blocks[y][x].type;
         if (type & BlockType::sand) {
            updateSandPhysics(x, y);
         } else if (type & BlockType::grass) {
            updateGrassPhysics(x, y);
         } else if (type & BlockType::dirt) {
            updateDirtPhysics(x, y);
         } else if (type & BlockType::torch) {
            updateTorchPhysics(x, y);
         }
      }
   }
}

// Update playing

void GameState::updatePlaying() {
   const float zoomFactor = isKeyPressed(KEY_EQUAL) - isKeyPressed(KEY_MINUS);
   if (zoomFactor != 0.f) {
      camera.zoom = /* std:: */clamp<float>(std::exp(std::log(camera.zoom) + zoomFactor * 0.2f), minCameraZoom, maxCameraZoom);
   }
   inventory.update();
   calculateCameraBounds();

   if (phase != Phase::playing) {
      return;
   }

   // Update furniture
   const Vector2 translatedMousePos = GetScreenToWorld2D(GetMousePosition(), camera);
   for (Furniture &obj: map.furniture) {
      obj.update(map, player, translatedMousePos);
   }

   map.furniture.erase(std::remove_if(map.furniture.begin(), map.furniture.end(), [](Furniture &f) -> bool {
      return f.deleted;
   }), map.furniture.end());

   // Place and destroy blocks
   Vector2 mousePos = GetScreenToWorld2D(GetMousePosition(), camera);
   Vector2 playerCenter = player.getCenter();
   int mouseX = mousePos.x;
   int mouseY = mousePos.y;

   if (map.isPositionValid(mouseX, mouseY) && Vector2DistanceSqr(mousePos, playerCenter) <= maxToolRange) {
      canDrawPreview = (inventory.canPlaceBlock() && (inventory.getSelected().isFurniture || !CheckCollisionRecs(player.getBounds(), {(float)mouseX, (float)mouseY, 1, 1})));
      player.breakingBlock = (isMouseDownOutsideUI(MOUSE_BUTTON_LEFT) && (!(map.blocks[mouseY][mouseX].type & BlockType::empty) || !(map.walls[mouseY][mouseX].type & BlockType::empty) || (map.blocks[mouseY][mouseX].type & BlockType::furniture)));

      if (isMouseDownOutsideUI(MOUSE_BUTTON_RIGHT) && inventory.canPlaceBlock()) {
         inventory.placeBlock(mouseX, mouseY, player.flipX);
         canDrawPreview = canDrawPreview && inventory.canPlaceBlock(); // To avoid attempting to draw air on placing last block
      } else if (isMousePressedOutsideUI(MOUSE_BUTTON_MIDDLE)) {
         inventory.selectItem(mouseX, mouseY);
      } else if (player.breakingBlock) {
         bool isWall = (map.blocks[mouseY][mouseX].type & BlockType::empty);
         bool isFurniture = (map.blocks[mouseY][mouseX].type & BlockType::furniture);
         Block &block = (isWall ? map.walls : map.blocks)[mouseY][mouseX];

         if (mouseX != player.lastBreakingX || mouseY != player.lastBreakingY || isWall != player.breakingWall || isFurniture != player.breakingFurniture) {
            player.breakTime = 0;
         }

         player.breakTime += realDt * inventory.getBlockBreakingMultiplier();
         player.breakingWall = isWall;
         player.breakingFurniture = isFurniture;
         player.lastBreakingX = mouseX;
         player.lastBreakingY = mouseY;

         if (player.breakTime >= (player.breakingFurniture ? getFurnitureBreakingTime(map.getFurnitureAtPosition(mouseX, mouseY).id) : getBlockBreakingTime(block.id))) {
            if (player.breakingFurniture) {
               map.getFurnitureAtPosition(mouseX, mouseY).destroy(map, inventory, mouseX, mouseY, inventory.getBlockBreakingLevel());
            } else {
               if (getBlockBreakingLevel(block.id) <= inventory.getBlockBreakingLevel()) {
                  Item item = getBlockDropId(block.id, player.breakingWall);
                  inventory.tryToPlaceItemOrDropAtCoordinates(item, mouseX, mouseY);
               }
               map.deleteBlockWithoutDeletingLiquids(mouseX, mouseY, player.breakingWall);
            }
            player.breakTime = 0;
         }
      }
   } else {
      canDrawPreview = false;
      player.breakingBlock = false;
   }

   // Update dropped items
   for (auto &droppedItem: droppedItems) {
      droppedItem.update(cameraBounds, dt);

      if (!droppedItem.inBounds || Vector2DistanceSqr(playerCenter, {(float)droppedItem.tileX, (float)droppedItem.tileY}) > maxPickupRange) {
         continue;
      }
      Item item {droppedItem.type, droppedItem.id, droppedItem.count, droppedItem.isFurniture, droppedItem.isWall, false};
      const int count = droppedItem.count;

      droppedItem.count = (inventory.placeItem(item) ? 0 : item.count);
      if (count != droppedItem.count) {
         playSound("pickup");
      }
   }

   droppedItems.erase(std::remove_if(droppedItems.begin(), droppedItems.end(), [](DroppedItem &i) -> bool {
      return i.flagForDeletion || i.count <= 0;
   }), droppedItems.end());
}

// Update pause screen

void GameState::updatePausing() {
   continueButton.update(dt);
   menuButton.update(dt);

   if (continueButton.clicked) {
      phase = Phase::playing;
   }

   if (menuButton.clicked) {
      fadingOut = true;
   }
}

// Update death screen

void GameState::updateDying() {
   deathTimer += realDt;
   if (deathTimer >= timeToRespawn) {
      player.previousPosition = player.position = playerSpawnPosition;
      player.hearts = player.lastHearts = player.displayHearts = player.maxHearts;
      player.displayBreath = player.breath = maxBreath;
      player.velocity = {0, 0};
      player.timeSinceLastDamage = player.immunityFrame = 1.2f; // Give the player a second of immunity
      player.onGround = player.shouldBounce = player.feetCollision = player.torsoCollision = false;
      player.fallTimer = player.walkTimer = player.jumpTimer = player.coyoteTimer = player.foxTimer = 0.0f;

      phase = Phase::playing;
      deathTimer = 0.0f;
   }
}

// Block physic update functions

static constexpr unsigned char calculateFlowDown(unsigned char flow1, unsigned char flow2) {
   unsigned char availableSpace = maxLiquidLayers - flow2;
   return min(availableSpace, flow1);
}

static void applyFlowDown(unsigned char &flow1, unsigned char &flow2) {
   unsigned char flowDown = calculateFlowDown(flow1, flow2);
   flow1 -= flowDown;
   flow2 += flowDown;
}

static void applyHalfFlowDown(unsigned char &flow1, unsigned char &flow2) {
   unsigned char flowDown = calculateFlowDown(flow1, flow2);
   unsigned char halfFlowDown = (flowDown == 1 ? 1 : flowDown / 2);
   flow1 -= halfFlowDown;
   flow2 += halfFlowDown;
}

bool GameState::handleLiquidToBlock(int x, int y, LiquidType type, unsigned short blockId) {
   // What even is C++ syntax?
   for (const Vector2 &offset: {Vector2{1, 0}, Vector2{0, 1}, Vector2{-1, 0}, Vector2{0, -1}}) {
      if (!map.isLiquidAtAll(x + offset.x, y + offset.y) || !map.isLiquidOfType(x + offset.x, y + offset.y, type)) {
         continue;
      }

      if (map.getLiquidHeight(x + offset.x, y + offset.y) < liquidToBlockThreshold || !(map.blocks[y + offset.y][x + offset.x].type & BlockType::empty)) {
         map.liquidTypes[y + offset.y][x + offset.x] = LiquidType::none;
         map.liquidsHeights[y + offset.y][x + offset.x] = 0;
         continue;
      }

      if (map.getLiquidHeight(x, y) >= liquidToBlockThreshold) {
         map.setBlock(x + offset.x, y + offset.y, blockId);
      }
      map.liquidTypes[y][x] = LiquidType::none;
      map.liquidsHeights[y][x] = 0;
   }
   return map.isLiquidAtAll(x, y);
}

void GameState::updateFluid(int x, int y) {
   unsigned char height = map.getLiquidHeight(x, y);
   LiquidType type = map.liquidTypes[y][x];

   // Delete the liquid if its height is zero
   if (height == 0) {
      map.liquidsHeights[y][x] = 0;
      map.liquidTypes[y][x] = LiquidType::none;
      return;
   }

   // Handle liquid going down
   if (map.is(x, y + 1, BlockType::flowable) && !map.isLiquidAtAll(x, y + 1)) {
      std::swap(map.liquidTypes[y][x], map.liquidTypes[y + 1][x]);
      std::swap(map.liquidsHeights[y][x], map.liquidsHeights[y + 1][x]);
      return;
   } else if (map.isLiquidAtAll(x, y + 1) && map.isLiquidOfType(x, y + 1, type) && map.getLiquidHeight(x, y + 1) < maxLiquidLayers) {
      applyFlowDown(map.liquidsHeights[y][x], map.liquidsHeights[y + 1][x]);
   }

   // Handle liquid going left
   if ((map.is(x - 1, y, BlockType::flowable) && !map.isLiquidAtAll(x - 1, y))
    || (map.isLiquidAtAll(x - 1, y) && map.isLiquidOfType(x - 1, y, type) && map.getLiquidHeight(x - 1, y) < height && map.getLiquidHeight(x - 1, y) < maxLiquidLayers)) {
      map.liquidTypes[y][x - 1] = type;
      applyHalfFlowDown(map.liquidsHeights[y][x], map.liquidsHeights[y][x - 1]);
   }

   // Handle liquid going right
   if ((map.is(x + 1, y, BlockType::flowable) && !map.isLiquidAtAll(x + 1, y))
    || (map.isLiquidAtAll(x + 1, y) && map.isLiquidOfType(x + 1, y, type) && map.getLiquidHeight(x + 1, y) < height && map.getLiquidHeight(x + 1, y) < maxLiquidLayers)) {
      map.liquidTypes[y][x + 1] = type;
      applyHalfFlowDown(map.liquidsHeights[y][x], map.liquidsHeights[y][x + 1]);
   }
}

void GameState::updateWaterPhysics(int x, int y) {
   // Since lava updates 2x slower and in batch, make water turn
   // nearby tiles into obsidian
   if (handleLiquidToBlock(x, y, LiquidType::lava, getBlockIdFromName("obsidian"))) {
      if (handleLiquidToBlock(x, y, LiquidType::honey, getBlockIdFromName("honey_block"))) {
         updateFluid(x, y);
      }
   }
}

void GameState::updateLavaPhysics(int x, int y) {
   // Same with honey. It on purpose updates 2x slower than lava,
   // to make preventing liquid clashes easier
   if (handleLiquidToBlock(x, y, LiquidType::honey, getBlockIdFromName("crispy_honey_block"))) {
      updateFluid(x, y);
   }
}

void GameState::updateHoneyPhysics(int x, int y) {
   updateFluid(x, y);
}

void GameState::updateSandPhysics(int x, int y) {
   if (map.isNotSolid(x, y + 1)) {
      map.moveBlock(x, y, x, y + 1);
      return;
   }

   bool leftEmpty  = map.isNotSolid(x - 1, y + 1);
   bool rightEmpty = map.isNotSolid(x + 1, y + 1);

   // Hacky solution, but works
   if (rightEmpty && leftEmpty && chance(50)) {
      rightEmpty = false;
   }

   if (rightEmpty) {
      map.moveBlock(x, y, x + 1, y + 1);
   } else if (leftEmpty) {
      map.moveBlock(x, y, x - 1, y + 1);
   }
}

void GameState::updateGrassPhysics(int x, int y) {
   if (!map.is(x, y - 1, BlockType::solid)) {
      return;
   }

   Block &block = map.blocks[y][x];
   if (block.value2 == 0) {
      block.value2 = random(grassGrowSpeedMin, grassGrowSpeedMax);
   }

   block.value += 1;
   if (block.value >= block.value2) {
      block.value = 0;
      block.value2 = 0;

      // This might be a tripping point in the future, when more dirt and
      // grass is added. I don't care though, I don't want to create a map
      // here, which'll also be a tripping point. Just define grass exactly
      // before dirt in objs/map.cpp, please.
      map.setBlock(x, y, block.id + 1);
   }
}

void GameState::updateDirtPhysics(int x, int y) {
   if (map.is(x, y - 1, BlockType::solid)) {
      return;
   }

   Block &block = map.blocks[y][x];
   if (block.value2 == 0) {
      block.value2 = random(grassGrowSpeedMin, grassGrowSpeedMax);
   }

   block.value += 1;
   if (block.value >= block.value2) {
      block.value = 0;
      block.value2 = 0;
   
      // Same as before. Just define grass exactly before dirt, so IDs
      // match right
      map.setBlock(x, y, block.id - 1);
   }
}

void GameState::updateTorchPhysics(int x, int y) {
   Block &block = map.blocks[y][x];
   block.value = (block.value + 1) % 5;

   if (map.getLiquidHeight(x, y) > liquidToBlockThreshold) {
      map.deleteBlockWithoutDeletingLiquids(x, y);
      return;
   }
   bool downEmpty = !map.is(x, y + 1, BlockType::solid) && !map.is(x, y + 1, BlockType::furniture);

   if (downEmpty && map.isStable(x - 1, y)) {
      block.value2 = 2;
   } else if (downEmpty && map.isStable(x + 1, y)) {
      block.value2 = 3;
   } else if (downEmpty && !(map.walls[y][x].type & BlockType::empty)) {
      block.value2 = 4;
   } else if (!downEmpty && map.isStable(x, y - 1)) {
      block.value2 = 1;
   } else if (!downEmpty) {
      block.value2 = 0;
   } else {
      map.deleteBlock(x, y);
   }
}

// Render

void GameState::render() {
   const float delta = (phase != Phase::playing ? 0 : player.delta.x * dt);
   drawBackground(foregroundTexture, backgroundTexture, delta, delta, (phase == Phase::paused ? 0.0f : 1.0f) * dt);

   BeginMode2D(camera);
   map.render(droppedItems, player, accumulator, cameraBounds, camera, inventory);

   renderParticles();
   for (const DamageIndicator &indicator: map.damageIndicators) {
      drawText(indicator.position, std::to_string(indicator.damage).c_str(), 1.0f, (indicator.critical ? YELLOW : RED), 0.1f);
   }

   // Render effects
   if (player.hearts != player.maxHearts) {
      drawTextureNoOrigin(getTexture("vignette"), {0, 0}, getScreenSize(), Fade(WHITE, 1.0f - float(player.hearts) / player.maxHearts));
   }

   if (phase == Phase::died) {
      EndMode2D();
      drawText(getScreenCenter({0, -30.0f}), "YOU'VE DIED!", 120, RED);
      drawText(getScreenCenter({0, 30.0f}), format("RESPAWN IN {}...", int(timeToRespawn - deathTimer)).c_str(), 50, RED);
      return;
   }

   // Render block preview
   if (canDrawPreview) {
      Vector2 mousePos = GetScreenToWorld2D(GetMousePosition(), camera);
      int mouseX = mousePos.x;
      int mouseY = mousePos.y;
      
      const Item &item = inventory.getSelected();

      if (item.isFurniture) {
         BlockType below = (map.isPositionValid(mouseX, mouseY + furniturePreview.sizeY) ? map.blocks[mouseY + furniturePreview.sizeY][mouseX].type : BlockType::empty);

         if (lastFurnitureType != getFurnitureType(item.id) || oldBlockBelowPreview != below || flippedPreviewX != player.flipX) {
            furniturePreview = getFurniture(mouseX, mouseY, map, getFurnitureType(item.id), player.flipX, true);
         }
         flippedPreviewX = player.flipX;
         lastFurnitureType = furniturePreview.type;
         oldBlockBelowPreview = below;

         furniturePreview.posX = mouseX;
         furniturePreview.posY = mouseY;
         furniturePreview.preview(map);
      } else {
         DrawTexturePro(getTexture(getBlockNameFromId(item.id)), {0, 0, 8, 8}, {(float)mouseX, (float)mouseY, 1, 1}, {0, 0}, 0, Fade(item.isWall ? wallTint : WHITE, previewAlpha));
      }
   }

   // Render block breaking preview
   if (player.breakTime != 0.0f) {
      // TODO: Change this in the future, shit logic
      float breakTime = (player.breakingFurniture ? getFurnitureBreakingTime(map.getFurnitureAtPosition(player.lastBreakingX, player.lastBreakingY).id) : getBlockBreakingTime((player.breakingWall ? map.walls : map.blocks)[player.lastBreakingY][player.lastBreakingX].id));
      int textureX = (player.breakTime / breakTime) * 5;
      Texture2D &texture = getTexture("breaking");
      DrawTexturePro(texture, {textureX * 8.0f, 0, 8, 8}, {(float)player.lastBreakingX, (float)player.lastBreakingY, 1, 1}, {0, 0}, 0, (player.breakingWall ? wallTint : WHITE));
   }

   // Render breath dynamically
   if (player.breath != maxBreath) {
      Texture2D &bubbleIcon = getTexture("bubble_icon");
      
      float size = 1.25f;
      float padding = size + 0.075f;
      int breathValue = 10;
      int bubbles = 10;
      float startingY = player.position.y - 1.25f;
      float startingX = player.getCenter().x - padding * 5;

      float static sineCounter = 0.0f;
      sineCounter += 1.0f - float(player.breath) / maxBreath;
      float sine = std::sin(sineCounter * 0.5f) / 20.0f;
      float halfSine = sine / 2.0f;

      for (int i = 0; i < bubbles; ++i) {
         float a = 1.0f - min(1.0f, float((i + 1) * breathValue - player.displayBreath) / breathValue);
         drawTextureNoOrigin(bubbleIcon, {startingX + padding * i - halfSine, startingY - halfSine}, {size + sine, size + sine}, Fade(WHITE, a));
      }
   }
   EndMode2D();

   // Render all of the hearts dynamically
   Texture2D &heartIcon = getTexture("heart_icon");
   Shader &grayscaleShader = getShader("grayscale");
   
   float size = 25;
   float padding = size + 5;
   int heartValue = 20;
   int counter = player.maxHearts / heartValue;
   int heartsPerRow = 10;
   float startingY = 40;
   float startingX = GetScreenWidth() - size * heartsPerRow - 5 * (heartsPerRow - 1) - 15;

   float static sineCounter = 0.0f;
   sineCounter += 1.0f - float(player.hearts) / player.maxHearts;
   float sine = std::sin(sineCounter * 0.5f);
   float halfSine = sine / 2.0f;

   BeginShaderMode(grayscaleShader);
   for (int i = 0; i < counter; ++i) {
      float a = 1.0f - min(1.0f, float((i + 1) * heartValue - player.displayHearts) / heartValue);
      drawTextureNoOrigin(heartIcon, {startingX + padding * (i % heartsPerRow) - halfSine, startingY + padding * int(i / heartsPerRow) - halfSine}, {size + sine, size + sine}, Fade(WHITE, a));
   }
   EndShaderMode();
   drawText({startingX + (GetScreenWidth() - startingX) / 2.0f, startingY / 2.0f}, format("HP: {}/{}", player.hearts, player.maxHearts).c_str(), 20);

   // Render other game UI
   inventory.render();

   if (phase == Phase::paused) {
      continueButton.render();
      menuButton.render();
   }
   pauseButton.render();
}

// Change states

State* GameState::change() {
   return new MenuState();
}

// Helper functions

void GameState::calculateCameraBounds() {
   // formula I pulled out my ass that magically works
   camera.target.x = clamp(camera.target.x * camera.zoom, camera.offset.x, map.sizeX * camera.zoom - camera.offset.x) / camera.zoom;
   camera.target.y = clamp(camera.target.y * camera.zoom, camera.offset.y, map.sizeY * camera.zoom - camera.offset.y) / camera.zoom;

   cameraBounds = getCameraBounds(camera);
   cameraBounds.x = max(0, int(cameraBounds.x));
   cameraBounds.y = max(0, int(cameraBounds.y));
   cameraBounds.width = min(map.sizeX - 1, int(cameraBounds.x + cameraBounds.width) + 1);
   cameraBounds.height = min(map.sizeY - 1, int(cameraBounds.y + cameraBounds.height) + 1);
}
