// internal
#include "ai.hpp"
#include "tiny_ecs.hpp"
#include "util.hpp"
#include "game_components.hpp"
#include "board.hpp"
#include "debug.hpp"

#include <random>
#include <iostream>
#include <iterator>
#include <unordered_set>
#include <queue>

void AISystem::step(float elapsed_ms, vec2 window_size_in_game_units)
{
	for (auto& e : ECS::registry<Freeze>.entities) {
		Freeze& f = ECS::registry<Freeze>.get(e);
		f.elapsed_ms += elapsed_ms;
		if (f.totalTime <= f.elapsed_ms) {
			ECS::registry<Freeze>.remove(e);
		}
	}

	//Squish interpolation
	for (auto& e : ECS::registry<Squish>.entities) {
		Squish& s = ECS::registry<Squish>.get(e);
		Motion& m = ECS::registry<Motion>.get(e);
		s.elapsed_ms += elapsed_ms;
		if (s.elapsed_ms > s.phases * s.max_ms) {
			m.scale = vec2(s.old_x_scale, s.old_y_scale);
			ECS::registry<Squish>.remove(e);
		}
		else {
			float prop = s.elapsed_ms / s.max_ms;
			int phase = s.elapsed_ms / s.max_ms;
			prop -= phase;
			std::vector<vec2> x;
			x.push_back(vec2(s.old_x_scale, 0));
			x.push_back(vec2(s.old_x_scale * s.x_ratio * pow(s.dampening, phase), 0));
			std::vector<vec2> y;
			y.push_back(vec2(s.old_y_scale, 0));
			y.push_back(vec2(s.old_y_scale*s.y_ratio * pow(s.dampening, phase), 0));
			float new_x_scale = util::deCasteljau_bezier(x, prop)[0];
			float new_y_scale = util::deCasteljau_bezier(y, prop)[0];

			m.scale = vec2(new_x_scale, new_y_scale);
		}
	}

	//Draw line for bound units
	for (auto& e : ECS::registry<Bind>.entities) {
		Bind b = ECS::registry<Bind>.get(e);
		if (ECS::registry<CombatStats>.has(*b.bound_unit)) {
			CombatStats cs1 = ECS::registry<CombatStats>.get(*b.bound_unit);
			CombatStats cs2 = ECS::registry<CombatStats>.get(e);
			if (cs1.currentHp > 0 && cs2.currentHp > 0) {
				Motion m1 = ECS::registry<Motion>.get(*b.bound_unit);
				Motion m2 = ECS::registry<Motion>.get(e);
				DebugSystem::createLineFromPoints(m1.position, m2.position, vec3(0, 0, 0));
			}
			else {
				ECS::registry<Bind>.remove(e);
			}
		}
	}

	for (auto e : ECS::registry<WindUp>.entities) {
		WindUp& w = ECS::registry<WindUp>.get(e);
		CombatStats& cs = ECS::registry<CombatStats>.get(e);
		w.timeCasting += elapsed_ms;
		if (w.castTime <= w.timeCasting) {
			ECS::registry<WindUp>.remove(e);
			Casting& c = ECS::registry<Casting>.emplace(e);
			c.castTime = cs.skill_duration;
		}
	}

	for (auto e : ECS::registry<Casting>.entities) {
		Casting& c = ECS::registry<Casting>.get(e);
		c.timeCasting += elapsed_ms;
		if (c.castTime <= c.timeCasting) {
			ECS::registry<Casting>.remove(e);
		}
	}

	for (auto e : ECS::registry<WindDown>.entities) {
		WindDown& w = ECS::registry<WindDown>.get(e);
		w.timeCasting += elapsed_ms;
		if (w.castTime <= w.timeCasting) {
			ECS::registry<WindDown>.remove(e);
		}
	}
	this->time_until_next_step -= elapsed_ms;
	if (time_until_next_step < 0) {
		time_until_next_step = INTERVAL_MS;
		auto a = ECS::registry<UnitAI>.entities;
		for (auto uai : ECS::registry<UnitAI>.entities) {
			UnitAI ai = ECS::registry<UnitAI>.get(uai);
			ai.execute();
		}
	}


}

