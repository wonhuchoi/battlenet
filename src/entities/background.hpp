#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"

struct Background 
{
	// Creates all the associated render resources and default transform
	static ECS::Entity creator(std::string key);
	static ECS::Entity creator(std::string key, std::string shader);
	
	static ECS::Entity createPlanBG();
	static ECS::Entity createPlanBG2();
	static ECS::Entity createPlanBG3();

	static ECS::Entity createBatBG();
	static ECS::Entity createBatBG2();
	static ECS::Entity createBatBG3();

	static ECS::Entity createStart();
	static ECS::Entity createStart2();
	static ECS::Entity createStart3();

	static ECS::Entity createDefeat();
	static ECS::Entity createVictory();

	static void clear();
};
