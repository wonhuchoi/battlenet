// internal
#include "level_gen.hpp"
#include "game_components.hpp"
#include "tiny_ecs.hpp"
#include "unit.hpp"
#include "Units/unit_ddos.hpp"
#include "Units/unit_debugger.hpp"
#include "Units/unit_hdd.hpp"
#include "Units/unit_trojan.hpp"
#include "Units/units.hpp"
#include "board.hpp"
#include <iostream>
#include <algorithm>
#include <chrono>

void LevelSystem::generate(ECS::Entity board, std::vector<int> unitVec) {
	BoardInfo& bi = ECS::registry<BoardInfo>.get(board);
	WorldPosition wp = ECS::registry<WorldPosition>.get(board);
	std::cout << "Current Level: " << bi.currLevel << "\n";
	bi.boardUnits = 0;
	makeEnemies(bi, wp, unitVec);
	
	switch (bi.currLevel) {
	case -1: //Tutorial
		bi.unitLimit = 3;
		break;
	case 0: 
		makeAllies(bi, wp);
		bi.boardUnits = 5;
		bi.unitLimit = 7;
		break;
	case 1:
		bi.unitLimit = 4;
		break;
	case 2:
		bi.unitLimit = 5;
		break;
	case 3:
		bi.unitLimit = 6;
		break;
	case 4:
		bi.unitLimit = 7;
		break;
	case 5:
		bi.unitLimit = 8;
		break;
	case 6:
		bi.unitLimit = 9;
		break;
	case 7:
		bi.unitLimit = 10;
		break;
	default:
		break;
	}
}

void LevelSystem::makeEnemies(BoardInfo bi, WorldPosition wp, std::vector<int> unitVec) {
	std::default_random_engine rng = std::default_random_engine(std::random_device()());
	switch (bi.currLevel) {
	case -1: //Tutorial
		UnitDebugger::createUnitDebugger(vec2(3, 1), bi, wp, enemy);
		break;
	case 0:
		UnitDebugger::createUnitDebugger(vec2(2, 1), bi, wp, enemy);
		UnitTrojan::createUnitTrojan(vec2(5, 0), bi, wp, enemy);
		UnitDDOS::createUnitDDOS(vec2(0, 1), bi, wp, enemy);
		UnitUDP::createUnitUDP(vec2(5, 1), bi, wp, enemy);
		break;
	case 1:
		shuffleEnemy(bi, wp, unitVec, 3);
		break;
	case 2:
		shuffleEnemy(bi, wp, unitVec, 4);
		break;
	case 3:
		shuffleEnemy(bi, wp, unitVec, 5);
		break;
	case 4:
		shuffleEnemy(bi, wp, unitVec, 6);
		break;
	case 5:
		shuffleEnemy(bi, wp, unitVec, 7);
		break;
	case 6:
		shuffleEnemy(bi, wp, unitVec, 8);
		break;
	case 7:
		shuffleEnemy(bi, wp, unitVec, 9);
		break;
	case 9:
		UnitDebugger::createUnitDebugger(vec2(2, 2), bi, wp, enemy);
		break;
	default:
		break;
	}
}

void LevelSystem::makeAllies(BoardInfo bi, WorldPosition wp) {
	switch (bi.currLevel) {
	case 0:
		UnitDebugger::createUnitDebugger(vec2(5, 4), bi, wp, ally);
		UnitTrojan::createUnitTrojan(vec2(2, 4), bi, wp, ally);
		UnitUDP::createUnitUDP(vec2(3, 4), bi, wp, ally);
		UnitDDOS::createUnitDDOS(vec2(0, 4), bi, wp, ally);
		UnitHDD::createUnitHDD(vec2(4, 5), bi, wp, ally);
		break;
	case 1:
		UnitDebugger::createUnitDebugger(vec2(5, 4), bi, wp, ally);
		UnitHDD::createUnitHDD(vec2(2, 4), bi, wp, ally);
		UnitTrojan::createUnitTrojan(vec2(4, 5), bi, wp, ally);
		break;
	case 2:
		UnitTrojan::createUnitTrojan(vec2(5, 4), bi, wp, ally);
		UnitDDOS::createUnitDDOS(vec2(2, 4), bi, wp, ally);
		UnitDebugger::createUnitDebugger(vec2(4, 5), bi, wp, ally);
		UnitHDD::createUnitHDD(vec2(6, 4), bi, wp, ally);
		UnitTrojan::createUnitTrojan(vec2(1, 4), bi, wp, ally);
		UnitTrojan::createUnitTrojan(vec2(0, 5), bi, wp, ally);
		break;

	case 3:
		UnitTrojan::createUnitTrojan(vec2(6, 4), bi, wp, ally);
		UnitDDOS::createUnitDDOS(vec2(1, 4), bi, wp, ally);
		UnitHDD::createUnitHDD(vec2(0, 5), bi, wp, ally);
		break;
	case 4:
		UnitDebugger::createUnitDebugger(vec2(4, 5), bi, wp, ally);
		UnitDDOS::createUnitDDOS(vec2(6, 4), bi, wp, ally);
		UnitTrojan::createUnitTrojan(vec2(1, 4), bi, wp, ally);
		UnitTrojan::createUnitTrojan(vec2(0, 5), bi, wp, ally);
		break;
	default:
		break;
	}
}

void LevelSystem::shuffleEnemy(BoardInfo bi, WorldPosition wp, std::vector<int> unitVec, int count) {
	std::default_random_engine rng = std::default_random_engine(std::random_device()());
	rng.seed(std::chrono::system_clock::now().time_since_epoch().count());
	std::random_shuffle(unitVec.begin(), unitVec.end());
	std::vector<vec2> positions;
	while (count--) {
		UnitType ut = static_cast<UnitType>(unitVec[0]);
		vec2 enemyPos = vec2(rand() % int(bi.boardSize.x), rand() % int(bi.boardSize.y - 3));
		while (find(positions.begin(), positions.end(), enemyPos) != positions.end()) {
			enemyPos = vec2(rand() % int(bi.boardSize.x), rand() % int(bi.boardSize.y - 3));
		}
		positions.push_back(enemyPos);
		switch (ut) {
		case debugger:
			UnitDebugger::createUnitDebugger(enemyPos, bi, wp, enemy);
			break;
		case trojan:
			UnitTrojan::createUnitTrojan(enemyPos, bi, wp, enemy);
			break;
		case hdd:
			UnitHDD::createUnitHDD(enemyPos, bi, wp, enemy);
			break;
		case ddos:
			UnitDDOS::createUnitDDOS(enemyPos, bi, wp, enemy);
			break;
		case udp:
			UnitUDP::createUnitUDP(enemyPos, bi, wp, enemy);
			break;
		case dhcp:
			UnitDHCP::createUnitDHCP(enemyPos, bi, wp, enemy);
			break;
		case wifi:
			UnitWifi::createUnitWifi(enemyPos, bi, wp, enemy);
			break;
		case cpu:
			UnitCPU::createUnitCPU(enemyPos, bi, wp, enemy);
			break;
		case phishing:
			UnitPhishing::createUnitPhishing(enemyPos, bi, wp, enemy);
			break;
		case ransomware:
			UnitRansomware::createUnitRansomware(enemyPos, bi, wp, enemy);
			break;
		case ram:
			UnitRAM::createUnitRAM(enemyPos, bi, wp, enemy);
			break;
		default:
			break;
		}
		std::random_shuffle(unitVec.begin(), unitVec.end());
	}
}
