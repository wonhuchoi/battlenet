#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "game_components.hpp"

enum InvaderType { regular, floating };

std::string getInvaderPath(InvaderType it);

struct SpaceInvader
{
	// Creates all the associated render resources and default transform
	static ECS::Entity createSpaceInvader(vec2 position, vec2 scale, InvaderType type);
};
