#include "game/state.hpp"
#include "mngr/particle.hpp"
#include "mngr/resource.hpp"
#include "util/position.hpp"
#include "util/random.hpp"
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
         {random(minimum.position.x, maximum.position.x), random(minimum.position.y, maximum.position.y)},
         {random(minimum.velocity.x, maximum.velocity.x), random(minimum.velocity.y, maximum.velocity.y)},
         {random(minimum.size.x, maximum.size.x), random(minimum.size.y, maximum.size.y)},
         {random(minimum.sizeVelocity.x, maximum.sizeVelocity.x), random(minimum.sizeVelocity.y, maximum.sizeVelocity.y)},
         random(minimum.rotation, maximum.rotation),
         random(minimum.rotationVelocity, maximum.rotationVelocity),
         random(minimum.lifetime, maximum.lifetime),
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
