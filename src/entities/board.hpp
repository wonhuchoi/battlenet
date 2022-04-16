#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "unit.hpp"

struct Board 
{
	// Creates all the associated render resources and default transform
	// Boards are all of the same dimensions/size so we don't need those params
	static ECS::Entity createBoard(vec2 worldPosition);
	static void hide(ECS::Entity board);
	static void show(ECS::Entity board);
	static ECS::Entity* getBench(ECS::Entity board);
	static ECS::Entity getCell(ECS::Entity board, vec2 pos);
	static ECS::Entity** getCells(ECS::Entity board);
};

