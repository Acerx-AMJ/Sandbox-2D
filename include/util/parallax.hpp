#ifndef UTIL_PARALLAX_HPP
#define UTIL_PARALLAX_HPP

#include <raylib.h>

// Parallax functions

void drawSky(float currentTime);
void drawParallaxTexture(const Texture &texture, float &progress, float speed, float currentTime, bool background);
Texture& getRandomBackground();
Texture& getRandomForeground();

// Sun functions

void drawSunAndMoon(float &currentTime, float speed, int &moonPhase, bool &isNight);

#endif
