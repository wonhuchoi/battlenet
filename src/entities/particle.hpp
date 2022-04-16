#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "game_components.hpp"


struct Gravity {
  // Gravitational acceleration in pixels/ms^2
  float strength = 0;
};

struct Particle
{ 
private:
	std::string texture_key;
	Gravity grv;
public:
	// Creates a particle with random motion
  static ECS::Entity createParticle(vec2 position, vec2 scale, std::string fileName, float minLifetime, float maxLifetime, float gravity, float minVelX, float maxVelX, float minVelY, float maxVelY);
  static void createParticles(int amt, vec2 position, vec2 scale, std::string fileName, float minLifetime, float maxLifetime, float gravity, float minVelX, float maxVelX, float minVelY, float maxVelY);

  vec2 position;
  vec2 velocity;
  int lifetime = 0;
};
