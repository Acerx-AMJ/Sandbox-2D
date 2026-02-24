#include "util/parallax.hpp"
#include "mngr/resource.hpp"
#include "util/position.hpp"
#include "util/random.hpp"
#include "util/render.hpp"
#include <cmath>
#include <string>
#include <vector>

// Constants

constexpr inline float parallaxBgSpeed = 75.0f;
constexpr inline float parallaxFgSpeed = 100.0f;

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

static inline const std::vector<std::string> backgroundTextures {
   "mountains1", "mountains2", "mountains3", "mountains4", "mountains5"
};

static inline const std::vector<std::string> foregroundTextures {
   "bg_trees1", "bg_trees2", "bg_trees3", "bg_trees4"
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

// Color helper functions

inline Color fadeColor(const Color &a, const Color &b, float t) {
   float i = 1.0f - t;
   return {(unsigned char)((a.r * i) + (b.r * t)), (unsigned char)((a.g * i) + (b.g * t)), (unsigned char)((a.b * i) + (b.b * t)), 255};
}

float getFadeStrengthBasedOnTime() {
   if (currentTime >= 45.0f && currentTime <= 135.0f) {
      return 1.0f;
   } else if (currentTime >= 225.0f && currentTime <= 315.0f) {
      return 0.0f;
   } else if (currentTime >= 315.0f) {
      return (currentTime - 315.0f) / 90.0f;
   } else if (currentTime <= 45.0f) {
      return (currentTime + 45.0f) / 90.0f;
   } else if (currentTime >= 135.0f && currentTime <= 225.0f) {
      return 1.0f - (currentTime - 135.0f) / 90.f;
   }
   return 0.0f;
}

// Background functions

void resetBackground() {
   fgProgress = bgProgress = currentTime = lastTime = 0;
   moonPhase = lastMoonPhase = -1;
   isNight = false;
}

void drawBackground(const Texture &fgTexture, const Texture &bgTexture, float bgSpeed, float fgSpeed, float daySpeed) {
   Vector2 screenSize = getScreenSize();
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
   DrawTexturePro(getTexture("sky"), getBox(getTexture("sky")), {0, 0, getScreenSize().x, getScreenSize().y}, {0, 0}, 0, fadeColor(skyColorNight, skyColorDay, t));

   // Draw the stars
   if (prevMoonPhase != moonPhase) {
      starCount = random(starCountMin, starCountMax);
      for (int i = 0; i < starCount; ++i) {
         Star &star = stars[i];
         star.size.x = star.size.y = random(starSizeMin.x, starSizeMax.x);
         
         star.position = {random(0.0f, (float)GetScreenWidth()), random(0.0f, GetScreenHeight() / 2.f)};
         star.frameX = random(0, 3);
      }
   }

   Texture &starTexture = getTexture("stars");
   for (int i = 0; i < starCount; ++i) {
      Star &star = stars[i];
      DrawTexturePro(starTexture, {(float)star.frameX * starTexture.height, 0.0f, (float)starTexture.height, (float)starTexture.height}, {star.position.x, star.position.y, star.size.x, star.size.y}, {0, 0}, 0, Fade(WHITE, 0.5f - t));
   }

   // Draw either moon or sun based on the time
   if (isNight) {
      Texture &texture = getTexture("moon");
      Vector2 position = {origin.x, screenSize.y};

      DrawTexturePro(texture, {(float)moonPhase * texture.height, 0.0f, (float)texture.height, (float)texture.height}, {position.x, position.y, moonSize.x, moonSize.y}, origin, currentTime - 180.0f, WHITE);
   } else {
      Texture &texture = getTexture("sun");
      Vector2 position = {origin.x, screenSize.y};

      DrawTexturePro(texture, getBox(texture), {position.x, position.y, sunSize.x, sunSize.y}, origin, currentTime, WHITE);
   }

   // Draw backgrounds
   Color bgColor = fadeColor(backgroundTintNight, backgroundTintDay, t);
   drawTextureNoOrigin(bgTexture, {bgProgress, 0}, screenSize, bgColor);
   drawTextureNoOrigin(bgTexture, {screenSize.x + bgProgress, 0}, screenSize, bgColor);

   Color fgColor = fadeColor(foregroundTintNight, foregroundTintDay, t);
   drawTextureNoOrigin(fgTexture, {fgProgress, 0}, screenSize, fgColor);
   drawTextureNoOrigin(fgTexture, {screenSize.x + fgProgress, 0}, screenSize, fgColor);
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

Color getLightBasedOnTime() {
   return fadeColor({15, 15, 15, 255}, WHITE, getFadeStrengthBasedOnTime());
}

Texture& getRandomBackground() {
   return getTexture(random(backgroundTextures));
}

Texture& getRandomForeground() {
   return getTexture(random(foregroundTextures));
}
