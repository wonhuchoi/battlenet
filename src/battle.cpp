#include "battle.hpp"
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

/*
  Returns an unordered set of occupied squares.
  Contains squares that units are occupying if they are stationary or squares they are traveling to if not.
*/
USetVec2 BattleSystem::occupiedSquares(std::vector<ECS::Entity> units) {
	USetVec2 ret;
	for (auto& e : units) {
		if (ECS::registry<BoardPosition>.has(e)) {
			auto bp = ECS::registry<BoardPosition>.get(e);
			if (bp.futurePos.x != -1) {
				ret.insert(bp.futurePos);
			}
			else {
				ret.insert(bp.bPos);
			}
		}
	}
	return ret;
}

// Returns the position of the next adjacent square to move to, or the current position if no adjacent squares are available.
vec2 BattleSystem::moveEntity(ECS::Entity e, USetVec2 targets, USetVec2& occupied, vec2 boardSize) {
	// Queue of vec4s that holds (next position, first adjacent square origin)
	std::queue<vec4> q;
	auto bp = ECS::registry<BoardPosition>.get(e);
	USetVec2 visited;
	visited.insert(bp.bPos);
	// Enqueue our adjacent squares if not occupied and in bounds
	for (int i = 0; i < 2; i++) {
		for (int j = -1; j < 2; j += 2) {
			vec2 intendedPos = bp.bPos;
			if (i % 2) {
				intendedPos += vec2(0, j);
			}
			else {
				intendedPos += vec2(j, 0);
			}
			// If there will be a target beside us, wait for it 
			if (targets.find(intendedPos) != targets.end()) {
				return bp.bPos;
			}
			// If we are in bounds and not targeting an occupied square:
			if (intendedPos.x >= 0 && intendedPos.x < boardSize.x &&
				intendedPos.y >= 0 && intendedPos.y < boardSize.y &&
				occupied.find(intendedPos) == occupied.end()) {
				q.push(vec4(intendedPos, intendedPos));
				visited.insert(intendedPos);
			}
		}
	}
	// Set the target to the unit's current position
	vec2 target = bp.bPos;
	while (q.size()) {
		vec4 next = q.front();
		q.pop();
		// Explore adjacent squares
		for (int i = 0; i < 2; i++) {
			for (int j = -1; j < 2; j += 2) {
				vec2 intendedPos = vec2(next.x, next.y);
				if (i % 2) {
					intendedPos += vec2(0, j);
				}
				else {
					intendedPos += vec2(j, 0);
				}
				// If we have reached a target, return the initial adjacent square we reached it by
				if (targets.find(intendedPos) != targets.end()) {
					target = vec2(next.z, next.w);
					return target;
				}
				// If we are in bounds and not targeting an occupied or visited square:
				if (intendedPos.x >= 0 && intendedPos.x < boardSize.x &&
					intendedPos.y >= 0 && intendedPos.y < boardSize.y &&
					occupied.find(intendedPos) == occupied.end() &&
					visited.find(intendedPos) == visited.end()) {
					// Enqueue next position and the adjacent square we initially came from
					q.push(vec4(intendedPos, vec2(next.z, next.w)));
					visited.insert(intendedPos);
				}
			}
		}
	}
	return target;
}

void BattleSystem::resolveFuturePos(std::vector<ECS::Entity> entities) {
	for (auto e : entities) {
		if (ECS::registry<BoardPosition>.has(e)) {
			auto& bp = ECS::registry<BoardPosition>.get(e);
			if (bp.futurePos.x != -1) {
				bp.bPos = bp.futurePos;
				bp.futurePos = vec2(-1, -1);
			}
		}
	}
}

