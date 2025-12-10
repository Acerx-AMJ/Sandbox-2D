#include "util/parallax.hpp"
#include "mngr/resource.hpp"
#include "util/config.hpp"
#include "util/position.hpp"
#include "util/random.hpp"
#include "util/render.hpp"
#include <string>
#include <vector>

// Constants

static inline std::vector<std::string> backgroundTextures {
   "mountains1", "mountains2", "mountains3", "mountains4"
};

static inline std::vector<std::string> foregroundTextures {
   "bg_trees1", "bg_trees2", "bg_trees3", "bg_trees4"
};

// Parallax functions

void drawParallaxTexture(const Texture &texture, float &progress, float speed, bool background) {
   Vector2 screenSize = getScreenSize();
   progress -= speed * GetFrameTime();
   
   if (progress <= -screenSize.x) {
      progress = 0.f;
   }
   if (progress > 0.f) {
      progress = -screenSize.x;
   }

   Color color = (background ? backgroundTint : foregroundTint); 
   drawTextureNoOrigin(texture, {progress, 0}, screenSize, color);
   drawTextureNoOrigin(texture, {screenSize.x + progress, 0}, screenSize, color);
}

Texture& getRandomBackground() {
   return getTexture(random(backgroundTextures));
}

Texture& getRandomForeground() {
   return getTexture(random(foregroundTextures));
}
