#include "unit.hpp"
#include "game_components.hpp"
#include "render.hpp"
#include "tiny_ecs.hpp"
#include "unit_ddos.hpp"
#include "ai.hpp"
#include "particle.hpp"
#include "projectile.hpp"

#include <iostream>

ECS::Entity UnitDDOS::createUnitDDOS(vec2 boardPosition, BoardInfo bi, WorldPosition wp, Team t) {
	CombatStats cs = CombatStats();
	//setting combat Stats
	cs.range = 1;
	cs.totalHp = 400;
	cs.currentHp = cs.totalHp;
	cs.attacks_per_second = 1;
	cs.attack = 15;
	UnitType ut = ddos;
	ECS::Entity entity = Unit::createUnit(boardPosition, bi, wp, t, ut, cs);
	Identity& i = ECS::registry<Identity>.get(entity);
	i.abilityDesc = "Upon cast, DDOS will freeze all adjacent enemies for a short duration";
	UnitAI uai = ECS::registry<UnitAI>.get(entity);
	CastNode* cn = new DDOSCastNode(&entity);
	uai.setCastNode(cn);
	return entity;
}

DDOSCastNode::DDOSCastNode(ECS::Entity* agent) {
	this->agent = *agent;
	this->status = IDLE;
	this->isRunning = false;
}

nodeStatus DDOSCastNode::execute() {
	if (ECS::registry<WindUp>.has(this->agent)) {
		return RUNNING;
	}

	if (ECS::registry<WindDown>.has(this->agent)) {
		return RUNNING;
	}
	if (this->canCast()) {
		//set WindUp
		CombatStats& cs = ECS::registry<CombatStats>.get(this->agent);
		Motion& m = ECS::registry<Motion>.get(this->agent);
		this->status = RUNNING;
		ECS::registry<WindUp>.emplace(this->agent);
		TeamAffiliation team = ECS::registry<TeamAffiliation>.get(this->agent);

		//find adjacent enemies
		for (auto& e : ECS::registry<Unit>.entities) {
			Motion& e_m = ECS::registry<Motion>.get(e);
			CombatStats& cs = ECS::registry<CombatStats>.get(e);
			TeamAffiliation e_team = ECS::registry<TeamAffiliation>.get(e);
			float dist = length(m.position - e_m.position);
			if (dist <= 150 && e_team.t != team.t && cs.currentHp > 0) {
				this->adj_enemies.push_back(&e);
			}
		}
		Particle::createParticles(30, m.position,vec2(20.,20.), "snowflake_particle", 600.,600., 0,-150.,150., -150, 150);
		cs.currMana = 0.f;
		return RUNNING;
	}

	if (ECS::registry<Casting>.has(this->agent)) {
		for (auto& e : this->adj_enemies) {
			if (!ECS::registry<Freeze>.has(*e)) {
				Freeze& f = ECS::registry<Freeze>.emplace(*e);
				Motion& m = ECS::registry<Motion>.get(*e);
				Particle::createParticles(1, m.position, vec2(50., 50.), "snowflake_particle", f.totalTime, f.totalTime, 0, 0., 0, 0, 0);
			}
		}
		ECS::registry<WindDown>.emplace(this->agent);
		this->adj_enemies.clear();

		this->status = RUNNING;
	}

	return FAILED;
}
