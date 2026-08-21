#include "game/gameState.hpp"
#include "game/menuState.hpp"
#include "mngr/input.hpp"
#include "mngr/fileio.hpp"
#include "objs/parallax.hpp"
#include "SRU/audio.hpp"
#include "SRU/assets.hpp"
#include "SRU/particles.hpp"
#include "SRU/random.hpp"
#include "SRU/render.hpp"
#include "SRU/util.hpp"

// Constructors

GameState::GameState(const std::string &worldName) {
   this->worldName = worldName;
   
   // Init world and camera
   loadWorldData(worldName, player, camera.zoom, map, console, inventory, droppedItems);

   camera.zoom = std::clamp(camera.zoom, minCameraZoom, maxCameraZoom);
   camera.target = player.getCenter();
   camera.offset = getWindowCenter();
   camera.rotation = 0.0f;
   calculateCameraBounds();

   // Init UI
   Font font = getFont("andy");
   Texture button = getTexture("button");
   continueButton.init(font, button, CENTER, "Continue");
   menuButton.init(font, button, CENTER, "Save & Quit");
   pauseButton.init(font, {0}, CENTER, "Pause");

   liquidCounters.resize(getLiquidCount());
   console.init(*this);
   updateResponsiveness();
}

GameState::~GameState() {
   inventory.discardSelection();
   pushPendingDroppedItems();

   saveWorldData(worldName, player.spawnPos, player.position, player.creative, player.breath, player.hearts, player.maxHearts, camera.zoom, map, &console, &inventory, &droppedItems);
   resetBackground();
}

// Update

