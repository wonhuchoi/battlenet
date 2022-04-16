#pragma once

#include "common.hpp"
#include "game_components.hpp"
#include "tiny_ecs.hpp"
#include "ai.hpp"

struct UnitDDOS
{
	// Creates all the associated render resources and default transform
	static ECS::Entity createUnitDDOS(vec2 boardPosition, BoardInfo bi, WorldPosition wp, Team t);

};

class DDOSCastNode : public CastNode
{
private:
	std::vector<ECS::Entity*> adj_enemies;
public:
	nodeStatus execute();
	DDOSCastNode(ECS::Entity* agent);
};

