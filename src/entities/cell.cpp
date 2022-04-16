#include "game_components.hpp"
#include "cell.hpp"
#include "render.hpp"
#include "tiny_ecs.hpp"

ECS::Entity Cell::createCell(vec2 bp, vec2 wp, vec2 cellSize, CellType ct)
{
	auto entity = ECS::Entity();

	std::string key = "cell <" + std::to_string(wp.x) + ", " + std::to_string(wp.y) + ">";
	//Sets color of Cell based on what type it is (Can change later, just for clarity when developing)
	std::string spriteStr = "cell.png";
	switch (ct) {
	case enemyCell:
		spriteStr = "enemy" + spriteStr;
		break;
	case allyCell:
		spriteStr = "ally" + spriteStr;
		break;
	case benchCell:
		spriteStr = "bench" + spriteStr;
		break;
	case shopCell:
		spriteStr = "shop" + spriteStr;
		break;
	}
	ShadedMesh& resource = cache_resource(key);
	if (resource.texture.size == ivec2(0, 0))
	{
		resource = ShadedMesh();
		RenderSystem::createSprite(resource, textures_path(spriteStr), "textured");
	}

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	ECS::registry<ShadedMeshRef>.emplace(entity, resource);

	// Setting WorldPosition component
	WorldPosition& worldPosition = ECS::registry<WorldPosition>.emplace(entity);
	worldPosition.position = wp;

	// Setting initial motion values
	Motion& motion = ECS::registry<Motion>.emplace(entity);
	motion.position = wp;
	motion.scale = cellSize;

	CellStruct& cts = ECS::registry<CellStruct>.emplace(entity);
	cts.ct = ct;
	cts.isBench = ct == benchCell;
	BoardPosition& boardPos = ECS::registry<BoardPosition>.emplace(entity);
	boardPos.startPos = bp;
	if (cts.isBench) {
		boardPos.isBench = true;
	}

	return entity;
}

void Cell::hide(ECS::Entity entity)
{
	auto& cellMot = ECS::registry<Motion>.get(entity);
	cellMot.is_in_foreground = false;
}

void Cell::show(ECS::Entity entity)
{
	auto& cellMot = ECS::registry<Motion>.get(entity);
	cellMot.is_in_foreground = true;
}
