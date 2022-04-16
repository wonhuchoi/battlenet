#pragma once

#include "common.hpp"
#include "game_components.hpp"
#include "tiny_ecs.hpp"
#include "ai.hpp"

struct UnitDebugger
{
	// Creates all the associated render resources and default transform
	static ECS::Entity createUnitDebugger(vec2 boardPosition, BoardInfo bi, WorldPosition wp, Team t);

};

class DebuggerCastNode : public CastNode
{
private:
	ECS::Entity* AllyToBeHealed;
public:
	nodeStatus execute();
	DebuggerCastNode(ECS::Entity* agent);
};

