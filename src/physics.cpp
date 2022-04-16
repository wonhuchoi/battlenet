// internal
#include "physics.hpp"
#include "game_components.hpp"
#include "item.hpp"
#include "particle.hpp"
#include "player.hpp"
#include "tiny_ecs.hpp"
#include "debug.hpp"
#include "util.hpp"
#include "unit.hpp"
#include "common.hpp"
#include "projectile.hpp"
#include <iterator>
#include <iostream>
#include "render_components.hpp"
#include "world.hpp"


// Returns the local bounding coordinates scaled by the current size of the entity 
vec2 get_bounding_box(const Motion& motion)
{
	// fabs is to avoid negative scale due to the facing direction.
	return { abs(motion.scale.x), abs(motion.scale.y) };
}

// This is a SUPER APPROXIMATE check that puts a circle around the bounding boxes and sees
// if the center point of either object is inside the other's bounding-box-circle. You don't
// need to try to use this technique.
bool collides(const Motion& motion1, const Motion& motion2)
{
	auto dp = motion1.position - motion2.position;
	float dist_squared = dot(dp,dp);
	float other_r = std::sqrt(std::pow(get_bounding_box(motion1).x/2.0f, 2.f) + std::pow(get_bounding_box(motion1).y/2.0f, 2.f));
	float my_r = std::sqrt(std::pow(get_bounding_box(motion2).x/2.0f, 2.f) + std::pow(get_bounding_box(motion2).y/2.0f, 2.f));
	float r = max(other_r, my_r);
	if (dist_squared < r * r)
		return true;
	return false;
}

