#include "ui/bar.hpp"
#include "SRU/assets.hpp"
#include "SRU/render.hpp"
#include <raymath.h>

void Bar::update(float alpha) {
   progressInterpolation = Lerp(progressInterpolation, progress, alpha * 10.0f);
}

void Bar::render() const {
   if (!texture) {
      return;
   }
   drawTextureCentered(*texture, {rectangle.x, rectangle.y}, {rectangle.width, rectangle.height}, backgroundTint);

   Shader &clipShader = getShader("clip");
   int progressLocation = GetShaderLocation(clipShader, "progress");
   SetShaderValue(clipShader, progressLocation, &progressInterpolation, SHADER_UNIFORM_FLOAT);

   BeginShaderMode(clipShader);
      drawTextureCentered(*texture, {rectangle.x, rectangle.y}, {rectangle.width, rectangle.height}, foregroundTint);
   EndShaderMode();
}
