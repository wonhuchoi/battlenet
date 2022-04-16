#include "particle.hpp"
#include "game_components.hpp"
#include "tiny_ecs.hpp"
#include "render.hpp"
#include <random>
#include <iostream>

// Creates a particle with randomized direction and velocity.  
// These will be affected by gravity in physics.cpp and removed after their lifetime (in ms)
ECS::Entity Particle::createParticle(vec2 position, vec2 scale, std::string key, float minLifetime, float maxLifetime, float gravity, float minVelX, float maxVelX, float minVelY, float maxVelY) {
  auto entity = ECS::Entity();

  ShadedMesh &resource = cache_resource(key);
	if (resource.effect.program.resource == 0) {
		RenderSystem::createSprite(resource, textures_path(key + ".png"), "textured");
  }
  
  ECS::registry<ShadedMeshRef>.emplace(entity, resource);

  Motion& motion = ECS::registry<Motion>.emplace(entity);
  std::random_device rd;  //Will be used to obtain a seed for the random number engine
  std::mt19937 rng(rd()); //Standard mersenne_twister_engine seeded with rd()
  std::uniform_real_distribution<float> x_uniform_dist(minVelX, maxVelX);
  std::uniform_real_distribution<float> y_uniform_dist(minVelY, maxVelY);
  std::uniform_real_distribution<float> lifetime_uniform_dist(minLifetime, maxLifetime);
  
  motion.position = position;
  motion.scale = scale;
  motion.is_moving = true;
  motion.is_in_foreground = true;
  motion.velocity.x = x_uniform_dist(rng);
  motion.velocity.y = y_uniform_dist(rng);

  Gravity& grv = ECS::registry<Gravity>.emplace(entity);
  grv.strength = gravity;
  
  Particle& part = ECS::registry<Particle>.emplace(entity);
  part.lifetime = lifetime_uniform_dist(rng);
  return entity;
}

void Particle::createParticles(int amt, vec2 position, vec2 scale, std::string key, float minLifetime, float maxLifetime, float gravity, float minVelX, float maxVelX, float minVelY, float maxVelY) {
  for (int i = 0; i < amt; i++) {
    createParticle(position, scale, key, minLifetime, maxLifetime, gravity, minVelX, maxVelX, minVelY, maxVelY);
  }
}
