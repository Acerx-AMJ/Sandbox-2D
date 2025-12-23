#ifndef UI_UICONSTANTS_HPP
#define UI_UICONSTANTS_HPP

#include <raylib.h>

// Block constants

constexpr inline Color wallTint     = {120, 120, 120, 255};
constexpr inline float previewAlpha = 0.75f;
constexpr inline int textureSize = 8;

// Parallax constants

constexpr inline float parallaxBgSpeed = 75.0f;
constexpr inline float parallaxFgSpeed = 100.0f;

// UI constants

constexpr inline float fadeTime = 0.4f;

constexpr inline float buttonWidth         = 210.0f;
constexpr inline float buttonHeight        = 70.0f;
constexpr inline float buttonPaddingX      = buttonWidth + 20.0f;
constexpr inline float buttonPaddingY      = buttonHeight + 20.0f;
constexpr inline float buttonOffsetX     = 120.0f;
constexpr inline float scrollBarWidth = 56.667f;

#endif
