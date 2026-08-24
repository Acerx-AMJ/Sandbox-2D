#include "objs/parallax.hpp"
#include "SRU/assets.hpp"
#include "SRU/random.hpp"
#include "SRU/render.hpp"
#include "SRU/util.hpp"
#include <cmath>
#include <string>
#include <vector>

// Constants

constexpr inline float parallaxBgSpeed = 75.0f;
constexpr inline float parallaxFgSpeed = 100.0f;

constexpr int starCountMin = 10;
constexpr int starCountMax = 50;
constexpr int moonPhaseCount = 8;
constexpr Vector2 sunSize = {0.083f, 0.083f};
constexpr Vector2 moonSize = {0.056f, 0.056f};
constexpr Vector2 starSizeMin = {20.0f, 20.0f};
constexpr Vector2 starSizeMax = {50.0f, 50.0f};

constexpr Color skyColorNight = {30, 30, 30, 255};
constexpr Color skyColorDay = {255, 255, 255, 255};
constexpr Color backgroundTintNight = {35, 35, 35, 255};
constexpr Color backgroundTintDay = {190, 190, 170, 255};
constexpr Color foregroundTintNight = {40, 40, 40, 255};
constexpr Color foregroundTintDay = {210, 210, 190, 255};

using s = std::vector<std::string>;
static inline const std::array<s, (size_t)MapGenerator::Biome::count> biomeBackgroundTextures {
   s{"mountains1", "mountains2", "mountains3", "mountains4", "mountains5"},       // plains
   s{"mountains1", "mountains2", "mountains3", "mountains4", "mountains5"},       // forest
   s{"mountains_m1", "mountains_m2", "mountains_m3"},                             // mountains
   s{"mountains_sand1", "mountains_sand2", "mountains_sand3", "mountains_sand4"}, // oasis
   s{"mountains_sand1", "mountains_sand2", "mountains_sand3", "mountains_sand4"}, // desert
   s{"mountains1", "mountains2", "mountains3", "mountains4", "mountains5"},       // tundra
   s{"mountains1", "mountains2", "mountains3", "mountains4", "mountains5"},       // jungle
};

static inline const std::array<s, (size_t)MapGenerator::Biome::count> biomeForegroundTextures {
   s{"bg_trees1", "bg_trees2", "bg_trees3", "bg_trees4"},   // plains
   s{"bg_trees1", "bg_trees2", "bg_trees3", "bg_trees4"},   // forest
   s{"bg_trees1", "bg_trees2"},                             // mountains
   s{"bg_trees_sand1", "bg_trees_sand2", "bg_trees_sand3"}, // oasis
   s{"bg_trees_sand1", "bg_trees_sand2", "bg_trees_sand3"}, // desert
   s{"bg_trees1", "bg_trees2", "bg_trees3", "bg_trees4"},   // tundra
   s{"bg_trees1", "bg_trees2", "bg_trees3", "bg_trees4"},   // jungle
};

struct Star {
   Vector2 size, position;
   unsigned char frameX = 0;
};

// Static members

static Star stars[starCountMax];
static int starCount = 0;
static float fgProgress = 0;
static float bgProgress = 0;
static float currentTime = 0;
static float lastTime = 0;
static int moonPhase = -1;
static int lastMoonPhase = -1;
static bool isNight = false;
static MapGenerator::Biome biome;
static Texture *bgTexture = nullptr;
static Texture *fgTexture = nullptr;

// Color helper functions

float getFadeStrengthBasedOnTime() {
   if (currentTime >= 45.0f && currentTime <= 135.0f) {
      return 0.0f;
   } else if (currentTime >= 225.0f && currentTime <= 315.0f) {
      return 1.0f;
   } else if (currentTime >= 315.0f) {
      return 1.0f - (currentTime - 315.0f) / 90.0f;
   } else if (currentTime <= 45.0f) {
      return 1.0f - (currentTime + 45.0f) / 90.0f;
   } else if (currentTime >= 135.0f && currentTime <= 225.0f) {
      return (currentTime - 135.0f) / 90.f;
   }
   return 1.0f;
}

// Background functions

void resetBackground() {
   fgProgress = bgProgress = currentTime = lastTime = 0;
   moonPhase = lastMoonPhase = -1;
   isNight = false;
}

