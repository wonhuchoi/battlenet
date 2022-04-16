#include "unit.hpp"
#include "game_components.hpp"
#include "render.hpp"
#include "tiny_ecs.hpp"
#include "unit_cpu.hpp"
#include "ai.hpp"
#include "particle.hpp"

#include <iostream>

ECS::Entity UnitCPU::createUnitCPU(vec2 boardPosition, BoardInfo bi, WorldPosition wp, Team t) {
	CombatStats cs = CombatStats();
	cs.range = 3;
	cs.totalHp = 150;
	cs.currentHp = cs.totalHp;
	cs.attack = 20;
	UnitType ut = cpu;
	ECS::Entity entity = Unit::createUnit(boardPosition, bi, wp, t, ut, cs);
	Identity& i = ECS::registry<Identity>.get(entity);
	i.abilityDesc = "The CPU will overclock, causing itself to attack much faster than normal";
	UnitAI uai = ECS::registry<UnitAI>.get(entity);
	CastNode* cn = new CPUCastNode(&entity);
	uai.setCastNode(cn);
	return entity;
}

CPUCastNode::CPUCastNode(ECS::Entity* agent) {
	this->agent = *agent;
	this->status = IDLE;
	this->isRunning = false;
}

nodeStatus CPUCastNode::execute() {
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
		Motion& m = ECS::registry<Motion>.get(this->agent);
		//cast ability
		cs.attacks_per_second *= 1.3;
		Particle::createParticle(m.position, { 50.,50. }, "overclock", 700, 700, 0.1, 0, 0, -50, -50);
		ECS::registry<WindDown>.emplace(this->agent);
		this->status = RUNNING;
	}

	return FAILED;
}
