#include "unit.hpp"
#include "game_components.hpp"
#include "render.hpp"
#include "tiny_ecs.hpp"
#include "unit_wifi.hpp"
#include "ai.hpp"
#include "particle.hpp"
#include "projectile.hpp"

#include <iostream>

ECS::Entity UnitWifi::createUnitWifi(vec2 boardPosition, BoardInfo bi, WorldPosition wp, Team t) {
	CombatStats cs = CombatStats();
	//setting combat Stats
	cs.range = 1;
	cs.totalHp = 300;
	cs.currentHp = cs.totalHp;
	cs.attacks_per_second = 1.5;
	cs.attack = 15;
	UnitType ut = wifi;
	ECS::Entity entity = Unit::createUnit(boardPosition, bi, wp, t, ut, cs);
	Identity& i = ECS::registry<Identity>.get(entity);
	i.abilityDesc = "Wifi is in the air, packets will orbit the unit unit they hit enemy units. Signal is stronger the closeer packets are to the source.";
	UnitAI uai = ECS::registry<UnitAI>.get(entity);
	CastNode* cn = new WifiCastNode(&entity);
	uai.setCastNode(cn);
	return entity;
}

WifiCastNode::WifiCastNode(ECS::Entity* agent) {
	this->agent = *agent;
	this->status = IDLE;
	this->isRunning = false;
}

nodeStatus WifiCastNode::execute() {
	if (ECS::registry<WindUp>.has(this->agent)) {
		return RUNNING;
	}

	if (ECS::registry<WindDown>.has(this->agent)) {
		return RUNNING;
	}
	if (this->canCast()) {
		this->status = RUNNING;
		CombatStats& cs = ECS::registry<CombatStats>.get(this->agent);
		ECS::registry<WindUp>.emplace(this->agent);
		cs.currMana = 0.f;
		return RUNNING;
	}

	if (ECS::registry<Casting>.has(this->agent)) {
		TeamAffiliation team = ECS::registry<TeamAffiliation>.get(this->agent);
		//cast ability
		Motion& m = ECS::registry<Motion>.get(this->agent);
		float max_dist = 0.;
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
		for (int i = 0; i < 20; i++) {
			float angle = ((double)rand() / (RAND_MAX)) * 2 * PI;
			mat2 mtx = mat2(cos(angle), -sin(angle), sin(angle), cos(angle));
			vec2 dir = vec2(1, 0) * mtx;
			Projectile::createMissile(m.position + (dir * 100.f), "projectile", &this->agent, 3., 250, team.t, 20, 1, 1.4, false);
		}
		ECS::registry<WindDown>.emplace(this->agent);
		this->status = RUNNING;
	}

	return FAILED;
}

bool WifiCastNode::canCast() {
	CombatStats& cs = ECS::registry<CombatStats>.get(this->agent);
	return cs.currMana >= cs.manaToCast;
}
