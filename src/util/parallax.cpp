#include "util/parallax.hpp"
#include "mngr/resource.hpp"
#include "util/config.hpp"
#include "util/position.hpp"
#include "util/random.hpp"
#include "util/render.hpp"
#include <cmath>
#include <string>
#include <vector>

// Constants

static inline std::vector<std::string> backgroundTextures {
   "mountains1", "mountains2", "mountains3", "mountains4"
};

static inline std::vector<std::string> foregroundTextures {
   "bg_trees1", "bg_trees2", "bg_trees3", "bg_trees4"
};

// Color helper functions

inline Color fadeColor(const Color &a, const Color &b, float t) {
   float i = 1.0f - t;
   return {(unsigned char)((a.r * i) + (b.r * t)), (unsigned char)((a.g * i) + (b.g * t)), (unsigned char)((a.b * i) + (b.b * t)), 255};
}

float getFadeStrengthBasedOnTime(float currentTime) {
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

// Parallax functions

void drawSky(float currentTime) {
   float t = getFadeStrengthBasedOnTime(currentTime);
   DrawTexturePro(getTexture("sky"), getBox(getTexture("sky")), {0, 0, getScreenSize().x, getScreenSize().y}, {0, 0}, 0, fadeColor(skyColorNight, skyColorDay, t));
}

void drawParallaxTexture(const Texture &texture, float &progress, float speed, float currentTime, bool background) {
   Vector2 screenSize = getScreenSize();
   progress -= speed * GetFrameTime();
   
   if (progress <= -screenSize.x) {
      progress = 0.f;
   }
   if (progress > 0.f) {
      progress = -screenSize.x;
   }

   Color colorDay = (background ? backgroundTintDay : foregroundTintDay);
   Color colorNight = (background ? backgroundTintNight : foregroundTintNight);
   Color color = fadeColor(colorNight, colorDay, getFadeStrengthBasedOnTime(currentTime));

   drawTextureNoOrigin(texture, {progress, 0}, screenSize, color);
   drawTextureNoOrigin(texture, {screenSize.x + progress, 0}, screenSize, color);
}

Texture& getRandomBackground() {
   return getTexture(random(backgroundTextures));
}

Texture& getRandomForeground() {
   return getTexture(random(foregroundTextures));
}

// Sun functions

void drawSunAndMoon(float &currentTime, float speed, int &moonPhase, bool &isNight) {
   Vector2 screenSize = getScreenSize();
   currentTime = std::fmod(currentTime + speed * GetFrameTime(), 360.0f);

   Vector2 origin = getOrigin(screenSize);
   Vector2 position = {origin.x, screenSize.y};

   bool wasNight = isNight;
   isNight = (currentTime >= 180.0f);

   // Change moon phases
   if (wasNight && !isNight) {
      moonPhase = (moonPhase + 1) % moonPhaseCount;
   }

   // Draw either moon or sun based on the time
   if (isNight) {
      Texture &texture = getTexture("moon");
      DrawTexturePro(texture, {(float)moonPhase * textureSize * 2.0f, 0.0f, (float)textureSize * 2.0f, (float)texture.height}, {position.x, position.y, moonSize.x, moonSize.y}, origin, currentTime - 180.0f, WHITE);
   } else {
      Texture &texture = getTexture("sun");
      DrawTexturePro(texture, getBox(texture), {position.x, position.y, sunSize.x, sunSize.y}, origin, currentTime, WHITE);
   }
}
