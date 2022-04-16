#pragma once

#include <vector>

#include "common.hpp"
#include "game_components.hpp"
#include "tiny_ecs.hpp"
#include <observer.hpp>

class LevelSystem
{
public:
	static void generate(ECS::Entity board, std::vector<int> unitVec);
	static void makeEnemies(BoardInfo bi, WorldPosition wp, std::vector<int> unitVec);
	static void makeAllies(BoardInfo bi, WorldPosition wp);
	static void shuffleEnemy(BoardInfo bi, WorldPosition wp, std::vector<int> unitVec, int count);
};
