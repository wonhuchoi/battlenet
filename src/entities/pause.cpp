#include "game_components.hpp"
#include "pause.hpp"
#include "render.hpp"
#include "tiny_ecs.hpp"

ECS::Entity Pause::createPause()
{
	auto entity = ECS::Entity();
	std::string key = "pause";
	ShadedMesh& resource = cache_resource(key);
	if (resource.texture.size == ivec2(0, 0))
	{
		resource = ShadedMesh();
		RenderSystem::createSprite(resource, textures_path("pause.jpg"), "textured");
	}
	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	ECS::registry<ShadedMeshRef>.emplace(entity, resource);

	// Setting WorldPosition component
	WorldPosition& worldPosition = ECS::registry<WorldPosition>.emplace(entity);
	vec2 pos = { worldPosition.worldSize.x / 2, worldPosition.worldSize.y / 2 };
	worldPosition.position = pos;

	// Setting initial motion values
	Motion& motion = ECS::registry<Motion>.emplace(entity);
	motion.position = pos;
	motion.scale = worldPosition.worldSize;
	motion.is_in_foreground = false;

	ECS::registry<Pause>.emplace(entity);

	return entity;
}

void Pause::switchPause(ECS::Entity entity)
{
	std::string key = "pause";
	ShadedMesh& resource = cache_resource(key);
	if (resource.texture.size == ivec2(0,0))
	{
		resource = ShadedMesh();
		RenderSystem::createSprite(resource, textures_path("pause.jpg"), "textured");
	}
	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	ECS::registry<ShadedMeshRef>.emplace(entity, resource);
}