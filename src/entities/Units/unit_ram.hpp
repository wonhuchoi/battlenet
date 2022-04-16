#pragma once

#include "common.hpp"
#include "game_components.hpp"
#include "tiny_ecs.hpp"
#include "ai.hpp"

struct UnitRAM
{
	// Creates all the associated render resources and default transform
	static ECS::Entity createUnitRAM(vec2 boardPosition, BoardInfo bi, WorldPosition wp, Team t);

};

