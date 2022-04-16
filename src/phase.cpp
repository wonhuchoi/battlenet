// internal
#include "phase.hpp"
#include "tiny_ecs.hpp"
#include "pause.hpp"
#include "background.hpp"
#include "board.hpp"
#include "unit.hpp"
#include "game_components.hpp"

#include <iterator>
#include <unordered_set>
#include <queue>
#include <iostream>

void PhaseSystem::step(float elapsed_ms)
{
	// animate backgrounds
	if (world->worldState == world->WORLD_PAUSE) {
		return;
	}
	else if (world->worldState != world->WORLD_VICTORY 
		&& world->worldState != world->WORLD_DEFEAT 
		&& timeStep > timeRefresh)
	{
		if (screenStep == 1)
		{
			ECS::registry<Motion>.get(bg).is_in_foreground = false;
			ECS::registry<Motion>.get(bg2).is_in_foreground = true;
			screenStep = 2;
			timeStep = 0;
		}
		else if (screenStep == 2)
		{
			ECS::registry<Motion>.get(bg2).is_in_foreground = false;
			ECS::registry<Motion>.get(bg3).is_in_foreground = true;
			screenStep = 3;
			timeStep = 0;
		}
		else if (screenStep == 3)
		{
			ECS::registry<Motion>.get(bg3).is_in_foreground = false;
			ECS::registry<Motion>.get(bg).is_in_foreground = true;
			screenStep = 1;
			timeStep = 0;
		}
		if (world->worldState == world->WORLD_CAMPAIGN) {
			boardRefresh();
		}
	}
	timeStep++;
	(void)elapsed_ms; // placeholder to silence unused warning until implemented
}

void PhaseSystem::boardRefresh() {
	Board::hide(world->board);
	Board::show(world->board);
	for (auto e : ECS::registry<Unit>.entities)
	{
		Motion& m = ECS::registry<Motion>.get(e);
		m.is_in_foreground = false;
		m.is_in_foreground = true;
	}
	for (auto e : ECS::registry<Parallax>.entities)
	{
		Motion& m = ECS::registry<Motion>.get(e);
		m.is_in_foreground = false;
		m.is_in_foreground = true;
	}
	for (auto e : ECS::registry<Item>.entities)
	{
		Motion& m = ECS::registry<Motion>.get(e);
		m.is_in_foreground = false;
		m.is_in_foreground = true;
	}
	for (auto e : ECS::registry<Player>.entities)
	{
		Motion& m = ECS::registry<Motion>.get(e);
		m.is_in_foreground = false;
		m.is_in_foreground = true;
	}
}

void PhaseSystem::boardHide() {
	Board::hide(world->board);
	for (auto e : ECS::registry<Unit>.entities)
	{
		Motion& m = ECS::registry<Motion>.get(e);
		m.is_in_foreground = false;
	}
	for (auto e : ECS::registry<Parallax>.entities)
	{
		Motion& m = ECS::registry<Motion>.get(e);
		m.is_in_foreground = false;
	}
	for (auto e : ECS::registry<Item>.entities)
	{
		Motion& m = ECS::registry<Motion>.get(e);
		m.is_in_foreground = false;
	}
	for (auto e : ECS::registry<Player>.entities)
	{
		Motion& m = ECS::registry<Motion>.get(e);
		m.is_in_foreground = false;
	}
}


void PhaseSystem::begin()
{
	screenStep = 1;
	timeStep = 0;

	// add all backgrounds

	start = Background::createStart();
	start2 = Background::createStart2();
	start3 = Background::createStart3();

	bat = Background::createBatBG();
	bat2 = Background::createBatBG2();
	bat3 = Background::createBatBG3();

	plan = Background::createPlanBG();
	plan2 = Background::createPlanBG2();
	plan3 = Background::createPlanBG3();

	defeat = Background::createDefeat();
	victory = Background::createVictory();

	bg = start;
	bg2 = start2;
	bg3 = start3;

	ECS::registry<Motion>.get(bg).is_in_foreground = true;
}

void PhaseSystem::cleanUnits() {
	while (ECS::registry<Unit>.entities.size() > 0)
		ECS::ContainerInterface::remove_all_components_of(ECS::registry<Unit>.entities.back());
}

void PhaseSystem::moveEnemyUnits() {
	int w, h;
	glfwGetWindowSize(world->window, &w, &h);
	int xPos = w - 100;
	endScreenSpeed = 20.0f;
	for (auto e : ECS::registry<Unit>.entities) {
		ECS::ContainerInterface::remove_all_components_of(e);
	}
	for (int i = 0; i < (w/100) - 2; i++) {
		ECS::Entity e = Unit::createUnit(vec2(0, 0),
			ECS::registry<BoardInfo>.get(world->board),
			ECS::registry<WorldPosition>.get(world->board),
			enemy, trojan, CombatStats());
		Motion& m = ECS::registry<Motion>.get(e);
		m.angle = 0;
		m.is_in_foreground = true;
		m.speed = endScreenSpeed * 6;
		m.position = { xPos, 75 };
		m.is_moving = true;
		m.destination = { xPos - (i * 100) , 75 };

		ECS::Entity e2 = Unit::createUnit(vec2(0, 0),
			ECS::registry<BoardInfo>.get(world->board),
			ECS::registry<WorldPosition>.get(world->board),
			enemy, trojan, CombatStats());
		Motion& m2 = ECS::registry<Motion>.get(e2);
		m2.angle = 0;
		m2.is_in_foreground = true;
		m2.speed = endScreenSpeed * 6;
		m2.position = { xPos, h - 75 };
		m2.is_moving = true;
		m2.destination = { xPos - (i * 100) , h - 75 };
	}
}

