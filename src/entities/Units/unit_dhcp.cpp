#include "unit.hpp"
#include "game_components.hpp"
#include "render.hpp"
#include "tiny_ecs.hpp"
#include "unit_dhcp.hpp"
#include "ai.hpp"
#include "particle.hpp"
#include "projectile.hpp"

#include <iostream>

ECS::Entity UnitDHCP::createUnitDHCP(vec2 boardPosition, BoardInfo bi, WorldPosition wp, Team t) {
	CombatStats cs = CombatStats();
	//setting combat Stats
	cs.range = 2;
	cs.totalHp = 200;
	cs.currentHp = cs.totalHp;
	cs.attacks_per_second = 1.5;
	cs.attack = 15;
	UnitType ut = dhcp;
	ECS::Entity entity = Unit::createUnit(boardPosition, bi, wp, t, ut, cs);
	Identity& i = ECS::registry<Identity>.get(entity);
	i.abilityDesc = "DHCP will fire a missile to an unsuspecting enemy unit, the missile will explode on impact dealing damage to nearby enemy units. (Ability can miss)";
	UnitAI uai = ECS::registry<UnitAI>.get(entity);
	CastNode* cn = new DHCPCastNode(&entity);
	uai.setCastNode(cn);
	return entity;
}

DHCPCastNode::DHCPCastNode(ECS::Entity* agent) {
	this->agent = *agent;
	this->status = IDLE;
	this->isRunning = false;
}

nodeStatus DHCPCastNode::execute() {
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
		Projectile::createMissile(m.position, "missile", this->cast_target, 10., 300, team.t, 100, 0, 10, true);
		ECS::registry<WindDown>.emplace(this->agent);
		this->status = RUNNING;
	}

	return FAILED;
}

bool DHCPCastNode::canCast() {
	CombatStats& cs = ECS::registry<CombatStats>.get(this->agent);
	return cs.currMana >= cs.manaToCast;
}
