#ifndef MNGR_PARTICLE
#define MNGR_PARTICLE

#include <raylib.h>

// Particle

struct Particle {
   Texture2D *texture;
   Vector2 position, velocity;
   Vector2 size, sizeVelocity;
   float rotation = 0.0f;
   float rotationVelocity = 0.0f;
   float lifetime = 0.0f;
   float age = 0.0f;
};

// Update/render particles

void updateParticles();
void renderParticles();

// Spawn particles

void spawnParticles(const Particle &example, int count);
void spawnParticles(const Particle &minimum, const Particle &maximum, int count);

// Particle presets

void spawnDeathParticles(const Vector2 &position);

#endif
