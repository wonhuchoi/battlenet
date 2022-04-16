#pragma once

#include "common.hpp"
#include "game_components.hpp"
#include "tiny_ecs.hpp"
#include "ai.hpp"

struct UnitWifi
{
	// Creates all the associated render resources and default transform
	static ECS::Entity createUnitWifi(vec2 boardPosition, BoardInfo bi, WorldPosition wp, Team t);

};

class WifiCastNode : public CastNode
{
private:
	ECS::Entity* cast_target;
public:
	nodeStatus execute();
	WifiCastNode(ECS::Entity* agent);
	bool canCast();
};

