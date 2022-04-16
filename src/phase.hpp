#pragma once

#include <vector>

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "world.hpp"
#include <observer.hpp>

class PhaseSystem : public Observer<WorldSystem>
{
public:
	void update(WorldSystem* subject);
	void step(float elapsed_ms);
	void begin();
	void boardRefresh();
	void boardHide();
	void cleanUnits();
	void moveEnemyUnits();
	void moveAllyUnits();
	void playIntro();

private:
	// time step
	int timeStep;
	int timeRefresh = 20;
	int screenStep;

	Motion playerMot;
	bool savedPlayer = false;
	float endScreenSpeed = 20.0f;

	ECS::Entity start;
	ECS::Entity start2;
	ECS::Entity start3;

	ECS::Entity bg;
	ECS::Entity bg2;
	ECS::Entity bg3;

	ECS::Entity plan;
	ECS::Entity plan2;
	ECS::Entity plan3;

	ECS::Entity bat;
	ECS::Entity bat2;
	ECS::Entity bat3;

	ECS::Entity defeat;
	ECS::Entity victory;

	WorldSystem* world;
};
