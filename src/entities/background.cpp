#include "game_components.hpp"
#include "background.hpp"
#include "render.hpp"
#include "tiny_ecs.hpp"

ECS::Entity Background::creator(std::string key) {
	return creator(key, "textured");
}

ECS::Entity Background::creator(std::string key, std::string shader) {
	auto entity = ECS::Entity();
	ShadedMesh& resource = cache_resource(key);
	if (resource.mesh.vertices.size() == 0)
	{
		resource = ShadedMesh();
		RenderSystem::createSprite(resource, textures_path(key + ".jpg"), shader);
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

	ECS::registry<Background>.emplace(entity);

	return entity;
}

ECS::Entity Background::createPlanBG()
{
  return creator("plan");
}

ECS::Entity Background::createPlanBG2()
{
	return creator("plan2");
}

ECS::Entity Background::createPlanBG3()
{
	return creator("plan3");
}

ECS::Entity Background::createBatBG()
{
	return creator("bat");
}

ECS::Entity Background::createBatBG2()
{
	return creator("bat2");
}

ECS::Entity Background::createBatBG3()
{
	return creator("bat3");
}

ECS::Entity Background::createStart()
{
	return creator("start");
}

ECS::Entity Background::createStart2()
{
	return creator("start2");
}

ECS::Entity Background::createStart3()
{
	return creator("start3");
}

ECS::Entity Background::createDefeat()
{
	return creator("defeat", "glitchy");
}

ECS::Entity Background::createVictory()
{
	return creator("victory", "glitchy");
}

void Background::clear()
{
	while (ECS::registry<Background>.entities.size() > 0)
		ECS::ContainerInterface::remove_all_components_of(ECS::registry<Background>.entities.back());
}