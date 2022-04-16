#pragma once

#include "common.hpp"
#include "game_components.hpp"
#include "tiny_ecs.hpp"
#include "ai.hpp"

struct UnitPhishing
{
	// Creates all the associated render resources and default transform
	static ECS::Entity createUnitPhishing(vec2 boardPosition, BoardInfo bi, WorldPosition wp, Team t);

};

class PhishingCastNode : public CastNode
{
public:
	Randomizer* randomizer;
	nodeStatus execute();
	PhishingCastNode(ECS::Entity* agent);
	int runningIndex;
};

