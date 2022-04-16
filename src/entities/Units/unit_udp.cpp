#include "unit.hpp"
#include "game_components.hpp"
#include "render.hpp"
#include "tiny_ecs.hpp"
#include "unit_udp.hpp"
#include "ai.hpp"
#include "particle.hpp"
#include "projectile.hpp"

#include <iostream>

ECS::Entity UnitUDP::createUnitUDP(vec2 boardPosition, BoardInfo bi, WorldPosition wp, Team t) {
	CombatStats cs = CombatStats();
	//setting combat Stats
	cs.range = 3;
	cs.totalHp = 150;
	cs.currentHp = cs.totalHp;
	cs.attacks_per_second = 1.5;
	cs.attack = 15;
	UnitType ut = udp;
	ECS::Entity entity = Unit::createUnit(boardPosition, bi, wp, t, ut, cs);
	Identity& i = ECS::registry<Identity>.get(entity);
	i.abilityDesc = "UDP tosses a packet to the farthest enemy. (Ability can miss)";
	UnitAI uai = ECS::registry<UnitAI>.get(entity);
	CastNode* cn = new UDPCastNode(&entity);
	uai.setCastNode(cn);
	return entity;
}

UDPCastNode::UDPCastNode(ECS::Entity* agent) {
	this->agent = *agent;
	this->status = IDLE;
	this->isRunning = false;
}

nodeStatus UDPCastNode::execute() {
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
		float max_dist = 1500.;

		//find farthest enemy
		for (auto& e : ECS::registry<Unit>.entities) {
			CombatStats& entity_cs = ECS::registry<CombatStats>.get(e);
			TeamAffiliation e_team = ECS::registry<TeamAffiliation>.get(e);
			Motion& e_m = ECS::registry<Motion>.get(e);
			
			float dist = length(m.position - e_m.position);
			if (dist > max_dist && e_team.t != team.t && entity_cs.currentHp > 0) {
				max_dist = dist;
				this->cast_target = &e;
			}
		}
		cs.currMana = 0.f;
		return RUNNING;
	}

	if (ECS::registry<Casting>.has(this->agent)) {
		CombatStats& cs = ECS::registry<CombatStats>.get(this->agent);
		TeamAffiliation team = ECS::registry<TeamAffiliation>.get(this->agent);
		//cast ability
		Motion& m = ECS::registry<Motion>.get(this->agent);
		ECS::Entity target = cs.target;
		Motion& target_m = ECS::registry<Motion>.get(target);
		vec2 dir = glm::normalize(target_m.position - m.position);
		dir *= 500;
		Projectile::createProjectile(m.position, 10., dir, team.t, 15, vec3(0.6f, 1.f, 0.6f));
		ECS::registry<WindDown>.emplace(this->agent);
		this->cast_target = NULL;
		this->status = RUNNING;
	}

	return FAILED;
}

bool UDPCastNode::canCast() {
	CombatStats& cs = ECS::registry<CombatStats>.get(this->agent);
	if (cs.currMana < cs.manaToCast)
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