void PhaseSystem::moveAllyUnits() {
	int xPos = 100, yPos = 75;
	int w, h;
	glfwGetWindowSize(world->window, &w, &h);
	for (auto e : ECS::registry<Unit>.entities)
	{
		if (ECS::registry<Allies>.has(e) && ECS::registry<CombatStats>.get(e).currentHp > 0) {
			Motion& m = ECS::registry<Motion>.get(e);
			m.is_in_foreground = true;
			m.angle = 0;
			m.position = { xPos, yPos };
			m.destination = { w , yPos };
			m.is_moving = true;
			xPos += 100;
		}
		else {
			ECS::ContainerInterface::remove_all_components_of(e);
		}
	}
	for (auto e : ECS::registry<Player>.entities)
	{
		Motion& m = ECS::registry<Motion>.get(e);
		m.is_in_foreground = true;
		playerMot = m;
		savedPlayer = true;
		endScreenSpeed = (w - xPos) / 10;
		m.speed = endScreenSpeed;
		m.angle = 0;
		m.velocity = { 0,0 };
		m.position = { xPos, yPos };
		m.destination = { w , yPos };
		m.is_moving = true;
		xPos += 100;
	}
	for (auto e : ECS::registry<Unit>.entities)
	{
		Motion& m = ECS::registry<Motion>.get(e);
		m.speed = endScreenSpeed;
	}
}

void PhaseSystem::playIntro() {
	int w, h;
	glfwGetWindowSize(world->window, &w, &h);
	for (auto e : ECS::registry<Player>.entities)
	{
		Motion& m = ECS::registry<Motion>.get(e);
		m.speed = endScreenSpeed;
		m.angle = 0;
		m.velocity = { 0,0 };
		m.is_in_foreground = true;
		m.position = { w/2 , 200 };
		m.is_moving = true;
	}
}

void PhaseSystem::update(WorldSystem* subject)
{
	// shift phases and backgrounds here
	world = subject;

	if (world->worldState == world->WORLD_START && ECS::registry<Motion>.entities.size() == 0) {
		begin();
	} else if (world->worldState == world->WORLD_CAMPAIGN){
		// remove start and pause screens
		/*Background::clear();*/
		
		ECS::registry<Motion>.get(start).is_in_foreground = false;
		ECS::registry<Motion>.get(start2).is_in_foreground = false;
		ECS::registry<Motion>.get(start3).is_in_foreground = false;

		// planning phase
		if (ECS::registry<PlanningPhase>.has(world->board)) {
			for (auto e : ECS::registry<Player>.entities)
			{
				if (savedPlayer) {
					Motion& m = ECS::registry<Motion>.get(e);
					m = playerMot;
					savedPlayer = false;
				}
			}
			bg = plan;
			bg2 = plan2;
			bg3 = plan3;
			ECS::registry<Motion>.get(bg).is_in_foreground = true;

			ECS::registry<Motion>.get(bat).is_in_foreground = false;
			ECS::registry<Motion>.get(bat2).is_in_foreground = false;
			ECS::registry<Motion>.get(bat3).is_in_foreground = false;
			ECS::registry<Motion>.get(victory).is_in_foreground = false;
			ECS::registry<Motion>.get(defeat).is_in_foreground = false;
		}
		else if (ECS::registry<BattlePhase>.has(world->board)) {
			bg = bat;
			bg2 = bat2;
			bg3 = bat3;
			ECS::registry<Motion>.get(bg).is_in_foreground = true;

			ECS::registry<Motion>.get(plan).is_in_foreground = false;
			ECS::registry<Motion>.get(plan2).is_in_foreground = false;
			ECS::registry<Motion>.get(plan3).is_in_foreground = false;
		}
		else if (ECS::registry<DefeatPhase>.has(world->board)) {
			bg = defeat;
			bg2 = defeat;
			bg3 = defeat;
			ECS::registry<Motion>.get(bg).is_in_foreground = true;

			boardHide();
			moveEnemyUnits();

			world->worldState = world->WORLD_DEFEAT;
		}
		else if (ECS::registry<VictoryPhase>.has(world->board)) {
			if (world->modeState == world->WORLD_TUTORIAL)
			{
				world->tutorialStepDone = true;
				ECS::registry<VictoryPhase>.remove(world->board);
				ECS::registry<BattlePhase>.emplace(world->board);
			}
			else {
				bg = victory;
				bg2 = victory;
				bg3 = victory;
				ECS::registry<Motion>.get(bg).is_in_foreground = true;
				world->playerTeam = {};
				boardHide();
				for (auto e : ECS::registry<Allies>.entities)
				{
					if (ECS::registry<CombatStats>.get(e).currentHp > 0) {
						BoardInfo& bi = ECS::registry<BoardInfo>.components[0];
						if (world->playerTeam.size() < (bi.boardSize.x - 1)) {
							world->playerTeam.push_back(ECS::registry<Identity>.get(e).ut);
						}
						else {
							bi.gold += 5;
						}
					}
				}
				moveAllyUnits();

				world->worldState = world->WORLD_VICTORY;
      }
		}
	}
	else if (world->worldState == world->WORLD_INTRO || world->worldState == world->WORLD_SELECT) {
		ECS::registry<Motion>.get(start).is_in_foreground = false;
		ECS::registry<Motion>.get(start2).is_in_foreground = false;
		ECS::registry<Motion>.get(start3).is_in_foreground = false;
		bg = plan;
		bg2 = plan2;
		bg3 = plan3;
		world->player = Player::createPlayer({ 100, 100 }, ally, playerCharacter);
		playIntro();
	}
}
