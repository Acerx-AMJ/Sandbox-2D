#pragma once
#include "objs/generation.hpp"
#include <raylib.h>

void resetBackground();
void drawBackground(float bgSpeed, float fgSpeed, float daySpeed);

int getMoonPhase();
float getTimeOfDay();

void setMoonPhase(int moonPhase);
void setTimeOfDay(float timeOfDay);

unsigned char getLightBasedOnTime();
void setCurrentBackgroundBiome(MapGenerator::Biome biome);