void AISystem::update(AttackSystem *subject)
{
  printf("enemy has died!");
}

Selector::Selector(std::vector<AINode*> nodes, ECS::Entity* agent) {
	this->children = nodes;
	this->agent = *agent;
	this->isRunning = false;
	this->running_node = NULL;
	this->status = IDLE;
}

nodeStatus Selector::execute() {
	if (ECS::registry<BattlePhase>.entities.size() == 0)
		return IDLE;
	else
		this->status = RUNNING;
	if (this->status == RUNNING) {
		for (unsigned int i = 0; i < this->children.size(); i++) {
			AINode* node = this->children[i];
			if (node->execute() == RUNNING) {
				this->running_node = node;
				this->status = RUNNING;
				return RUNNING;
			}
		}
	}
	//if no action can execute, fail node
	this->status = FAILED;
	return FAILED;
}


nodeStatus Randomizer::execute() {
	if (ECS::registry<BattlePhase>.entities.size() == 0)
		return IDLE;
	else
		this->status = RUNNING;
	if (this->running_node != NULL && this->running_node->execute() == RUNNING) {
		this->status = RUNNING;
		return RUNNING;
	}
	if (this->status == RUNNING) {
		std::vector<bool> tried (this->children.size(), false);
		int r;
		bool finished = false;
		while (finished == false) {
			r = rand() % tried.size();
			AINode* node = this->children[r];
			if (node->execute() == RUNNING) {
				this->running_node = node;
				this->status = RUNNING;
				return RUNNING;
			}
			tried[r] = true;
			if (std::find(tried.begin(), tried.end(), false) == tried.end()) {
				finished = true;
			}
		}
	}
	//if no action can execute, fail node
	this->status = FAILED;
	return FAILED;
}

UnitAI::UnitAI(ECS::Entity* unit) {
	this->unit = *unit;
	this->isRunning = false;
	this->status = RUNNING;
	std::vector<AINode*> vec;
	MoveNode* mn = new MoveNode(unit);
	IdleNode* in = new IdleNode(unit);
	CastNode* cn = new CastNode(unit);
	AttackNode* an = new AttackNode(unit);
	vec.push_back(in);
	vec.push_back(cn);
	vec.push_back(an);
	vec.push_back(mn);
	Selector* s = new Selector(vec, unit);
	this->root = s;
}

void UnitAI::setCastNode(CastNode* cn) {
	std::vector<AINode*>& vec = this->root->getChildren();
	vec[1] = cn;
}

std::vector<AINode*>& Selector::getChildren() {
	return this->children;
}

nodeStatus UnitAI::execute() {
	if (ECS::registry<BattlePhase>.entities.size() == 0)
		return IDLE;
	else
		this->status = RUNNING;
	this->isRunning = true;
	this->root->isRunning = true;
	this->root->execute();
	return RUNNING;
}

MoveNode::MoveNode(ECS::Entity* agent) {
	this->agent = *agent;
	this->status = IDLE;
	this->isRunning = false;
}

IdleNode::IdleNode(ECS::Entity* agent) {
	this->agent = *agent;
	this->status = IDLE;
	this->isRunning = false;
}

AttackNode::AttackNode(ECS::Entity* agent) {
	this->agent = *agent;
	this->status = IDLE;
	this->isRunning = false;
}

CastNode::CastNode(ECS::Entity* agent) {
	this->agent = *agent;
	this->status = IDLE;
	this->isRunning = false;
}

Randomizer::Randomizer(std::vector<AINode*> nodes, ECS::Entity* agent) {
	this->children = nodes;
	this->agent = *agent;
	this->isRunning = false;
	this->running_node = NULL;
	this->status = IDLE;
}

CastNode::CastNode() {
}