void PhysicsSystem::step(float elapsed_ms, vec2 window_size_in_game_units, int worldState)
{
	float G = 20;
	for (auto& entity : ECS::registry<Missile>.entities)
	{
		if (ECS::registry<BattlePhase>.entities.size() > 0) {
			Motion& m = ECS::registry<Motion>.get(entity);
			Missile& missile = ECS::registry<Missile>.get(entity);
			if (ECS::registry<CombatStats>.has(*missile.target) && ECS::registry<CombatStats>.get(*missile.target).currentHp <= 0)
				continue;
			Motion& other = ECS::registry<Motion>.get(*missile.target);
			vec2 dir = glm::normalize(other.position - m.position);
			vec2 default_vec = vec2(0.f, -1.f);
			m.angle = acos(glm::dot(dir, default_vec));
			m.velocity += G * (dir * missile.weight) / pow(glm::length(other.position - m.position) / 100, 2.f);
		}
	}
	// Move entities based on how much time has passed, this is to (partially) avoid
	// having entities move at different speed based on the machine.
	for (auto& e : ECS::registry<Motion>.entities)
	{
		Motion& motion = ECS::registry<Motion>.get(e);
		float speed = motion.speed;
		bool unit_in_vic_def_screen = ECS::registry<Unit>.has(e) && (worldState == WorldSystem::WORLD_VICTORY || worldState == WorldSystem::WORLD_DEFEAT);
		// Handle bezier curve velocity if entity has it and it is not a unit in vic/def screen.
		if (ECS::registry<BezierCurve>.has(e) && !unit_in_vic_def_screen) {
			BezierCurve& bc = ECS::registry<BezierCurve>.get(e);
			float totalDist = util::get_distance(motion.initialPosition, motion.destination);
			float elapsedDist = util::get_distance(motion.initialPosition, motion.position);
			float t = elapsedDist/totalDist;
			speed = bc.motionScale * util::deCasteljau_bezier(bc.controlPoints, t).y;
			// Don't use bezier curve motion on units that are following player in victory/defeat animation
		}
		float step_seconds = 1.0f * (elapsed_ms / 1000.f);


		if (ECS::registry<Particle>.has(e)) {
			Particle& p = ECS::registry<Particle>.get(e);
			p.lifetime -= elapsed_ms;
			// Remove the particle if its lifetime has fully elapsed
			if (p.lifetime <= 0) {
				ECS::ContainerInterface::remove_all_components_of(e);
			} else {
				Gravity g = ECS::registry<Gravity>.get(e);
				motion.velocity.y += g.strength * elapsed_ms; 
			}
		}

		else if (motion.is_moving) {
			// condition could be better - further research could be done
			double a = motion.angle;
			if (a > 0) {
				motion.velocity = vec2(cos(a), sin(a)) * speed;
			} else {
				motion.velocity = util::normalize(motion.destination - motion.position) * speed;
			}
			vec2 next_dest = motion.position + (motion.velocity * step_seconds);
			if (util::get_distance(motion.position, motion.destination) <= util::get_distance(motion.position, next_dest)) {
				motion.is_moving = false;
				motion.velocity = { 0,0 };
				motion.position = motion.destination;
				motion.initialPosition = motion.destination;
				//could have callback here for when finished moving
			}
		}

		motion.position = motion.position + (motion.velocity * step_seconds);
		auto boundaries = motion.boundaries;
		int x_min = boundaries[0]+motion.scale.x/2, y_min = boundaries[1] + motion.scale.y / 2, x_max = boundaries[2]- motion.scale.x / 2, y_max = boundaries[3] - motion.scale.y / 2;

		bool out_of_bounds = false;
		if (motion.position.x < x_min) {
			motion.position.x = x_min;
			out_of_bounds = true;
		}
		if (motion.position.y < y_min) {
			motion.position.y = y_min;
			out_of_bounds = true;
		}
		if (motion.position.x > x_max) {
			motion.position.x = x_max;
			out_of_bounds = true;
		}
		if (motion.position.y > y_max) {
			motion.position.y = y_max;
			out_of_bounds = true;
		}
		if (out_of_bounds) {
			motion.is_moving = false;
			motion.velocity = vec2(0, 0);
		}

	}

	//remove projectiles which are outside their boundaries
	for (auto& p : ECS::registry<Projectile>.entities) {
		auto& motion = ECS::registry<Motion>.get(p);
		auto boundaries = motion.boundaries;
		int x_min = boundaries[0] + motion.scale.x / 2, y_min = boundaries[1] + motion.scale.y / 2, x_max = boundaries[2] - motion.scale.x / 2, y_max = boundaries[3] - motion.scale.y / 2;
		if ((motion.position.x <= x_min) || (motion.position.x >= x_max) || (motion.position.y <= y_min) || (motion.position.y >= y_max)) {
			ECS::ContainerInterface::remove_all_components_of(p);
		}
	}

	// Visualization for debugging the position and scale of objects
	if (DebugSystem::in_debug_mode)
	{
		for (auto& motion : ECS::registry<Motion>.components)
		{
			if (motion.is_moving) {
				DebugSystem::createLineFromPoints(motion.position, motion.destination, vec3( 1.0,1.0,1.0 ));
			}
		}
		//draw convex hull
		for (auto& u : ECS::registry<Unit>.entities) {
			std::vector<vec2> oldHull = *ECS::registry<HullMeshRef>.get(u).reference_to_cache;
			Motion motion = ECS::registry<Motion>.get(u);
			std::vector<vec2> hull;
			hull = util::transformHull(oldHull, motion);
			int n = hull.size();
			for (int i = 0; i < n; i++) {
				DebugSystem::createLineFromPoints(hull[i], hull[(i + 1) % n], vec3(1.0, 1.0, 1.0));
			}
		}
	}

	std::vector<ECS::Entity> allAllies = ECS::registry<Allies>.entities;
	std::vector<ECS::Entity> allEnemies = ECS::registry<Enemies>.entities;

	std::vector<ECS::Entity> allies;
	std::vector<ECS::Entity> enemies;

	// copy only alive units:
	std::copy_if(allAllies.begin(), allAllies.end(), std::back_inserter(allies), [](ECS::Entity e) {return ECS::registry<CombatStats>.get(e).currentHp > 0; });
	std::copy_if(allEnemies.begin(), allEnemies.end(), std::back_inserter(enemies), [](ECS::Entity e) {return ECS::registry<CombatStats>.get(e).currentHp > 0; });

	util::createHPBars(enemies, red);
	util::createHPBars(allies, green);
	util::createManaBars(enemies, blue);
	util::createManaBars(allies, blue);

	// Check for collisions between players and items
	auto& items = ECS::registry<Item>.entities;
	auto& players = ECS::registry<Player>.entities;
	for (unsigned int i=0; i < players.size(); i++) {
		auto& player = ECS::registry<Player>.entities[i];
		Motion& playerMotion = ECS::registry<Motion>.get(player);
		// for (auto [i, motion_i] : enumerate(motion_container.components)) // in c++ 17 we will be able to do this instead of the next three lines
		for (unsigned int i=0; i<items.size(); i++)
		{
			ECS::Entity entity_i = items[i];
			Motion& motion_i = ECS::registry<Motion>.get(entity_i);
			if (collides(motion_i, playerMotion))
			{
				// Create a collision event
				// Note, we are abusing the ECS system a bit in that we potentially insert muliple collisions for the same entity, hence, emplace_with_duplicates
				ECS::registry<Collision>.emplace_with_duplicates(entity_i, player);
				ECS::registry<Collision>.emplace_with_duplicates(player, entity_i);
			}
		}
	}

	// Check for collisions between projectiles and units
	auto& projectiles = ECS::registry<Projectile>.entities;
	auto& units = ECS::registry<Unit>.entities;
	for (unsigned int i = 0; i < projectiles.size(); i++) {
		for (unsigned int j = 0; j < units.size(); j++) {
			auto& unit = ECS::registry<Unit>.entities[j];
			Motion& unitMotion = ECS::registry<Motion>.get(unit);

			ECS::Entity projectile_i = projectiles[i];
			Motion& motion_i = ECS::registry<Motion>.get(projectile_i);
			// can have closer bound TODO if slow or lots of projectiles
			if (distance(motion_i.position, unitMotion.position) < max(motion_i.scale.x, motion_i.scale.y) + max(unitMotion.scale.x, unitMotion.scale.y))
			{
				
				// Create a collision event
				// Note, we are abusing the ECS system a bit in that we potentially insert muliple collisions for the same entity, hence, emplace_with_duplicates
				ECS::registry<Collision>.emplace_with_duplicates(projectile_i, unit);

			}
		}
	}
	
}

PhysicsSystem::Collision::Collision(ECS::Entity& other)
{
	this->other = other;
}
