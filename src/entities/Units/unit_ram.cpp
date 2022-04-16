#include "unit.hpp"
#include "game_components.hpp"
#include "render.hpp"
#include "tiny_ecs.hpp"
#include "ai.hpp"
#include "unit_ram.hpp"
#include "particle.hpp"
#include "debug.hpp"

#include <iostream>

ECS::Entity UnitRAM::createUnitRAM(vec2 boardPosition, BoardInfo bi, WorldPosition wp, Team t) {
	CombatStats cs = CombatStats();
	cs.range = 2;
	cs.totalHp = 200;
	cs.currentHp = cs.totalHp;
	cs.attack = 15;
	UnitType ut = ram;
	ECS::Entity entity = Unit::createUnit(boardPosition, bi, wp, t, ut, cs);
	Identity& i = ECS::registry<Identity>.get(entity);
	i.abilityDesc = "RAM pays for itself. Add it to your board to get an extra unit";
	return entity;
}

