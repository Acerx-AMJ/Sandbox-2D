#include "ui/bar.hpp"
#include "util/render.hpp"
#include "SRU/assets.hpp"
#include <raymath.h>

void Bar::update(float alpha) {
   progressInterpolation = Lerp(progressInterpolation, progress, alpha * 10.0f);
}

void Bar::render() const {
   if (!texture) {
      return;
   }
   drawTexture(*texture, {rectangle.x, rectangle.y}, {rectangle.width, rectangle.height}, 0, backgroundTint);

   Shader &clipShader = getShader("clip");
   int progressLocation = GetShaderLocation(clipShader, "progress");
   SetShaderValue(clipShader, progressLocation, &progressInterpolation, SHADER_UNIFORM_FLOAT);

   BeginShaderMode(clipShader);
      drawTexture(*texture, {rectangle.x, rectangle.y}, {rectangle.width, rectangle.height}, 0, foregroundTint);
   EndShaderMode();
}
