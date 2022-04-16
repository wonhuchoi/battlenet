// Header
#include "item.hpp"
#include "render.hpp"

ECS::Entity Item::createItem(vec2 position)
{
	auto entity = ECS::Entity();

	// Create rendering primitives
	std::string key = "item";
	ShadedMesh& resource = cache_resource(key);
	if (resource.effect.program.resource == 0)
		AnimationSystem::load(resource, textures_path("coin_sprite_sheet.png"), "spritesheet", key);

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	ECS::registry<ShadedMeshRef>.emplace(entity, resource);

	// Initialize the motion
	auto& motion = ECS::registry<Motion>.emplace(entity);

	motion.position = position;
	motion.angle = 0.f;
	motion.velocity = {0.f, 0.f};
	motion.scale = { 60.f, 60.f };

	ECS::registry<Item>.emplace(entity);
	ECS::registry<Animated>.emplace(entity);

	return entity;
}
