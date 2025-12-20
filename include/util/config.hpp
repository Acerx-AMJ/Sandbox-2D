#ifndef UTIL_CONFIG_HPP
#define UTIL_CONFIG_HPP

#include <raylib.h>

// Typedefs

using blockid_t = unsigned char;
using objid_t   = unsigned char;

// Keybinds

constexpr int pauseKey           = KEY_ESCAPE;
constexpr int toggleInventoryKey = KEY_E;

constexpr int moveRightKey     = KEY_D;
constexpr int moveLeftKey      = KEY_A;
constexpr int moveDownKey      = KEY_S;
constexpr int moveUpKey        = KEY_W;
constexpr int moveFastDebugKey = KEY_LEFT_SHIFT;
constexpr int jumpKey          = KEY_SPACE;

constexpr int zoomInKey  = KEY_EQUAL;
constexpr int zoomOutKey = KEY_MINUS;

constexpr int tempSwitchForward  = KEY_E;
constexpr int tempSwitchBackward = KEY_Q;
constexpr int tempSwitchWall     = KEY_R;

// Camera constants

constexpr float cameraFollowSpeed = 0.416f;
constexpr float minCameraZoom     = 12.5f;
constexpr float maxCameraZoom     = 200.0f;

// Physics constants

constexpr float physicsUpdateTime = 0.1f;
constexpr int lavaUpdateSpeed     = 6;
constexpr int cactusGrowSpeedMin  = 50;
constexpr int cactusGrowSpeedMax  = 255;
constexpr int grassGrowSpeedMin   = 100;
constexpr int grassGrowSpeedMax   = 255;

// Parallax constants

constexpr float parallaxBgSpeed = 75.0f;
constexpr float parallaxFgSpeed = 100.0f;
constexpr float menuSunSpeed    = 15.0f;
constexpr float gameSunSpeed    = 3.0f;

constexpr int starCountMin    = 10;
constexpr int starCountMax    = 50;
constexpr int moonPhaseCount  = 8;
constexpr Vector2 sunSize     = {90.0f, 90.0f};
constexpr Vector2 moonSize    = {60.0f, 60.0f};
constexpr Vector2 starSizeMin = {20.0f, 20.0f};
constexpr Vector2 starSizeMax = {50.0f, 50.0f};

constexpr Color skyColorNight       = {30, 30, 30, 255};
constexpr Color skyColorDay         = {255, 255, 255, 255};
constexpr Color backgroundTintNight = {35, 35, 35, 255};
constexpr Color backgroundTintDay   = {190, 190, 170, 255};
constexpr Color foregroundTintNight = {40, 40, 40, 255};
constexpr Color foregroundTintDay   = {210, 210, 190, 255};

// Block constants

constexpr Color wallTint     = {120, 120, 120, 255};
constexpr float previewAlpha = 0.75f;

// Game constants

constexpr int inventoryWidth  = 10;
constexpr int inventoryHeight = 4;

constexpr int defaultMapSizeX = 2000;
constexpr int defaultMapSizeY = 750;

// Texture constants

constexpr int textureSize = 8;

// Sound constants

constexpr float soundPitchMin = 0.95f;
constexpr float soundPitchMax = 1.05f;

// Generation constants

constexpr int saplingGrowTimeMin = 200;
constexpr int saplingGrowTimeMax = 1500;
constexpr int cactusGrowTimeMin  = 350;
constexpr int cactusGrowTimeMax  = 1750;

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

constexpr float startY        = 0.5f;
constexpr float seaLevel      = 0.4f;
constexpr int rockOffsetStart = 12;
constexpr int rockOffsetMin   = 5;
constexpr int rockOffsetMax   = 25;
constexpr int maxWaterLength  = 100;

// Player constants

constexpr Vector2 playerSize    = {1.8f, 2.7f};
constexpr float playerFrameSize = 16;

constexpr float playerUpdateSpeed = 1.f / 60.f;
constexpr float playerSpeed       = 2.182f;
constexpr float airMultiplier     = 0.6f;
constexpr float debugFlySpeed     = playerSpeed * 2.f;
constexpr float debugFastFlySpeed = playerSpeed * 5.f;
constexpr float jumpSpeed         = -6.5f;
constexpr float gravity           = 0.267f;
constexpr float maxGravity        = 7.333f;
constexpr float acceleration      = 0.083f;
constexpr float deceleration      = 0.167f;
constexpr float playerSmoothing   = 0.166f;

constexpr float coyoteTime = 0.1f;
constexpr float foxTime    = 0.1f;

// UI constants

constexpr int itemStackSize       = 9999;
constexpr int equipmentStackSize  = 1;
constexpr int potionStackSize     = 99;

constexpr int droppedItemLifetime      = 60.0f * 15.0f;
constexpr float droppedItemFloatSpeed  = 1.5f;
constexpr float droppedItemFloatHeight = 0.25f;
constexpr Vector2 droppedItemSize      = {0.8f, 0.8f};

constexpr float fadeTime = 0.4f;

constexpr float buttonWidth    = 210.0f;
constexpr float buttonHeight   = 70.0f;
constexpr float buttonPaddingY = buttonHeight + 20.0f;
constexpr float buttonScaleMin = 0.98f;
constexpr float buttonScaleMax = 1.02f;

constexpr float inputTextWrapPadding = 10.f;
constexpr float inputTextFadeSpeed   = 0.3f;
constexpr int inputTextFadeMin       = 190;
constexpr int inputTextFadeValue     = 255 - inputTextFadeMin;

constexpr float scrollBarWidth = 56.667f;
constexpr float scrollSpeed    = 15.f;

// UI positioning constants

constexpr float splashWrapOffset         = 50.0f;
constexpr float loadingIconRotationSpeed = 360.0f;
constexpr Vector2 loadingIconSize        = {70.0f, 70.0f};
constexpr Vector2 loadingTextOffset      = {0.0f, -175.0f};
constexpr Vector2 splashTextOffset       = {0.0f, 100.0f};

constexpr Vector2 worldNameSize            = {420.0f, 140.0f};
constexpr int maxWorldNameSize             = 48;
constexpr Vector2 worldFramePosition       = {280.0f, 200.0f};
constexpr Vector2 worldFrameSizeOffset     = {600.0f, 360.0f};
constexpr float worldCreationButtonOffsetX = 120.0f;

constexpr Vector2 itemframeSize           = {60.0f, 60.0f};
constexpr Vector2 itemframePadding        = {itemframeSize.x + 5.0f, itemframeSize.y + 5.0f};
constexpr Vector2 itemframeTopLeft        = {15.0f, 15.0f};
constexpr Vector2 itemframeIndexOffset    = {15.0f, 15.0f};
constexpr Vector2 itemframeItemSize       = {30.0f, 30.0f};
constexpr Vector2 itemframeItemOffset     = {(itemframeSize.x - itemframeItemSize.x) / 2.0f, (itemframeSize.y - itemframeItemSize.y) / 2.0f};
constexpr Vector2 selectedItemFrameSize   = {65.0f, 65.0f};
constexpr Vector2 selectedItemFrameOffset = {(selectedItemFrameSize.x - itemframeSize.x) / 2.0f, (selectedItemFrameSize.y - itemframeSize.y) / 2.0f};

constexpr float titleOffsetX  = -200.0f;
constexpr float titleOffsetX2 = -400.0f;

#endif
