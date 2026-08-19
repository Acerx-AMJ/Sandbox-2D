#include "ui/bar.hpp"
#include "SRU/assets.hpp"
#include "SRU/render.hpp"
#include <raymath.h>

void Bar::init(Texture2D texture, Vector2 origin, Color foregroundTint, Color backgroundTint) {
   this->texture = texture;
   this->origin = origin;
   this->foregroundTint = foregroundTint;
   this->backgroundTint = backgroundTint;
}

void Bar::update(float alpha) {
   progressInterpolation = Lerp(progressInterpolation, progress, alpha * 10.0f);
}

void Bar::render() {
   if (texture.id == 0) return;
   drawTexture(texture, rect, origin, backgroundTint);

   Shader &clipShader = getShader("clip");
   int progressLocation = GetShaderLocation(clipShader, "progress");
   SetShaderValue(clipShader, progressLocation, &progressInterpolation, SHADER_UNIFORM_FLOAT);

   BeginShaderMode(clipShader);
      drawTexture(texture, rect, origin, foregroundTint);
   EndShaderMode();
}
