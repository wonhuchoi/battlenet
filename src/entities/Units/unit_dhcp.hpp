#pragma once

#include "common.hpp"
#include "game_components.hpp"
#include "tiny_ecs.hpp"
#include "ai.hpp"

struct UnitDHCP
{
	// Creates all the associated render resources and default transform
	static ECS::Entity createUnitDHCP(vec2 boardPosition, BoardInfo bi, WorldPosition wp, Team t);

};

class DHCPCastNode : public CastNode
{
private:
	ECS::Entity* cast_target;
public:
	nodeStatus execute();
	DHCPCastNode(ECS::Entity* agent);
	bool canCast();
};

