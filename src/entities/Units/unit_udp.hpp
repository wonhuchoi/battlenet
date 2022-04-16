#pragma once

#include "common.hpp"
#include "game_components.hpp"
#include "tiny_ecs.hpp"
#include "ai.hpp"

struct UnitUDP
{
	// Creates all the associated render resources and default transform
	static ECS::Entity createUnitUDP(vec2 boardPosition, BoardInfo bi, WorldPosition wp, Team t);

};

class UDPCastNode : public CastNode
{
private:
	ECS::Entity* cast_target;
public:
	nodeStatus execute();
	UDPCastNode(ECS::Entity* agent);
	bool canCast();
};

