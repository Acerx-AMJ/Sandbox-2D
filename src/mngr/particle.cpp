#include "game/state.hpp"
#include "mngr/particle.hpp"
#include "util/position.hpp"
#include "SRU/assets.hpp"
#include "SRU/random.hpp"
#include <raymath.h>
#include <algorithm>
#include <vector>

// Global variables

std::vector<Particle> particles;

// Update particles

void updateParticles() {
   for (Particle &particle: particles) {
      particle.age += fixedUpdateDT;
      particle.position = Vector2Add(particle.position, Vector2Scale(particle.velocity, fixedUpdateDT));
      particle.size = Vector2Add(particle.size, Vector2Scale(particle.sizeVelocity, fixedUpdateDT));
      particle.rotation += particle.rotationVelocity * fixedUpdateDT;
   }

   particles.erase(std::remove_if(particles.begin(), particles.end(), [](const Particle &p) -> bool {
      return p.age >= p.lifetime;
   }), particles.end());
}

// Render particles

void renderParticles() {
   for (const Particle &particle: particles) {
      DrawTexturePro(*particle.texture, getBox(*particle.texture), {particle.position.x, particle.position.y, particle.size.x, particle.size.y}, getOrigin(particle.size), particle.rotation, Fade(WHITE, 1.0f - particle.age / particle.lifetime));
   }
}

// Spawn particles

void spawnParticles(const Particle &example, int count) {
   particles.reserve(particles.size() + count);
   for (int i = 0; i < count; ++i)
      particles.push_back(example);
}

void spawnParticles(const Particle &minimum, const Particle &maximum, int count) {
   particles.reserve(particles.size() + count);
   for (int i = 0; i < count; ++i) {
      Particle particle = {
         minimum.texture,
         randomV2(minimum.position, maximum.position),
         randomV2(minimum.velocity, maximum.velocity),
         randomV2(minimum.size, maximum.size),
         randomV2(minimum.sizeVelocity, maximum.sizeVelocity),
         randomFloat(minimum.rotation, maximum.rotation),
         randomFloat(minimum.rotationVelocity, maximum.rotationVelocity),
         randomFloat(minimum.lifetime, maximum.lifetime),
         0.0f, // age
      };
      particles.push_back(particle);
   }
}

// Particle presets

void spawnDeathParticles(const Vector2 &position) {
   spawnParticles({
      &getTexture("dust"),
      Vector2Subtract(position, {1.0f, 1.0f}),
      {-1.3, -1.3},
      {0.5, 0.5},
      {0.99, 0.99},
      0.0f,
      -720.0f,
      0.5f
   }, {
      nullptr,
      Vector2Add(position, {1.0f, 1.0f}),
      {1.3, 1.3},
      {0.9, 0.9},
      {1.01, 1.01},
      360.0f,
      720.0f,
      1.3f
   }, 8);
}
