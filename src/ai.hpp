#pragma once

#include <vector>

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "game_components.hpp"
#include "battle.hpp"
#include "unit.hpp"
#include "attack.hpp"

#include <observer.hpp>

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// DON'T WORRY ABOUT THIS CLASS UNTIL ASSIGNMENT 3
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

class AISystem : public Observer<AttackSystem>
{
private:
	size_t INTERVAL_MS = 100;
	float time_until_next_step = 100;
public:
  void update (AttackSystem*subject);
	void step(float elapsed_ms, vec2 window_size_in_game_units);
	// AISystem();
};

enum nodeStatus {
	RUNNING,
	FAILED,
	IDLE
};

class AINode
{
public:
	bool isRunning = false;
	nodeStatus status = IDLE;
	virtual nodeStatus execute() { return IDLE; };
};

class Selector : public AINode
{
private:
	std::vector<AINode*> children;
	AINode* running_node;
	ECS::Entity agent;
public:
	Selector(std::vector<AINode*> nodes, ECS::Entity* agent);
	std::vector<AINode*>& getChildren();
	virtual nodeStatus execute();
};

class Randomizer : public AINode
{
private:
	std::vector<AINode*> children;
	AINode* running_node;
	ECS::Entity agent;
public:
	Randomizer(std::vector<AINode*> nodes, ECS::Entity* agent);
	std::vector<AINode*>& getChildren();
	virtual nodeStatus execute();
};

class CastNode : public AINode
{
protected:
	ECS::Entity agent;
	bool canCast();
public:
	virtual nodeStatus execute();
	CastNode(ECS::Entity* agent);
	CastNode();
};

class UnitAI : public AINode {
private:
	Selector* root;
	ECS::Entity unit;
public:
	UnitAI(ECS::Entity* unit);
	virtual nodeStatus execute();
	void setCastNode(CastNode* cn);
};

class MoveNode : public AINode
{
private:
	ECS::Entity agent;
public:
	virtual nodeStatus execute();
	MoveNode(ECS::Entity* agent);
};

class IdleNode : public AINode
{
private:
	ECS::Entity agent;
public:
	virtual nodeStatus execute();
	IdleNode(ECS::Entity* agent);
};


class AttackNode : public AINode
{
private:
	ECS::Entity agent;
public:
	virtual nodeStatus execute();
	AttackNode(ECS::Entity* agent);
};
