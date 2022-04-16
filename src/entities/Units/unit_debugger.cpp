#include "unit.hpp"
#include "game_components.hpp"
#include "render.hpp"
#include "tiny_ecs.hpp"
#include "unit_debugger.hpp"
#include "ai.hpp"
#include "particle.hpp"
#include "debug.hpp"

#include <iostream>

ECS::Entity UnitDebugger::createUnitDebugger(vec2 boardPosition, BoardInfo bi, WorldPosition wp, Team t) {
	CombatStats cs = CombatStats();
	cs.range = 2;
	cs.totalHp = 150;
	cs.currentHp = cs.totalHp;
	UnitType ut = debugger;
	ECS::Entity entity = Unit::createUnit(boardPosition, bi, wp, t, ut, cs);
	Identity& i = ECS::registry<Identity>.get(entity);
	i.abilityDesc = "Debuggers make bad code better. Upon casting, the Debugger will heal the Ally with the lowest health";
	UnitAI uai = ECS::registry<UnitAI>.get(entity);
	CastNode* cn = new DebuggerCastNode(&entity);
	uai.setCastNode(cn);
	return entity;
}

DebuggerCastNode::DebuggerCastNode(ECS::Entity* agent) {
	this->agent = *agent;
	this->status = IDLE;
	this->isRunning = false;
}

nodeStatus DebuggerCastNode::execute() {
	if (ECS::registry<WindUp>.has(this->agent)) {
		DebugSystem::createLineFromPoints(ECS::registry<Motion>.get(this->agent).position, ECS::registry<Motion>.get(*this->AllyToBeHealed).position, vec3(0.0 ,1.0, 0.0));
		return RUNNING;
	}

	if (ECS::registry<WindDown>.has(this->agent)) {
		return RUNNING;
	}

	if (this->canCast()) {
		//SetUp WindUp
		ECS::registry<WindUp>.emplace(this->agent);
		TeamAffiliation team = ECS::registry<TeamAffiliation>.get(this->agent);
		CombatStats& LowestAlly = ECS::registry<CombatStats>.get(this->agent);
		float min_hp = LowestAlly.currentHp;
		this->AllyToBeHealed = &this->agent;
		for (auto& e : ECS::registry<Unit>.entities) {
			CombatStats& entity_cs = ECS::registry<CombatStats>.get(e);
			TeamAffiliation e_team = ECS::registry<TeamAffiliation>.get(e);
			if (min_hp > entity_cs.currentHp && entity_cs.currentHp > 0 && e_team.t == team.t) {
				this->AllyToBeHealed = &e;
				min_hp = entity_cs.currentHp;
			}
		}
		CombatStats& cs = ECS::registry<CombatStats>.get(this->agent);
		cs.currMana = 0.f;
		return RUNNING;

	}
	if (ECS::registry<Casting>.has(this->agent)) {
		CombatStats& cs = ECS::registry<CombatStats>.get(*this->AllyToBeHealed);
		if (cs.currentHp > 0) {
			//cast ability
			Motion& m = ECS::registry<Motion>.get(*this->AllyToBeHealed);
			cs.currentHp = std::min<int>((cs.currentHp + 20), cs.totalHp);
			for (int i = 0; i < 4; ++i) {
				float rand_x = (rand() % 50) - 25;
				float rand_y = (rand() % 50) - 25;
				float particle_x = m.position.x + rand_x;
				float particle_y = m.position.y + rand_y;
				Particle::createParticle({ particle_x, particle_y }, { 10.,10. }, "heal_particle", 100, 200, -1, 0, 0, -100, -50);
			}
			cs.currentHp = std::min<int>(cs.totalHp, (int)(cs.currentHp + (0.4 * cs.totalHp)));
			ECS::registry<WindDown>.emplace(this->agent);
			this->status = RUNNING;
		}
	}
	return FAILED;
}