void drawBackground(float bgSpeed, float fgSpeed, float daySpeed) {
   Vector2 screenSize = getWindowSize();
   Vector2 origin = getOrigin(screenSize);

   bool wasNight = isNight;
   int prevMoonPhase = moonPhase;

   // Update parallax backgrounds
   bgProgress -= bgSpeed * parallaxBgSpeed;
   fgProgress -= fgSpeed * parallaxFgSpeed;
   
   if (bgProgress <= -screenSize.x) {
      bgProgress = 0.f;
   }
   if (bgProgress > 0.f) {
      bgProgress = -screenSize.x;
   }

   if (fgProgress <= -screenSize.x) {
      fgProgress = 0.f;
   }
   if (fgProgress > 0.f) {
      fgProgress = -screenSize.x;
   }

   // Update night
   currentTime = std::fmod(currentTime + daySpeed, 360.0f);
   lastTime = currentTime;
   isNight = (currentTime >= 180.0f);

   if ((wasNight && !isNight) || moonPhase < 0) {
      moonPhase = (moonPhase + 1) % moonPhaseCount;
      lastMoonPhase = moonPhase;
   }

   float t = getFadeStrengthBasedOnTime();

   // Draw the sky
   drawTexture("sky", getWindowArea(), TOP_LEFT, ColorLerp(skyColorDay, skyColorNight, t));

   // Draw the stars
   if (prevMoonPhase != moonPhase) {
      starCount = randomInt(starCountMin, starCountMax);
      for (int i = 0; i < starCount; ++i) {
         Star &star = stars[i];
         star.size.x = star.size.y = randomFloat(starSizeMin.x, starSizeMax.x);
         
         star.position = {randomFloat(0.0f, 1.0f), randomFloat(0.0f, 0.5f)};
         star.frameX = randomInt(0, 3);
      }
   }

   Texture &starTexture = getTexture("stars");
   for (int i = 0; i < starCount; ++i) {
      Star &star = stars[i];
      DrawTexturePro(starTexture, {(float)star.frameX * starTexture.height, 0.0f, (float)starTexture.height, (float)starTexture.height}, {star.position.x * screenSize.x, star.position.y * screenSize.y, star.size.x, star.size.y}, {0, 0}, 0, Fade(WHITE, 0.5f - (1.0f - t)));
   }

   // Draw either moon or sun based on the time
   if (isNight) {
      Texture &texture = getTexture("moon");
      Vector2 position = {origin.x, screenSize.y};
      Vector2 size = mapRatioToArea(moonSize, WINDOW_AREA, CUBIC_RATIO);

      DrawTexturePro(texture, {(float)moonPhase * texture.height, 0.0f, (float)texture.height, (float)texture.height}, {position.x, position.y, size.x, size.y}, origin, currentTime - 180.0f, WHITE);
   } else {
      Texture &texture = getTexture("sun");
      Vector2 position = {origin.x, screenSize.y};
      Vector2 size = mapRatioToArea(sunSize, WINDOW_AREA, CUBIC_RATIO);

      DrawTexturePro(texture, getSource(texture), {position.x, position.y, size.x, size.y}, origin, currentTime, WHITE);
   }

   // Draw backgrounds
   if (bgTexture) {
      Color bgColor = ColorLerp(backgroundTintDay, backgroundTintNight, t);
      drawTexture(*bgTexture, {bgProgress, 0.0f}, screenSize, TOP_LEFT, bgColor);
      drawTexture(*bgTexture, {screenSize.x + bgProgress, 0}, screenSize, TOP_LEFT, bgColor);
   }

   if (fgTexture) {
      Color fgColor = ColorLerp(foregroundTintDay, foregroundTintNight, t);
      drawTexture(*fgTexture, {fgProgress, 0}, screenSize, TOP_LEFT, fgColor);
      drawTexture(*fgTexture, {screenSize.x + fgProgress, 0}, screenSize, TOP_LEFT, fgColor);
   }
}

// Time functions

float getTimeOfDay() {
   return currentTime;
}

int getMoonPhase() {
   return moonPhase;
}

void setTimeOfDay(float timeOfDay) {
   lastTime = currentTime = timeOfDay;
}

void setMoonPhase(int moonPhase) {
   lastMoonPhase = moonPhase - 1;
   ::moonPhase = moonPhase;
}

// Texture functions

unsigned char getLightBasedOnTime() {
   return Lerp(0, 140, getFadeStrengthBasedOnTime());
}

void setCurrentBackgroundBiome(MapGenerator::Biome biome) {
   if (::biome != biome || !bgTexture || !fgTexture) {
      bgTexture = &getTexture(randomElement(biomeBackgroundTextures[(size_t)biome]));
      fgTexture = &getTexture(randomElement(biomeForegroundTextures[(size_t)biome]));   
   }
   ::biome = biome;
}
