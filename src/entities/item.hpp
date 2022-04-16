#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "game_components.hpp"

// Salmon enemy 
struct Item
{
	// Creates all the associated render resources and default transform
	static ECS::Entity createItem(vec2 position);
};
