#pragma once

#include "common.hpp"
#include "game_components.hpp"
#include "tiny_ecs.hpp"
#include "ai.hpp"

struct UnitRansomware
{
	// Creates all the associated render resources and default transform
	static ECS::Entity createUnitRansomware(vec2 boardPosition, BoardInfo bi, WorldPosition wp, Team t);

};

class RansomwareCastNode : public CastNode
{
public:
	ECS::Entity* bound_unit;
	nodeStatus execute();
	RansomwareCastNode(ECS::Entity* agent);
	bool canCast();
};

