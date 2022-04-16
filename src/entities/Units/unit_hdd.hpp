#pragma once

#include "common.hpp"
#include "game_components.hpp"
#include "tiny_ecs.hpp"
#include "ai.hpp"

struct UnitHDD
{
	// Creates all the associated render resources and default transform
	static ECS::Entity createUnitHDD(vec2 boardPosition, BoardInfo bi, WorldPosition wp, Team t);

};

class HDDCastNode : public CastNode
{
private:
	std::vector<ECS::Entity*> adj_enemies;
public:
	nodeStatus execute();
	HDDCastNode(ECS::Entity* agent);
};