nodeStatus AttackNode::execute() {
	if (ECS::registry<BattlePhase>.entities.size() == 0)
		return IDLE;
	else
		this->status = RUNNING;
	
	CombatStats& cs = ECS::registry<CombatStats>.get(this->agent);
	ECS::Entity boardEntity;
	// If board has been created
	if (ECS::registry<Board>.entities.size()) {
		boardEntity = ECS::registry<Board>.entities[0];
	}
	else {
		return FAILED;
	}
	float cellSize = ECS::registry<BoardInfo>.get(boardEntity).cellSize.x;
	// return 1 (success) if we have target
	if (cs.hasTarget) {
		CombatStats& enemyCs = ECS::registry<CombatStats>.get(cs.target);
		Motion& motion = ECS::registry<Motion>.get(this->agent);
		Motion& enemyMotion = ECS::registry<Motion>.get(cs.target);
		// Check if the current target is in range
		if (util::inRange(motion.position, enemyMotion.position, cs.range * cellSize) && enemyCs.currentHp > 0) {
			// TODO: Attack speed restriction? 
			// May need to separate out this system into motion (once per second) and attack (continuous) to allow for attack speeds.
			// TODO: Trigger attack animation?
			cs.action = ATTACK;
			return RUNNING;
		}
		// Cancel target and try to find another
		else {
			cs.hasTarget = false;
		}
	}
	std::vector<ECS::Entity> opponentsInRange;
	std::vector<ECS::Entity> opponents;
	std::vector<ECS::Entity> allOpponents;
	if (ECS::registry<Allies>.has(this->agent)) {
		allOpponents = ECS::registry<Enemies>.entities;
	}
	else {
		allOpponents = ECS::registry<Allies>.entities;
	}
	std::copy_if(allOpponents.begin(), allOpponents.end(), std::back_inserter(opponents), [](ECS::Entity e) {return ECS::registry<CombatStats>.get(e).currentHp > 0; });
	for (ECS::Entity e1 : opponents) {
		Motion& motion = ECS::registry<Motion>.get(this->agent);
		Motion& enemyMotion = ECS::registry<Motion>.get(e1);
		if (util::inRange(motion.position, enemyMotion.position, cs.range * cellSize)) {
			opponentsInRange.push_back(e1);
		}
	}

	// return 1 (success) if we have target
	if (opponentsInRange.size()) {
		std::uniform_int_distribution<> distr(0, opponentsInRange.size() - 1);
		std::default_random_engine gen;
		ECS::Entity enemy = opponentsInRange[distr(gen)];
		cs.hasTarget = true;
		cs.target = enemy;
		// TODO: Trigger attack animation?
		cs.action = ATTACK;
		return RUNNING;
	}
	
	// If no unit is attackable, return failed status
	return FAILED;
}


nodeStatus MoveNode::execute() {
	
	ECS::Entity boardEntity;
	if (ECS::registry<BattlePhase>.entities.size() == 0)
		return IDLE;
	else
		this->status = RUNNING;
	// If board has been created
	if (ECS::registry<Board>.entities.size()) {
		boardEntity = ECS::registry<Board>.entities[0];
	}
	else {
		return FAILED;
	}

	// Holds the squares (currently or in future) occupied by enemies
	ECS::Entity e = this->agent;
	if (ECS::registry<BoardPosition>.has(e)) {
		CombatStats& cs = ECS::registry<CombatStats>.get(e);
		cs.action = MOVE;
		return RUNNING;
	}
	return FAILED;
}

nodeStatus IdleNode::execute() {
	if (ECS::registry<Freeze>.has(this->agent))
		return RUNNING;
	return FAILED;
}

bool CastNode::canCast() {
	CombatStats cs = ECS::registry<CombatStats>.get(this->agent);
	return cs.manaToCast <= cs.currMana;
}

nodeStatus CastNode::execute() {
	return FAILED;
}

USetVec2 occupiedSquares(std::vector<ECS::Entity> units) {
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
