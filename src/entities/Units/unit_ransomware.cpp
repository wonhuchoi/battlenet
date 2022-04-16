#include "unit.hpp"
#include "game_components.hpp"
#include "render.hpp"
#include "tiny_ecs.hpp"
#include "unit_ransomware.hpp"
#include "ai.hpp"
#include "particle.hpp"
#include "debug.hpp"

#include <iostream>

ECS::Entity UnitRansomware::createUnitRansomware(vec2 boardPosition, BoardInfo bi, WorldPosition wp, Team t) {
	CombatStats cs = CombatStats();
	cs.range = 0;
	cs.totalHp = 400;
	cs.currentHp = cs.totalHp;
	cs.attack = 5;
	cs.manaToCast = 0;
	UnitType ut = ransomware;
	ECS::Entity entity = Unit::createUnit(boardPosition, bi, wp, t, ut, cs);
	Identity& i = ECS::registry<Identity>.get(entity);
	i.abilityDesc = "Start the combat by attaching to a unit, upon everytime this unit takes damage, the bound unit will take the same amount. When a bound unit dies, this unit will bind to the closest enemy unit";
	UnitAI uai = ECS::registry<UnitAI>.get(entity);
	CastNode* cn = new RansomwareCastNode(&entity);
	uai.setCastNode(cn);
	return entity;
}

RansomwareCastNode::RansomwareCastNode(ECS::Entity* agent) {
	this->agent = *agent;
	this->status = IDLE;
	this->isRunning = false;
	this->bound_unit = NULL;
}

nodeStatus RansomwareCastNode::execute() {
	if (ECS::registry<WindUp>.has(this->agent)) {
		return RUNNING;
	}

	if (ECS::registry<WindDown>.has(this->agent)) {
		return RUNNING;
	}

	if (this->canCast()) {
		//SetUp WindUp
		ECS::registry<WindUp>.emplace(this->agent);
		TeamAffiliation team = ECS::registry<TeamAffiliation>.get(this->agent);
		float max_hp = 0;
		for (auto& e : ECS::registry<Unit>.entities) {
			CombatStats& entity_cs = ECS::registry<CombatStats>.get(e);
			TeamAffiliation e_team = ECS::registry<TeamAffiliation>.get(e);
			if (max_hp < entity_cs.currentHp && entity_cs.currentHp > 0 && e_team.t != team.t) {
				this->bound_unit = &e;
				max_hp = entity_cs.currentHp;
			}
		}
		if (!ECS::registry<Bind>.has(this->agent)) {
			Bind& b = ECS::registry<Bind>.emplace(this->agent);
			b.bound_unit = this->bound_unit;
		}
		return RUNNING;
	}
	if (ECS::registry<Casting>.has(this->agent)) {
		Motion m = ECS::registry<Motion>.get(*this->bound_unit);
		Particle::createParticle(m.position, { 50.,50. }, "skull", 400, 400, 0.015, 0, 0, -50, -50);
		ECS::registry<WindDown>.emplace(this->agent);
		this->status = RUNNING;
	}
	return FAILED;
}

bool RansomwareCastNode::canCast() {
	//check if we have valid bound unit
	if (this->bound_unit != NULL && ECS::registry<CombatStats>.has(*this->bound_unit) && ECS::registry<CombatStats>.get(*this->bound_unit).currentHp > 0)
		return false;
	TeamAffiliation& my_team = ECS::registry<TeamAffiliation>.get(this->agent);
	for (auto& e : ECS::registry<Unit>.entities) {
		CombatStats& entity_cs = ECS::registry<CombatStats>.get(e);
		TeamAffiliation e_team = ECS::registry<TeamAffiliation>.get(e);
		if (e_team.t != my_team.t && entity_cs.currentHp > 0) {
			return true;
		}
	}
	return false;
}
