#include "unit.hpp"
#include "game_components.hpp"
#include "render.hpp"
#include "tiny_ecs.hpp"
#include "unit_phishing.hpp"
#include "ai.hpp"
#include "unit_cpu.hpp"
#include "unit_dhcp.hpp"
#include "unit_udp.hpp"
#include "unit_hdd.hpp"
#include "unit_wifi.hpp"
#include "unit_debugger.hpp"
#include "particle.hpp"
#include "debug.hpp"

#include <iostream>

ECS::Entity UnitPhishing::createUnitPhishing(vec2 boardPosition, BoardInfo bi, WorldPosition wp, Team t) {
	CombatStats cs = CombatStats();
	cs.range = 2;
	cs.totalHp = 300;
	cs.currentHp = cs.totalHp;
	UnitType ut = phishing;
	ECS::Entity entity = Unit::createUnit(boardPosition, bi, wp, t, ut, cs);
	Identity& i = ECS::registry<Identity>.get(entity);
	i.abilityDesc = "Phishing attacks are not always what they seem, upon cast, they will cast a random ability";
	UnitAI uai = ECS::registry<UnitAI>.get(entity);
	CastNode* cn = new PhishingCastNode(&entity);
	uai.setCastNode(cn);
	return entity;
}

PhishingCastNode::PhishingCastNode(ECS::Entity* agent) {
	this->agent = *agent;
	this->status = IDLE;
	this->isRunning = false;
	this->randomizer = NULL;
	this->runningIndex = -1;
}

nodeStatus PhishingCastNode::execute() {
	if (this->randomizer == NULL) {
		std::vector<AINode*> vec;
		ECS::Entity* unit = &this->agent;
		HDDCastNode* hdd = new HDDCastNode(unit);
		UDPCastNode* udp = new UDPCastNode(unit);
		DebuggerCastNode* db = new DebuggerCastNode(unit);
		DHCPCastNode* dhcp = new DHCPCastNode(unit);
		WifiCastNode* wifi = new WifiCastNode(unit);
		vec.push_back(hdd);
		vec.push_back(udp);
		vec.push_back(db);
		vec.push_back(dhcp);
		vec.push_back(wifi);
		Randomizer* r = new Randomizer(vec, unit);
		this->randomizer = r;
	}
	return this->randomizer->execute();
}
