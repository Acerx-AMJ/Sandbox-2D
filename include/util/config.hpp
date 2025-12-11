#ifndef UTIL_CONFIG_HPP
#define UTIL_CONFIG_HPP

#include <raylib.h>

// Typedefs

using blockid_t = unsigned char;
using objid_t   = unsigned char;

// Camera constants

constexpr float cameraFollowSpeed  = 0.416f;
constexpr float minCameraZoom      = 12.5f;
constexpr float maxCameraZoom      = 200.0f;
constexpr float minCameraZoomDebug = 1.25f;
constexpr float maxCameraZoomDebug = 400.0f;

// Physics constants

constexpr float physicsUpdateTime = 0.1f;
constexpr int lavaUpdateSpeed     = 6;
constexpr int cactusGrowSpeedMin  = 50;
constexpr int cactusGrowSpeedMax  = 255;
constexpr int grassGrowSpeedMin   = 100;
constexpr int grassGrowSpeedMax   = 255;

// Parallax constants

constexpr Color backgroundTint  = {190, 190, 170, 255};
constexpr Color foregroundTint  = {210, 210, 190, 255};
constexpr float parallaxBgSpeed = 75.0f;
constexpr float parallaxFgSpeed = 100.0f;

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

constexpr Vector2 playerSize    = {2.f, 3.f};
constexpr float playerFrameSize = 16;

constexpr float playerUpdateSpeed = 1.f / 60.f;
constexpr float playerSpeed       = 2.182f;
constexpr float airMultiplier     = 0.6f;
constexpr float debugFlySpeed     = playerSpeed * 2.f;
constexpr float debugFastFlySpeed = playerSpeed * 5.f;
constexpr float jumpSpeed         = -3.333f;
constexpr float gravity           = 0.267f;
constexpr float maxGravity        = 7.333f;
constexpr float acceleration      = 0.083f;
constexpr float deceleration      = 0.167f;
constexpr float playerSmoothing   = 0.166f;
constexpr float jumpHoldTime      = 0.4f;

// UI constants

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

constexpr Vector2 itemframeSize        = {60.0f, 60.0f};
constexpr Vector2 itemframePadding     = {itemframeSize.x + 5.f, itemframeSize.y + 5.f};
constexpr Vector2 itemframeTopLeft     = {15.0f, 15.0f};
constexpr Vector2 itemframeIndexOffset = {15.0f, 15.0f};
constexpr Vector2 itemframeItemSize    = {30.0f, 30.0f};
constexpr Vector2 itemframeItemOffset  = {(itemframeSize.x - itemframeItemSize.x) / 2.f, (itemframeSize.y - itemframeItemSize.y) / 2.f};

constexpr float titleOffsetX  = -200.0f;
constexpr float titleOffsetX2 = -400.0f;

#endif
