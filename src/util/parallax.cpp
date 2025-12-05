#include "util/parallax.hpp"
#include "mngr/resource.hpp"
#include "util/position.hpp"
#include "util/random.hpp"
#include "util/render.hpp"
#include <string>
#include <vector>

static inline std::vector<std::string> backgroundTextures {
   "mountains1", "mountains2", "mountains3"
};

static inline std::vector<std::string> foregroundTextures {
   "bg_trees1", "bg_trees2", "bg_trees3"
};

void drawParallaxTexture(const Texture &texture, float &progress, float speed) {
   Vector2 screenSize = getScreenSize();
   progress -= speed * GetFrameTime();
   
   if (progress <= -screenSize.x) {
      progress = 0.f;
   }

   drawTextureNoOrigin(texture, {progress, 0}, screenSize);
   drawTextureNoOrigin(texture, {screenSize.x + progress, 0}, screenSize);
}

Texture& getRandomBackground() {
   return getTexture(backgroundTextures[random(0, backgroundTextures.size() - 1)]);
}

Texture& getRandomForeground() {
   return getTexture(foregroundTextures[random(0, foregroundTextures.size() - 1)]);
}
