#pragma once

#include "common.hpp"
#include "game_components.hpp"
#include "tiny_ecs.hpp"

struct Unit 
{
	// Creates all the associated render resources and default transform
	static ECS::Entity createUnit(vec2 boardPosition, BoardInfo bi, WorldPosition wp, Team t, UnitType ut, CombatStats cs);
	static int damageUnit(ECS::Entity e, int amount);
	bool alive = true;
};

struct Allies {
};
struct Enemies {
};

