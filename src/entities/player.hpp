#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "game_components.hpp"

struct Player 
{
	// Creates all the associated render resources and default transform
	static ECS::Entity createPlayer(vec2 position, Team t, UnitType ut);

	//TODO: remove this function if we end up not doing animation
	static void refresh(ECS::Entity entity);
};
