// Header
#include "projectile.hpp"
#include "tiny_ecs.hpp"
#include "render.hpp"

#include <cmath>
#include <iostream>

#include "render_components.hpp"

void createProjectileMesh(ShadedMesh& resource, vec3 color) {
	// create a procedural circle
	std::vector<ColoredVertex> vertices;
	std::vector<uint16_t> indices;

	constexpr float z = -0.1f;
	constexpr int NUM_TRIANGLES = 62;

	for (int i = 0; i < NUM_TRIANGLES; i++) {
		// Point on the circle
		ColoredVertex v;
		v.position = { 0.5f * std::cos(float(PI) * 2.0f * static_cast<float>(i) / NUM_TRIANGLES),
					   0.5f * std::sin(float(PI) * 2.0f * static_cast<float>(i) / NUM_TRIANGLES),
					   z };
		v.color = color;
		vertices.push_back(v);

		// Point on the circle ahead by on eposition in counter-clockwise direction
		v.position = { 0.5f * std::cos(float(PI) * 2.0f * static_cast<float>(i + 1) / NUM_TRIANGLES),
					   0.5f * std::sin(float(PI) * 2.0f * static_cast<float>(i + 1) / NUM_TRIANGLES),
					   z };
		v.color = color;
		vertices.push_back(v);

		// Circle center
		v.position = { 0, 0, z };
		v.color = color;
		vertices.push_back(v);

		// Indices
		// Note, one could create a mesh with less redundant vertices
		indices.push_back(static_cast<uint16_t>(i * 3 + 0));
		indices.push_back(static_cast<uint16_t>(i * 3 + 1));
		indices.push_back(static_cast<uint16_t>(i * 3 + 2));
	}
	resource.mesh.vertices = vertices;
	resource.mesh.vertex_indices = indices;
	RenderSystem::createColoredMesh(resource, "colored_mesh");
}

ECS::Entity Projectile::createProjectile(vec2 position, float radius, vec2 velocity, Team team, float damage, vec3 color)
{
	auto entity = ECS::Entity();

	std::string key = "projectile" + std::to_string(color[0]) + std::to_string(color[1]) + std::to_string(color[2]);
	ShadedMesh& resource = cache_resource(key);
	if (resource.effect.program.resource == 0)
	{
		createProjectileMesh(resource, color);
	}

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	ECS::registry<ShadedMeshRef>.emplace(entity, resource);

	// Create motion
	auto& motion = ECS::registry<Motion>.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = velocity;
	motion.position = position;
	motion.scale = vec2(radius*2, radius*2);
	
	BoardInfo bi = ECS::registry<BoardInfo>.components[0];
	auto& b = ECS::registry<BoardInfo>.entities[0];
	WorldPosition wp = ECS::registry<WorldPosition>.get(b);
	vec2 min_pos = boardPosToWorldPos(vec2(0,0), bi, wp) - wp.position + vec2(bi.cellSize.x/2, bi.cellSize.y/2);
	vec2 max_pos = boardPosToWorldPos(vec2(bi.boardSize.x, bi.boardSize.y), bi, wp) - wp.position + vec2(bi.cellSize.x / 2, bi.cellSize.y / 2);
	motion.boundaries =vec4(min_pos.x, min_pos.y, max_pos.x, max_pos.y);
	motion.is_in_foreground = true;
	auto& teamaffiliation = ECS::registry<TeamAffiliation>.emplace(entity);
	teamaffiliation.t = team;

	auto& combatstats = ECS::registry<CombatStats>.emplace(entity);
	combatstats.attack = damage;
	ECS::registry<Projectile>.emplace(entity);

	ECS::registry<Active>.emplace(entity);

	return entity;
}


ECS::Entity Projectile::createMissile(vec2 position, std::string key, ECS::Entity* target, float radius, float velocity, Team team, float damage, float angular_momentum, float weight, bool explode)
{
	auto entity = ECS::Entity();

	ShadedMesh& resource = cache_resource(key);
	std::vector<vec2>& hull_resource = cache_hull_resource(key);
	if (resource.effect.program.resource == 0)
		if (key == "projectile") {
			createProjectileMesh(resource, vec3(1,1,1));
		}
		else {
			RenderSystem::createSprite(resource, textures_path(key + ".png"), "textured", hull_resource, true);
		}
	if (hull_resource.size() == 0)
		RenderSystem::createHull(hull_resource, textures_path(key + ".png"));
	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	ECS::registry<ShadedMeshRef>.emplace(entity, resource);

	// Create motion
	auto& motion = ECS::registry<Motion>.emplace(entity);
	motion.angle = 0.f;	
	float angle = ((double)rand() / (RAND_MAX)) * 2 * PI;
	float velocity_magnitude = velocity;
	mat2 mtx = mat2(cos(angle), -sin(angle), sin(angle), cos(angle));
	vec2 dir = vec2(1, 0) * mtx;
	vec2 perp = vec2(-dir.y, dir.x);
	motion.velocity = velocity_magnitude * dir + perp * velocity_magnitude * angular_momentum;
	motion.position = position;
	motion.scale = vec2(radius * 3, radius * 3);

	BoardInfo bi = ECS::registry<BoardInfo>.components[0];
	auto& b = ECS::registry<BoardInfo>.entities[0];
	WorldPosition wp = ECS::registry<WorldPosition>.get(b);
	vec2 min_pos = boardPosToWorldPos(vec2(0, 0), bi, wp) - wp.position + vec2(bi.cellSize.x / 2, bi.cellSize.y / 2);
	vec2 max_pos = boardPosToWorldPos(vec2(bi.boardSize.x, bi.boardSize.y), bi, wp) - wp.position + vec2(bi.cellSize.x / 2, bi.cellSize.y / 2);
	motion.boundaries = vec4(min_pos.x, min_pos.y, max_pos.x, max_pos.y);
	motion.boundaries = vec4(motion.boundaries[0] - 200, motion.boundaries[1] - 200, motion.boundaries[2] + 200, motion.boundaries[3] + 200);
	motion.is_in_foreground = true;
	auto& teamaffiliation = ECS::registry<TeamAffiliation>.emplace(entity);
	teamaffiliation.t = team;

	auto& combatstats = ECS::registry<CombatStats>.emplace(entity);
	combatstats.attack = damage;
	ECS::registry<Projectile>.emplace(entity);

	ECS::registry<Active>.emplace(entity);
	Missile& missile = ECS::registry<Missile>.emplace(entity);
	if(explode == true)
		ECS::registry<Explode>.emplace(entity);
	missile.target = target;
	missile.weight = weight;
	missile.angular_momentum = 200.;

	return entity;
}
