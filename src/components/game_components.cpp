#include "game_components.hpp"
#include <string>
#include <unordered_map>
#include <iostream>
#include <../ext/nlohmann/json.hpp>
#include <vector>


using json = nlohmann::json;
// Maps of enum keys -> string
// FeelsBadMan: https://stackoverflow.com/questions/28828957/enum-to-string-in-modern-c11-c14-c17-and-future-c20

const std::string teamArr[] = {
    "ally",
    "enemy"
};

std::string teamToString(Team t) {
    return teamArr[t];
}

const std::vector<std::string> unitArr = {
    "debugger", // Healer
    "trojan",   // Brawler (Powerful attack)
    "hdd",      // Tank (Invincibility)
    "ddos",     // Controller (Freeze enemy)
	"udp",		// Throw projectile at farthest character (can miss)
	"dhcp",
	"wifi",
	"cpu",
	"phishing",
	"ransomware",
	"ram",
    // --------- IN THE ENUM AND IN THIS ARRAY DO NOT PUT UNITS BELOW THIS LINE ------------
    "playerCharacter",
    "enemyCharacter"
};

const std::vector<int> unitCostArr = {
    15,
    10,
    15,
    10,
    20,
    25,
    25,
    15,
    20,
    15,
    10
};

std::vector<std::string> getUnits() {
    return unitArr;
}

std::string unitToString(UnitType ut) {
    return unitArr[ut];
}

int getCostForUnit(UnitType ut) {
    return unitCostArr[ut];
}

const std::vector<std::string> classArr = {
    "software",
    "malware",
    "hardware",
    "network",
	"protocol",
	"memory"
};

const std::vector<std::string> classTooltipText = {
    "Software:\nActivate this class buff for extra attack speed!",
    "Malware:\nActivate this class buff for extra attack damage!",
    "Hardware:\nActivate this class buff for extra health!",
    "Network:\nActivate this class buff for extra range!",
	"Protocol:\nActivate this class buff for all-around increased stats!",
	"memory:\nActivate this class buff for an extra unit!"
};

std::vector<std::string> getClasses() {
    return classArr;
}

std::string classToString(ClassType ct) {
    return classArr[ct];
}

std::string getClassText(ClassType ct) {
    return classTooltipText[ct];
}

const Buff softwareBuff = Buff{
    0,          // extraHp 
    0,          // extraAttack
    0,          // extraRange
    0.5,        // extraAttackSpeed
	0,			//extraUnit
    software    // classBuff
};

const Buff malwareBuff = Buff{
    0,          // extraHp 
    5,          // extraAttack
    0,          // extraRange
    0,          // extraAttackSpeed
	0,			//extraUnit
    malware     // classBuff
};

const Buff hardwareBuff = Buff{
    50,         // extraHp 
    0,          // extraAttack
    0,          // extraRange
    0,          // extraAttackSpeed
	0,			//extraUnit
    hardware    // classBuff
};

const Buff networkBuff = Buff{
    0,          // extraHp 
    0,          // extraAttack
    1,          // extraRange
    0,        // extraAttackSpeed
	0,			//extraUnit
    network    // classBuff
};

const Buff protocolBuff = Buff{
    20,          // extraHp 
    2,          // extraAttack
    .5,          // extraRange
    0.2,        // extraAttackSpeed
	0,			//extraUnit
    protocol    // classBuff
};

const Buff memoryBuff = Buff{
	0,          // extraHp 
	0,          // extraAttack
	0.0,          // extraRange
	0.0,        // extraAttackSpeed
	1,			//extraUnit
	protocol    // classBuff
};

std::map<ClassType, Buff> classBuffs = {
    {software, softwareBuff},
    {malware, malwareBuff},
    {hardware, hardwareBuff},
    {network, networkBuff},
	{protocol, protocolBuff},
	{memory, memoryBuff}
};

Buff getClassBuff(ClassType ct) {
    return classBuffs[ct];
}

std::map<ClassType, int> buffLimits = {
    {software, 3},
    {malware, 2},
    {hardware, 2},
	{network, 2},
	{protocol, 2},
	{memory, 1}
};

// Returns the minimum amount of units to get a class buff
int getClassBuffMin(ClassType ct) {
    return buffLimits[ct];
}

std::map<UnitType, std::vector<ClassType>> classMap = {
	{debugger, {software}},
	{trojan, {software, malware}},
	{hdd, {hardware}},
	{ddos, {network, malware}},
	{udp, {network, protocol}},
	{dhcp, {network, protocol}},
	{wifi, {network}},
	{cpu, {hardware}},
	{phishing, {malware, protocol}},
	{ransomware, {malware, software}},
	{ram, {memory, hardware}}
};

std::vector<ClassType> getClassesForUnit(UnitType ut) {
    return classMap[ut];
}

// Needed to init position part of motion components for units.
// Returns the position centered in the cell.
vec2 boardPosToWorldPos(vec2 bp, BoardInfo bi, WorldPosition wp) {
    return vec2(wp.position.x + ((bp.x) * bi.cellSize.x), wp.position.y + ((bp.y) * bi.cellSize.y));
}

// Note, we could also use the functions from GLM but we write the transformations here to show the uderlying math
void Transform::scale(vec2 scale)
{
	mat3 S = { { scale.x, 0.f, 0.f },{ 0.f, scale.y, 0.f },{ 0.f, 0.f, 1.f } };
	mat = mat * S;
}

void Transform::rotate(float radians)
{
	float c = std::cos(radians);
	float s = std::sin(radians);
	mat3 R = { { c, s, 0.f },{ -s, c, 0.f },{ 0.f, 0.f, 1.f } };
	mat = mat * R;
}

void Transform::translate(vec2 offset)
{
	mat3 T = { { 1.f, 0.f, 0.f },{ 0.f, 1.f, 0.f },{ offset.x, offset.y, 1.f } };
	mat = mat * T;
}


json BoardPosition::getPositionJSON() {
	json js = { startPos.x, startPos.y };
	return js;
}

json TeamAffiliation::getTeamJSON() {
    json js;
    js["teamname"] = teamToString(t);
    js["team"] = t;
    return js;
}

json Identity::getIdentityJSON() {
    json js;
    js["unitname"] = unitToString(ut);
    js["unit"] = ut;
    return js;
}

int totalHp = 100;
int currentHp = 100;
int attack = 10;
// Number of attacks a unit can perform per second
int attacks_per_second = 1;

// Number of cells away on the grid that a unit can reach with an attack
int range = 1;


json CombatStats::getCombatStatsJSON() {
    json js;
    js["totalHp"] = totalHp;
    js["currentHp"] = currentHp;
    js["attack"] = attack;
    js["attacks_per_second"] = attacks_per_second;
    js["range"] = range;
    return js;
}

void CombatStats::setCombatStatsJSON(json js) {
    totalHp = js["totalHp"];
    currentHp = js["currentHp"];
    attack = js["attack"];
    attacks_per_second = js["attacks_per_second"];
    range = js["range"];
    return ;
}

