#pragma once

#include <vector>

#include <random>
#include "common.hpp"
#include "tiny_ecs.hpp"
#include "game_components.hpp"

struct Projectile {
	static ECS::Entity createProjectile(vec2 position, float radius, vec2 velocity, Team team, float damage, vec3 color);
	static ECS::Entity createMissile(vec2 position, std::string key, ECS::Entity* target, float radius, float velocity, Team team, float damage, float angular_momentum, float weight, bool explode);
};