// Moves elligible ally units towards nearest opposing unit.
void BattleSystem::handleMovement(std::vector<ECS::Entity> allies, std::vector<ECS::Entity> enemies) {
	// Assumes all boards same size if we ever have multiple
	ECS::Entity boardEntity;
	// If board has been created
	if (ECS::registry<Board>.entities.size()) {
		boardEntity = ECS::registry<Board>.entities[0];
	}
	else {
		return;
	}
	WorldPosition boardPos = ECS::registry<WorldPosition>.get(boardEntity);
	BoardInfo board = ECS::registry<BoardInfo>.get(boardEntity);
	vec2 boardSize = board.boardSize;
	// Holds the squares (currently or in future) occupied by allies
	USetVec2 allyOcc = occupiedSquares(allies);
	// Holds the squares (currently or in future) occupied by enemies
	USetVec2 enemyOcc = occupiedSquares(enemies);
	allyOcc.insert(enemyOcc.begin(), enemyOcc.end());

	std::vector<ECS::Entity> movingAllies;

	std::copy_if(allies.begin(), allies.end(), std::back_inserter(movingAllies), [](ECS::Entity e) {return ECS::registry<CombatStats>.get(e).action == MOVE; });

	for (auto e : movingAllies) {
		// Make sure they are not in inelligible set and are on the board and not benched.
		auto& bp = ECS::registry<BoardPosition>.get(e);
		vec2 nextSquare = moveEntity(e, enemyOcc, allyOcc, boardSize);
		if (nextSquare.x != bp.bPos.x || nextSquare.y != bp.bPos.y) {
			allyOcc.erase(bp.bPos);
			allyOcc.insert(nextSquare);
			bp.futurePos = nextSquare;
			auto& unitMotion = ECS::registry<Motion>.get(e);
			// unitMotion.position = boardPosToWorldPos(nextSquare, board, boardPos);
			vec2 nextPos = boardPosToWorldPos(nextSquare, board, boardPos);
			unitMotion.angle = util::get_dir_angle(unitMotion.position, nextPos);
			unitMotion.destination = nextPos;
			unitMotion.is_moving = true;
			unitMotion.speed = 125.f;
		}
	}
}

// Steps the battleSystem forward, simulating attacks every step.
// Units can be either moving or attacking, always attacking if they are in range of another unit.
// Units move greedily toward the nearest unit, while avoiding other units.
void BattleSystem::step(float elapsed_ms) {
	// If we aren't ready for next battle step, just decrement counter:
	if (ECS::registry<BattlePhase>.entities.size() == 0)
		return;
	//Update stun and Immune params
	for (auto e : ECS::registry<Immune>.entities) {
		Immune& i = ECS::registry<Immune>.get(e);
		i.timeImmune += elapsed_ms;
		if (i.duration <= i.timeImmune) {
			ECS::registry<Immune>.remove(e);
		}
	}

	if (nextStep - elapsed_ms > 0) {
		nextStep -= elapsed_ms;
		return;
	}

	std::cout << "Running battle system" << std::endl;
	std::vector<ECS::Entity> allAllies = ECS::registry<Allies>.entities;
	std::vector<ECS::Entity> allEnemies = ECS::registry<Enemies>.entities;
	std::vector<ECS::Entity> allies;
	std::vector<ECS::Entity> enemies;

	// filter only alive and unbenched units:
	std::copy_if(allAllies.begin(), allAllies.end(), std::back_inserter(allies), [](ECS::Entity e) {return ECS::registry<CombatStats>.get(e).currentHp > 0 && !ECS::registry<BoardPosition>.get(e).isBench; });
	std::copy_if(allEnemies.begin(), allEnemies.end(), std::back_inserter(enemies), [](ECS::Entity e) {return ECS::registry<CombatStats>.get(e).currentHp > 0; });

	// Resolve future positions. 
	// Because each battle step can accomodate a movement step, all future positions are reached by the time this runs.
	// Handle attacks from both directions
	// Attackers set to hold those which have attacked this step.

	//std::unordered_set<int> attackers = handleAttacks(allies, enemies);
	//std::unordered_set<int> enemyAttackers = handleAttacks(enemies, allies);
	
	// Set union
	//attackers.insert(enemyAttackers.begin(), enemyAttackers.end());
	handleMovement(allies, enemies);
	handleMovement(enemies, allies);
	resolveFuturePos(allies);
	resolveFuturePos(enemies);
	nextStep = 1000;
}
