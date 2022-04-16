// internal
#include "score.hpp"
#include "game_components.hpp"
#include "tiny_ecs.hpp"
#include "unit.hpp"
#include <iterator>
#include <unordered_set>
#include <queue>
#include <iostream>

void ScoreSystem::step(float elapsed_ms, vec2 window_size_in_game_units)
{
	(void)elapsed_ms; // placeholder to silence unused warning until implemented
	(void)window_size_in_game_units; // placeholder to silence unused warning until implemented
}

void ScoreSystem::update(AttackSystem *subject)
{
	std::vector<ECS::Entity> allAllies = ECS::registry<Allies>.entities;
	std::vector<ECS::Entity> allEnemies = ECS::registry<Enemies>.entities;

	std::vector<ECS::Entity> allies;
	std::vector<ECS::Entity> enemies;

	// filter only alive and unbenched units:
	std::copy_if(allAllies.begin(), allAllies.end(), std::back_inserter(allies), [](ECS::Entity e) 
		{return ECS::registry<CombatStats>.get(e).currentHp > 0 && !ECS::registry<BoardPosition>.get(e).isBench; });
	std::copy_if(allEnemies.begin(), allEnemies.end(), std::back_inserter(enemies), [](ECS::Entity e) 
		{return ECS::registry<CombatStats>.get(e).currentHp > 0; });

	bool alliesDead = allies.size() == 0, enemiesDead = enemies.size() == 0;
	
	if (alliesDead) {
		std::cout << "DEFEAT\n";
		int enemiesKilled = allEnemies.size() - enemies.size();
		waveEnemies = enemiesKilled;
		enemiesDefeated += enemiesKilled;
		if (!ECS::registry<BattlePhase>.entities.empty()) {
			ECS::registry<DefeatPhase>.emplace(ECS::registry<BattlePhase>.entities[0]);
			ECS::registry<BattlePhase>.remove(ECS::registry<BattlePhase>.entities[0]);
		}
	}
	else if (enemiesDead) {
		std::cout << "VICTORY\n";
		waveEnemies = allEnemies.size();
		enemiesDefeated += allEnemies.size();
		if (!ECS::registry<BattlePhase>.entities.empty()) {
			ECS::registry<VictoryPhase>.emplace(ECS::registry<BattlePhase>.entities[0]);
			ECS::registry<BattlePhase>.remove(ECS::registry<BattlePhase>.entities[0]);
		}
	}
}