void GameState::update() {
   if (phase != Phase::died && !console.input.typing) {
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
   camera.target = Vector2Lerp(camera.target, player.getCenter(), cameraFollowSpeed);
   if (phase == Phase::paused) {
      calculateCameraBounds(); // Make sure the camera does not go out of bounds
      return;
   }

   if (player.hearts == 0) {
      Phase lastPhase = phase;
      phase = Phase::died;

      if (lastPhase != phase) {
         playSound("die");
         spawnParticles("dust", 0, nullptr, player.getCenter(), false);
      }
      calculateCameraBounds(); // Make sure the camera does not go out of bounds
   } else {
      player.updatePlayer(map);
   }

   // Update physics
   physicsCounter = (physicsCounter + 1) % physicsTicks;
   if (physicsCounter != 0) {
      return;
   }

   Rectangle physicsBounds = cameraBounds;
   Vector2 halfSize = {(cameraBounds.width - cameraBounds.x) / 2.0f, (cameraBounds.height - cameraBounds.y) / 2.0f};
   physicsBounds.x = std::max<int>(0, cameraBounds.x - halfSize.x);
   physicsBounds.y = std::max<int>(0, cameraBounds.y - halfSize.y);
   physicsBounds.width = std::min<int>(map.sizeX - 1, cameraBounds.width + halfSize.x);
   physicsBounds.height = std::min<int>(map.sizeY - 1, cameraBounds.height + halfSize.y);

   // update liquid counters
   for (liquidid_t i = 1; i < liquidCounters.size(); ++i) {
      if (liquidCounters[i] >= getLiquidData(i).updateSpeed) {
         liquidCounters[i] = 0;
      }
      liquidCounters[i] += 1;
   }

   // Loop backwards to avoid updating most of the moving blocks twice
   for (int y = physicsBounds.height; y >= physicsBounds.y; --y) {
      for (int x = physicsBounds.width; x >= physicsBounds.x; --x) {
         if (map.isAnyLiquid(x, y)) {
            liquidid_t id = map.getLiquidId(x, y);

            if (liquidCounters[id] >= getLiquidData(id).updateSpeed) {
               updateLiquid(x, y, id);
            }
         }

         BlockType type = map.getBlock(x, y).type;
         if (BlockTypeHas(type, BlockType::sand)) {
            updateSandPhysics(x, y);
         } else if (BlockTypeHas(type, BlockType::grass)) {
            updateGrassPhysics(x, y);
         } else if (BlockTypeHas(type, BlockType::dirt)) {
            updateDirtPhysics(x, y);
         } else if (BlockTypeHas(type, BlockType::torch)) {
            updateTorchPhysics(x, y);
         }
      }
   }
}

void GameState::updateResponsiveness() {
   camera.offset = getWindowCenter();
   console.updateResponsiveness();

   Vector2 padding = convertRatio(buttonPadding, CUBIC_RATIO, RATIO);
   Vector2 rawPadding = convertRatio(rawButtonPadding, CUBIC_RATIO, RATIO);

   continueButton.rect = mapRatioToArea(R4(V2(0.5f, 0.5f), buttonSize), TOP_LEFT, WINDOW_AREA, CUBIC_RATIO);
   menuButton.rect = mapRatioToArea(R4(V2(0.5f, 0.5f + padding.y), buttonSize), TOP_LEFT, WINDOW_AREA, CUBIC_RATIO);
   pauseButton.rect = mapRatioToArea(R4(V2(1.0f, 1.0f) - rawPadding, buttonSize), CENTER, WINDOW_AREA, CUBIC_RATIO);

   // must update map's lightmap due to the render texture being fixed to the screen size
   if (map.lightmap.id != 0) UnloadRenderTexture(map.lightmap);
   map.lightmap = LoadRenderTexture(GetScreenWidth() / 2, GetScreenHeight() / 2);
}

// Update playing

void GameState::updatePlaying() {
   const float zoomFactor = isKeyPressed(KEY_EQUAL) - isKeyPressed(KEY_MINUS);
   if (zoomFactor != 0.f) {
      camera.zoom = std::clamp(std::exp(std::log(camera.zoom) + zoomFactor * 0.2f), minCameraZoom, maxCameraZoom);
   }

   // Console
   setInputBlocking(false);
   if (IsKeyDown(KEY_LEFT_CONTROL) && isKeyPressed(KEY_TAB)) {
      console.input.typing = !console.input.typing;
   }
   console.update(dt, *this);

   if (console.input.typing && IsKeyPressed(KEY_ESCAPE)) {
      console.input.typing = false;
   }
   setInputBlocking(console.input.typing);
   player.blockInput = console.input.typing;

   inventory.update(!console.input.typing);
   calculateCameraBounds();

   if (phase != Phase::playing) {
      return;
   }

   // Update furniture
   Vector2 mousePos = GetScreenToWorld2D(GetMousePosition(), camera);
   Vector2 playerCenter = player.getCenter();
   int mouseX = mousePos.x;
   int mouseY = mousePos.y;

   map.updateFurniture(player, mousePos, dt);

   // Place and destroy blocks
   bool actionPossible = map.isPositionValid(mouseX, mouseY) && Vector2Distance(mousePos, playerCenter) <= maxToolRange;
   player.breakingBlock = false;

   if (actionPossible && isMousePressedOutsideUI(MOUSE_BUTTON_MIDDLE)) {
      const Block &block = map.getBlock(mouseX, mouseY);
      blockid_t blockId = (block.tile == TileType::root) * block.id;
      blockid_t wallId = map.isEmpty(mouseX, mouseY) * map.getWall(mouseX, mouseY).id;
      furnitureid_t furnitureId = (block.tile == TileType::ghost) * block.id;
      liquidid_t liquidId = map.isNotSolid(mouseX, mouseY) * map.getLiquidId(mouseX, mouseY);

      inventory.pickItem(blockId, wallId, furnitureId, liquidId);
   }
   else if (actionPossible && isMouseDownOutsideUI(MOUSE_BUTTON_RIGHT) && inventory.anyItemSelected() && player.canPlaceBlock) {
      ItemData &data = inventory.getSelectedItem();

      if (data.action == ItemActionType::placeBlock && map.isNotSolid(mouseX, mouseY) && map.blockNear(mouseX, mouseY)) {
         map.setBlock(mouseX, mouseY, data.block);
         inventory.useSelectedItem();
         player.placeBlock();
      }
      else if (data.action == ItemActionType::placeWall && map.isWall(mouseX, mouseY, BlockType::empty) && map.blockNear(mouseX, mouseY)) {
         map.setWall(mouseX, mouseY, data.wall);
         inventory.useSelectedItem();
         player.placeBlock();
      }
      else if (data.action == ItemActionType::placeFurniture) {
         Furniture furniture = getFurniture(mouseX, mouseY, map, data.furniture, player.flipX);
         if (furniture.id != 0) {
            map.addFurniture(furniture);
            inventory.useSelectedItem();
            player.placeBlock();
         }
      }
      else if (data.action == ItemActionType::placeLiquid && (map.getBlock(mouseX, mouseY).tile != TileType::root || map.isEmpty(mouseX, mouseY))) {
         map.setLiquid(mouseX, mouseY, data.liquid, maxLiquidLayers);
         inventory.useSelectedItem();
         player.placeBlock();
      }
   }
   else if (actionPossible && isMouseDownOutsideUI(MOUSE_BUTTON_LEFT)) {
      const Block &block = map.getBlock(mouseX, mouseY);
      bool breakingFurniture = (block.tile == TileType::ghost);
      bool breakingWall = (!breakingFurniture && map.isNotSolid(mouseX, mouseY));
      float breakSpeed = 0.0f;
      
      if (breakingFurniture) {
         FurnitureData &data = getFurnitureData(block.id);
         breakSpeed = data.breakSpeed;
      }
      else if (breakingWall) {
         BlockData &data = getBlockData(map.getWall(mouseX, mouseY).id);
         breakSpeed = data.wallBreakSpeed;
      }
      else {
         BlockData &data = getBlockData(block.id);
         breakSpeed = data.breakSpeed;
      }

      if (mouseX != player.lastBreakingX || mouseY != player.lastBreakingY || breakingFurniture != player.breakingFurniture || breakingWall != player.breakingWall) {
         player.breakTime = 0.0f;
      }

      player.breakTime += realDt;
      player.breakSpeed = breakSpeed;
      player.lastBreakingX = mouseX;
      player.lastBreakingY = mouseY;
      player.breakingFurniture = breakingFurniture;
      player.breakingWall = breakingWall;
      player.breakingBlock = true;

      if (player.breakTime >= breakSpeed) {
         if (breakingFurniture) {
            pushDropTable(getFurnitureData(block.id).dropTable);
            map.furniture[block.ghostId].destroy(map);
         }
         else if (breakingWall) {
            pushDropTable(getBlockData(map.getWall(mouseX, mouseY).id).wallDropTable);
            map.deleteWall(mouseX, mouseY);
         }
         else {
            pushDropTable(getBlockData(block.id).dropTable);
            map.deleteBlockWithoutDeletingLiquids(mouseX, mouseY);
         }
         player.breakTime = 0.0f;
      }
   }

   // Update dropped items
   pushPendingDroppedItems();

   for (auto &droppedItem: droppedItems) {
      droppedItem.update(cameraBounds, dt);

      if (!droppedItem.inBounds || Vector2Distance(playerCenter, {(float)droppedItem.tileX, (float)droppedItem.tileY}) > maxPickupRange) {
         continue;
      }
      int count = droppedItem.count;
      Item item {droppedItem.count, droppedItem.id, false};
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
      player.previousPosition = player.position = player.spawnPos;
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
   return std::min(availableSpace, flow1);
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

bool GameState::handleLiquidToBlock(int x, int y, liquidid_t id) {
   LiquidData &data = getLiquidData(id);
   for (const Vector2 &offset: {Vector2{1, 0}, Vector2{0, 1}, Vector2{-1, 0}, Vector2{0, -1}}) {
      int dx = x + offset.x, dy = y + offset.y;
      if (!map.isAnyLiquid(dx, dy) || map.isLiquidOfType(dx, dy, id)) {
         continue;
      }

      if (map.getLiquidHeight(dx, dy) >= liquidToBlockThreshold && map.getLiquidHeight(x, y) >= liquidToBlockThreshold && map.isNotSolid(dx, dy)) {
         liquidid_t target = map.getLiquidId(dx, dy);
         if (data.conversionTable.find(target) != data.conversionTable.end()) {
            map.setBlock(dx, dy, data.conversionTable[target]);
         }
      }
      map.setLiquid(x, y, 0, 0);
   }
   return map.isAnyLiquid(x, y);
}

void GameState::updateLiquid(int x, int y, liquidid_t id) {
   if (!handleLiquidToBlock(x, y, id)) {
      return;
   }
   liquidlayer_t height = map.getLiquidHeight(x, y);

   // Delete the liquid if its height is zero
   if (height == 0) {
      map.setLiquid(x, y, 0, 0);
      return;
   }

   // Handle liquid going down
   if ((map.getBlock(x, y + 1).tile == TileType::ghost || map.is(x, y + 1, BlockType::flowable)) && !map.isAnyLiquid(x, y + 1)) {
      std::swap(map.liquidTypes[y * map.sizeX + x], map.liquidTypes[(y + 1) * map.sizeX + x]);
      std::swap(map.liquidHeights[y * map.sizeX + x], map.liquidHeights[(y + 1) * map.sizeX + x]);
      return;
   } else if (map.isAnyLiquid(x, y + 1) && map.isLiquidOfType(x, y + 1, id) && map.getLiquidHeight(x, y + 1) < maxLiquidLayers) {
      applyFlowDown(map.liquidHeights[y * map.sizeX + x], map.liquidHeights[(y + 1) * map.sizeX + x]);
   }

   // Handle liquid going left
   if (((map.getBlock(x - 1, y).tile == TileType::ghost || map.is(x - 1, y, BlockType::flowable)) && !map.isAnyLiquid(x - 1, y))
    || (map.isAnyLiquid(x - 1, y) && map.isLiquidOfType(x - 1, y, id) && map.getLiquidHeight(x - 1, y) < height && map.getLiquidHeight(x - 1, y) < maxLiquidLayers)) {
      map.liquidTypes[y * map.sizeX + x - 1] = id;
      applyHalfFlowDown(map.liquidHeights[y * map.sizeX + x], map.liquidHeights[y * map.sizeX + x - 1]);
   }

   // Handle liquid going right
   if (((map.getBlock(x + 1, y).tile == TileType::ghost || map.is(x + 1, y, BlockType::flowable)) && !map.isAnyLiquid(x + 1, y))
    || (map.isAnyLiquid(x + 1, y) && map.isLiquidOfType(x + 1, y, id) && map.getLiquidHeight(x + 1, y) < height && map.getLiquidHeight(x + 1, y) < maxLiquidLayers)) {
      map.liquidTypes[y * map.sizeX + x + 1] = id;
      applyHalfFlowDown(map.liquidHeights[y * map.sizeX + x], map.liquidHeights[y * map.sizeX + x + 1]);
   }
}

void GameState::updateSandPhysics(int x, int y) {
   if (map.isNotSolid(x, y + 1)) {
      map.swapBlocks(x, y, x, y + 1);
      return;
   }

   bool leftEmpty  = map.isNotSolid(x - 1, y + 1);
   bool rightEmpty = map.isNotSolid(x + 1, y + 1);

   // Hacky solution, but works
   if (rightEmpty && leftEmpty && chance(50)) {
      rightEmpty = false;
   }

   if (rightEmpty) {
      map.swapBlocks(x, y, x + 1, y + 1);
   } else if (leftEmpty) {
      map.swapBlocks(x, y, x - 1, y + 1);
   }
}

void GameState::updateGrassPhysics(int x, int y) {
   if (!map.is(x, y - 1, BlockType::solid)) {
      return;
   }

   Block &block = map.blocks[y * map.sizeX + x];
   if (block.value2 == 0) {
      block.value2 = randomFloat(grassGrowSpeedMin, grassGrowSpeedMax);
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

   Block &block = map.blocks[y * map.sizeX + x];
   if (block.value2 == 0) {
      block.value2 = randomFloat(grassGrowSpeedMin, grassGrowSpeedMax);
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
   Block &block = map.blocks[y * map.sizeX + x];
   block.value = (block.value + 1) % 5;

   if (map.getLiquidHeight(x, y) > liquidToBlockThreshold) {
      map.deleteBlockWithoutDeletingLiquids(x, y);
      return;
   }
   bool downEmpty = map.isNotSolid(x, y + 1);

   if (downEmpty && map.isStable(x - 1, y)) {
      block.value2 = 2;
   } else if (downEmpty && map.isStable(x + 1, y)) {
      block.value2 = 3;
   } else if (downEmpty && !map.isWall(x, y, BlockType::empty)) {
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
   Font font = getFont("andy");
   const float delta = (phase != Phase::playing ? 0 : player.delta.x * dt);
   drawBackground(delta, delta, (phase == Phase::paused ? 0.0f : 1.0f) * dt);

   BeginMode2D(camera);
   map.render(droppedItems, player, accumulator, cameraBounds, camera, inventory);
   drawParticles();

   // Render effects
   if (!player.creative && player.hearts != player.maxHearts) {
      drawTexture("vignette", getWindowArea(), TOP_LEFT, Fade(WHITE, 1.0f - float(player.hearts) / player.maxHearts));
   }

   if (phase == Phase::died) {
      EndMode2D();
      drawTextResponsive(font, V2(0.5f, 0.5f - 0.0278f), "YOU'VE DIED!", 120, CENTER, RED);
      drawTextResponsive(font, V2(0.5f, 0.5f + 0.0278f), TextFormat("RESPAWN IN %d...", int(timeToRespawn - deathTimer)), 50, CENTER, RED);
      return;
   }

   Vector2 mousePos = GetScreenToWorld2D(GetMousePosition(), camera);
   Vector2 playerCenter = player.getCenter();

   // Render block preview
   if (inventory.anyItemSelected() && Vector2Distance(mousePos, playerCenter) <= maxToolRange) {
      ItemData &data = inventory.getSelectedItem();
      int mouseX = mousePos.x;
      int mouseY = mousePos.y;

      if (data.action == ItemActionType::placeBlock) {
         Color tint = (map.isNotSolid(mouseX, mouseY) && map.blockNear(mouseX, mouseY) ? WHITE : RED);
         Texture texture = (BlockTypeHas(getBlockData(data.block).attributes, BlockType::torch) ? data.texture : getBlockData(data.block).texture);
         drawTexture(texture, V2(mouseX, mouseY), {1.0f, 1.0f}, TOP_LEFT, Fade(tint, furniturePreviewAlpha));
      }
      else if (data.action == ItemActionType::placeWall && map.isWall(mouseX, mouseY, BlockType::empty)) {
         Color tint = (map.isNotSolid(mouseX, mouseY) && map.blockNear(mouseX, mouseY) ? wallTint : MAROON);
         drawTexture(getBlockData(data.wall).texture, V2(mouseX, mouseY), {1.0f, 1.0f}, TOP_LEFT, Fade(tint, furniturePreviewAlpha));
      }
      else if (data.action == ItemActionType::placeFurniture) {
         blockid_t below = (map.isPositionValid(mouseX, mouseY + furniturePreview.height) ? map.getBlock(mouseX, mouseY + furniturePreview.height).id : 0);
         if (lastFurnitureId != furniturePreview.id || oldBlockBelowPreview != below || flippedPreviewX != player.flipX) {
            furniturePreview = getFurniture(mouseX, mouseY, map, data.furniture, player.flipX, true);
         }
         flippedPreviewX = player.flipX;
         lastFurnitureId = data.furniture;
         oldBlockBelowPreview = below;

         furniturePreview.x = mouseX;
         furniturePreview.y = mouseY;
         furniturePreview.preview(map);
      }
      else if (data.action == ItemActionType::placeLiquid) {
         Color tint = (map.getBlock(mouseX, mouseY).tile != TileType::root || map.isEmpty(mouseX, mouseY) ? WHITE : RED);
         Shader shader = getShader("water_preview");
         float time = GetTime();

         SetShaderValue(shader, GetShaderLocation(shader, "time"), &time, SHADER_UNIFORM_FLOAT);
         BeginShaderMode(shader);
         drawTexture(getLiquidData(data.liquid).texture, V2(mouseX, mouseY), {1.0f, 1.0f}, TOP_LEFT, Fade(tint, furniturePreviewAlpha));
         EndShaderMode();
      }
   }

   // Render block breaking preview
   if (player.breakTime != 0.0f) {
      int textureX = (player.breakTime / player.breakSpeed) * 5;
      DrawTexturePro(getTexture("breaking"), {textureX * 8.0f, 0, 8, 8}, {(float)player.lastBreakingX, (float)player.lastBreakingY, 1, 1}, {0, 0}, 0, (player.breakingWall ? wallTint : WHITE));
   }

   // Render breath dynamically
   if (!player.creative && player.breath != maxBreath) {
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
         float a = 1.0f - std::min(1.0f, float((i + 1) * breathValue - player.displayBreath) / breathValue);
         drawTexture(bubbleIcon, {startingX + padding * i - halfSine, startingY - halfSine}, {size + sine, size + sine}, TOP_LEFT, Fade(WHITE, a));
      }
   }
   EndMode2D();

   // Render all of the hearts dynamically
   if (!player.creative) {
      Texture2D &heartIcon = getTexture("heart_icon");
      Shader &grayscaleShader = getShader("grayscale");

      float static sineCounter = 0.0f;
      sineCounter += 1.0f - float(player.hearts) / player.maxHearts;
      float sine = std::sin(sineCounter * 0.5f);
   
      int heartValue = 20;
      int heartsPerRow = 10;
      int heartCount = player.maxHearts / heartValue;
      int rows = (heartCount / heartsPerRow) + 1;
      Rectangle area = mapRatioToArea(R4(0.99f, 0.035f, heartsPerRow * 0.025f, rows * 0.025f), TOP_RIGHT, WINDOW_AREA, CUBIC_RATIO);
      Vector2 size = mapRatioToArea(0.023f, 0.023f, WINDOW_AREA, CUBIC_RATIO) - V2(sine);

      BeginShaderMode(grayscaleShader);
      for (int i = 0; i < heartCount; ++i) {
         float a = 1.0f - std::min(1.0f, float((i + 1) * heartValue - player.displayHearts) / heartValue);
         drawTexture(heartIcon, gridPosition(area, heartsPerRow, rows, i % heartsPerRow, i / heartsPerRow, CENTER), size, CENTER, Fade(WHITE, a));
      }
      EndShaderMode();
      // bit of a grid hack. index out of bounds to get the preffered position. only works because there are no checks there
      drawText(font, gridPosition(area, heartsPerRow, rows, heartsPerRow / 2, -1, CENTER_LEFT), TextFormat("HP: %d/%d", player.hearts, player.maxHearts), getFontSizeScaled(20));
   }

   // Render other game UI
   if (phase != Phase::died) {
      console.render();
   }
   inventory.render();

   if (phase == Phase::paused) {
      continueButton.render();
      menuButton.render();
   }
   pauseButton.render();

   // // Optionally render FPS counter through console command
   // if (map.fpsEnabled) {
   //    drawText({getScreenCenter().x, 40.0f * hr}, TextFormat("%d FPS", GetFPS()), getFontSize(40));
   // }
}

// Change states

State* GameState::change() {
   return new MenuState();
}

// Helper functions

void GameState::calculateCameraBounds() {
   // formula I pulled out my ass that magically works
   camera.target.x = std::clamp(camera.target.x * camera.zoom, camera.offset.x, map.sizeX * camera.zoom - camera.offset.x) / camera.zoom;
   camera.target.y = std::clamp(camera.target.y * camera.zoom, camera.offset.y, map.sizeY * camera.zoom - camera.offset.y) / camera.zoom;

   Vector2 pos = GetScreenToWorld2D({0, 0}, camera);
   Vector2 size = getWindowSize() / camera.zoom;
   cameraBounds = {pos.x, pos.y, size.x, size.y};

   cameraBounds.x = std::max(0, int(cameraBounds.x));
   cameraBounds.y = std::max(0, int(cameraBounds.y));
   cameraBounds.width = std::min(map.sizeX - 1, int(cameraBounds.x + cameraBounds.width) + 1);
   cameraBounds.height = std::min(map.sizeY - 1, int(cameraBounds.y + cameraBounds.height) + 1);
}

void GameState::pushPendingDroppedItems() {
   Vector2 center = player.getCenter();
   Vector2 dropPosition = {std::clamp<float>(center.x + (player.flipX ? 3 : -3), 0, map.sizeX - 1), center.y};
   for (Item &item: inventory.pendingDrops) {
      droppedItems.emplace_back(item, dropPosition.x, dropPosition.y);
   }
   inventory.pendingDrops.clear();
}

void GameState::pushDropTable(droptableid_t id) {
   DropTable &table = getDropTable(id);
   for (Drop &drop: table.drops) {
      if (drop.dropChance == 1.0f || chancePrecise(drop.dropChance)) {
         Item item {randomInt(drop.dropMin, drop.dropMax), drop.item};
         inventory.placeItemOrDrop(item);
      }
   }
}
