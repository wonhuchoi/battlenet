#include "unit.hpp"
#include "game_components.hpp"
#include "render.hpp"
#include "tiny_ecs.hpp"
#include "unit_trojan.hpp"
#include "ai.hpp"
#include "particle.hpp"

#include <iostream>

ECS::Entity UnitTrojan::createUnitTrojan(vec2 boardPosition, BoardInfo bi, WorldPosition wp, Team t) {
	CombatStats cs = CombatStats();
	cs.range = 1;
	cs.totalHp = 400;
	cs.currentHp = cs.totalHp;
	UnitType ut = trojan;
	ECS::Entity entity = Unit::createUnit(boardPosition, bi, wp, t, ut, cs);
	Identity& i = ECS::registry<Identity>.get(entity);
	i.abilityDesc = "Trojans can do some serious damage. Upon cast, your next attack deals additional damage!";
	UnitAI uai = ECS::registry<UnitAI>.get(entity);
	CastNode* cn = new TrojanCastNode(&entity);
	uai.setCastNode(cn);
	return entity;
}

TrojanCastNode::TrojanCastNode(ECS::Entity* agent) {
	this->agent = *agent;
	this->status = IDLE;
	this->isRunning = false;
}

nodeStatus TrojanCastNode::execute() {
	if (ECS::registry<WindUp>.has(this->agent)) {
		return RUNNING;
	}

	if (ECS::registry<WindDown>.has(this->agent)) {
		return RUNNING;
	}
	if (this->canCast()) {
		//set WindUp
		WindUp& wu = ECS::registry<WindUp>.emplace(this->agent);
		CombatStats& cs = ECS::registry<CombatStats>.get(this->agent);
		wu.castTime = (1 / cs.attacks_per_second) * 1000;
		this->status = RUNNING;
		cs.currMana = 0.f;
		return RUNNING;
	}

	if (ECS::registry<Casting>.has(this->agent)) {
		// reset mana
		CombatStats& cs = ECS::registry<CombatStats>.get(this->agent);
		//cast ability
		Motion& m = ECS::registry<Motion>.get(this->agent);
		ECS::Entity target = cs.target;
		Motion& m2 = ECS::registry<Motion>.get(target);
		CombatStats& target_cs = ECS::registry<CombatStats>.get(target);
		Particle::createParticle(m.position, { 50.,50. }, "fist_particle", 200, 200, 0.15, 0, 0, -50, -50);
		Unit::damageUnit(target, 3 * cs.attack);
		if (!ECS::registry<Squish>.has(target)) {
			Squish& s = ECS::registry<Squish>.emplace(target);
			s.old_x_scale = m2.scale[0];
			s.old_y_scale = m2.scale[1];
		}
		WindDown& wd = ECS::registry<WindDown>.emplace(this->agent);
		wd.castTime = (1 / cs.attacks_per_second) * 1000;
		this->status = RUNNING;
	}

	return FAILED;
}
