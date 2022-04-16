#pragma once

#include "common.hpp"
#include "game_components.hpp"
#include "tiny_ecs.hpp"
#include "ai.hpp"

struct UnitTrojan
{
	// Creates all the associated render resources and default transform
	static ECS::Entity createUnitTrojan(vec2 boardPosition, BoardInfo bi, WorldPosition wp, Team t);

};

class TrojanCastNode : public CastNode
{
public:
	nodeStatus execute();
	TrojanCastNode(ECS::Entity* agent);
};

