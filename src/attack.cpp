#include "attack.hpp"
#include "game_components.hpp"
#include "tiny_ecs.hpp"
#include "unit.hpp"
#include "world.hpp"
#include "util.hpp"
#include "board.hpp"
#include <iterator>
#include <unordered_set>
#include <queue>
#include <iostream>
#include "debug.hpp"



void AttackSystem::step(float elapsed_ms)
{
	if (ECS::registry<BattlePhase>.entities.size() == 0) {
		return;
	}

	std::vector<ECS::Entity> allAllies = ECS::registry<Allies>.entities;
	std::vector<ECS::Entity> allEnemies = ECS::registry<Enemies>.entities;

	std::vector<ECS::Entity> allies;
	std::vector<ECS::Entity> enemies;
	// filter only alive units:
	std::copy_if(allAllies.begin(), allAllies.end(), std::back_inserter(allies), [](ECS::Entity e) {return ECS::registry<CombatStats>.get(e).currentHp > 0; });
	std::copy_if(allEnemies.begin(), allEnemies.end(), std::back_inserter(enemies), [](ECS::Entity e) {return ECS::registry<CombatStats>.get(e).currentHp > 0; });

	handleAttacks(allies, enemies, elapsed_ms);
	handleAttacks(enemies, allies, elapsed_ms);

	allAllies = ECS::registry<Allies>.entities;
	allEnemies = ECS::registry<Enemies>.entities;

	allies = std::vector<ECS::Entity>();
	enemies = std::vector<ECS::Entity>();

	std::copy_if(allAllies.begin(), allAllies.end(), std::back_inserter(allies), [](ECS::Entity e) {return ECS::registry<CombatStats>.get(e).currentHp > 0 && !ECS::registry<BoardPosition>.get(e).isBench; });
	std::copy_if(allEnemies.begin(), allEnemies.end(), std::back_inserter(enemies), [](ECS::Entity e) {return ECS::registry<CombatStats>.get(e).currentHp > 0; });

	if (allies.size() <= 0 || enemies.size() <= 0) {
		notifyObservers();
	}

	return;
}

// Handles attacks in the direction of allies -> enemies 
// Assumes AI has chosen target
void AttackSystem::handleAttacks(std::vector<ECS::Entity> allies, std::vector<ECS::Entity> enemies, float elapsed_ms) {
	// Check if units in range and if so, attack
	std::unordered_set<int> ret;
	std::vector<ECS::Entity> attackingAllies;
	std::copy_if(allies.begin(), allies.end(), std::back_inserter(attackingAllies), [](ECS::Entity e) {return ECS::registry<CombatStats>.get(e).action == ATTACK; });
	for (ECS::Entity e : allies) {
		// Check for currently targeted entity and if it is still in range
		CombatStats& cs = ECS::registry<CombatStats>.get(e);
		cs.time_from_last_attack += elapsed_ms;
		if (cs.hasTarget && (cs.time_from_last_attack > 1000/cs.attacks_per_second)) {
			CombatStats& enemyCs = ECS::registry<CombatStats>.get(cs.target);
			DebugSystem::createLineFromPoints(ECS::registry<Motion>.get(e).position, ECS::registry<Motion>.get(cs.target).position, vec3(1.0, 1.0, 1.0));
			cs.time_from_last_attack = 0.f;
			Unit::damageUnit(cs.target, cs.attack);
			cs.currMana = std::min(cs.currMana + 10, cs.manaToCast);
		}
	}
}
