#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"

struct Pause
{
	// Creates all the associated render resources and default transform
	static ECS::Entity createPause();
	static void switchPause(ECS::Entity entity);
};