#pragma once

#include "common.hpp"
#include "game_components.hpp"
#include "tiny_ecs.hpp"
#include "ai.hpp"

struct UnitCPU
{
	// Creates all the associated render resources and default transform
	static ECS::Entity createUnitCPU(vec2 boardPosition, BoardInfo bi, WorldPosition wp, Team t);

};

class CPUCastNode : public CastNode
{
public:
	nodeStatus execute();
	CPUCastNode(ECS::Entity* agent);
};

