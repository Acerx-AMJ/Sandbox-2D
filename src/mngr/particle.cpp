#include "mngr/particle.hpp"
#include "SRU/assets.hpp"
#include "SRU/util.hpp"

ParticleID deathParticles = 0;

void initParticles() {
   deathParticles = pushParticleConfig(ParticleConfig{&getTexture("dust"), {V2(), V2(-1.3f), V2(), V2(0.5f), 0.99f, 0.0f, -720.0f, 0.0f, 0.5f}, {V2(), V2(1.3f), V2(), V2(0.9f), 1.01f, 360.0f, 720.0f, 0.0f, 1.3f}, 8});
}
