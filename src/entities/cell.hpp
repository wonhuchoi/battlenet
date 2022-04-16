#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"

struct Cell
{
	// Creates all the associated render resources and default transform
	static ECS::Entity createCell(vec2 boardPosition, vec2 worldPosition, vec2 cellSize, CellType ct);
	static void hide(ECS::Entity entity);
	static void show(ECS::Entity entity);
};
