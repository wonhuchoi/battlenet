#include "unit.hpp"
#include "game_components.hpp"
#include "render.hpp"
#include "tiny_ecs.hpp"
#include "unit_hdd.hpp"
#include "ai.hpp"
#include "particle.hpp"
#include "projectile.hpp"

#include <iostream>

ECS::Entity UnitHDD::createUnitHDD(vec2 boardPosition, BoardInfo bi, WorldPosition wp, Team t) {
	CombatStats cs = CombatStats();
	//setting combat Stats
	cs.range = 1;
	cs.totalHp = 300;
	cs.currentHp = cs.totalHp;
	cs.attacks_per_second = 1;
	cs.attack = 15;
	UnitType ut = hdd;
	ECS::Entity entity = Unit::createUnit(boardPosition, bi, wp, t, ut, cs);
	Identity& i = ECS::registry<Identity>.get(entity);
	i.abilityDesc = "Its tough exterior makes the HDD more resiliant. Upon casting, the HDD will become invincible for a short duration";
	UnitAI uai = ECS::registry<UnitAI>.get(entity);
	CastNode* cn = new HDDCastNode(&entity);
	uai.setCastNode(cn);
	return entity;
}

HDDCastNode::HDDCastNode(ECS::Entity* agent) {
	this->agent = *agent;
	this->status = IDLE;
	this->isRunning = false;
}

nodeStatus HDDCastNode::execute() {
	if (ECS::registry<WindUp>.has(this->agent)) {
		return RUNNING;
	}

	if (ECS::registry<WindDown>.has(this->agent)) {
		return RUNNING;
	}

	if (this->canCast()) {
		//set WindUp
		CombatStats& cs = ECS::registry<CombatStats>.get(this->agent);
		this->status = RUNNING;
		ECS::registry<WindUp>.emplace(this->agent);
		cs.currMana = 0.f;
		return RUNNING;
	}

	if (ECS::registry<Casting>.has(this->agent)) {
		if(!ECS::registry<Immune>.has(this->agent))
			ECS::registry<Immune>.emplace(this->agent);
		else {
			Immune& i = ECS::registry<Immune>.get(this->agent);
			i.timeImmune = 0;
		}
		Motion& m = ECS::registry<Motion>.get(this->agent);
		
		for (int i = 0; i < 9; i++) {
			float rand_x = (rand() % 50) - 25;
			float rand_y = (rand() % 50) - 25;
			float particle_x = m.position.x + rand_x;
			float particle_y = m.position.y + rand_y;
			Particle::createParticles(1, vec2(particle_x, particle_y), vec2(20., 20.), "green_arrow", 500, 500, 0, 0, 0, -100, -50);
		}
		Particle::createParticles(1, m.position,vec2(50.,50.), "shield_particle", 600,600, 0,0,0., -50, -50);
		WindDown& wd = ECS::registry<WindDown>.emplace(this->agent);
		wd.castTime = 3000.;
		this->status = RUNNING;
		return RUNNING;
	}
	return FAILED;
}
