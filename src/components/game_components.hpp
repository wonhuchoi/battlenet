#pragma once
#include "common.hpp"
#include "tiny_ecs.hpp"
#include <../ext/nlohmann/json.hpp>
#include <map>
#include <unordered_set>


using json = nlohmann::json;

// Denotes the pixel position of top left of static objects
// e.g. The board may be static at position (200, 100)
struct WorldPosition {
	vec2 position;
	vec2 worldSize = world_size;
};

// Stores size of board and location in space
// There is only one board in the simple version of our game
struct BoardInfo {
	// Size of each board cell in pixels.
	vec2 cellSize = { 100, 100 };
	// Size of the board in cells.
	vec2 boardSize = { 7, 6 };
	// 2d array containing the cell entities
	ECS::Entity** cells;
	// current level
	int currLevel = 1;
	// starting gold
	int gold = 64;
	int unitLimit = 0;
	int boardUnits = 0;
};

// Stores position of a unit on the board
struct BoardPosition {
	// The initial position of a unit on the board (set during setup phase)
	vec2 startPos;
	// The current position of a unit on the board
	vec2 bPos;
	// The intended future position of a unit on the board ({-1, -1} if no future position set)
	vec2 futurePos = { -1, -1 };
	// Is this position on the bench?
	bool isBench = false;
	json getPositionJSON();
};
vec2 boardPosToWorldPos(vec2 bp, BoardInfo bi, WorldPosition wp);

// Stores position of a unit on the bench
struct BenchPosition {
	int idx;
};

enum Action {
	CAST,
	ATTACK,
	MOVE
};

struct Casting {
	float castTime = 400.f;
	float timeCasting = 0.f;
};

struct Freeze {
	float totalTime = 1000.f;
	float elapsed_ms = 0.f;
};


struct Immune {
	float duration = 1000;
	float timeImmune = 0.f;
};

struct WindUp {
	float castTime = 500.f;
	float timeCasting = 0.f;
};

struct WindDown {
	float castTime = 500.f;
	float timeCasting = 0.f;
};

struct Bind {
	ECS::Entity* bound_unit;
};

// TODO (#3): Add all unit/player types
// UPDATE unitArr IN game_components.cpp FOR STRING REPRESENTATION FUNCTION IF UPDATING THIS ENUM!!!
enum UnitType { 
	debugger, 
	trojan,
	hdd, 
	ddos, 
	udp, 
	dhcp, 
	wifi, 
	cpu, 
	phishing,
	ransomware,
	ram,
	// ---------------------- DO NOT PUT UNITS BELOW THIS LINE ----------------------
	playerCharacter,
	enemyCharacter 
};
std::string unitToString(UnitType ut);
int getCostForUnit(UnitType ut);
std::vector<std::string> getUnits();


enum ClassType { software, malware, hardware, network, protocol, memory, END_CLASSTYPE_ENUM };
std::string classToString(ClassType ct);
std::string getClassText(ClassType ct);
int getClassBuffMin(ClassType ct);
std::vector<std::string> getClasses();
std::vector<ClassType> getClassesForUnit(UnitType ut);

struct Buff {
	int extraHp = 0;
	int extraAttack = 0;
	float extraRange = 0;
	float extraAttackSpeed = 0;
	int extra_unit = 0;
	ClassType classBuff;
	bool operator==(const Buff &other) const {
		return 	extraHp == other.extraHp &&
				  	extraAttack == other.extraAttack &&
						extraRange == other.extraRange &&
						extraAttackSpeed == other.extraAttackSpeed &&
						classBuff == other.classBuff; 
	}
};

Buff getClassBuff(ClassType ct);

struct CombatStats {
	int totalHp = 100;
	int currentHp = 100;
	int attack = 10;
	int manaToCast = 50;
	float skill_duration = 400;
	int currMana = 0;
	// Number of attacks a unit can perform per second
	float attacks_per_second = 1.f;
	float time_from_last_attack = 0.f;
	// ms until next attack available
	float attack_cooldown = 0;
	// Number of cells away on the grid that a unit can reach with an attack
	int range = 1;
	// Targeted entity ref (so the unit isn't always switching targets)
	bool hasTarget = false;

	// Currently active buffs on these combatstats
	std::vector<Buff> currentBuffs;	
	ECS::Entity target;
	Action action = CAST;
	json getCombatStatsJSON();
	void setCombatStatsJSON(json js);
};

// UPDATE teamArr IN game_components.cpp FOR STRING REPRESENTATION FUNCTION IF UPDATING THIS ENUM!!!
enum CellType { allyCell, enemyCell, benchCell, shopCell };
// Stores team of a unit and player
struct CellStruct {
	CellType ct;
	bool isBench = false;
	bool isOccupied = false;
};

// UPDATE teamArr IN game_components.cpp FOR STRING REPRESENTATION FUNCTION IF UPDATING THIS ENUM!!!
enum Team { ally, enemy };
std::string teamToString(Team t);
// Stores team of a unit and player
struct TeamAffiliation {
	Team t = ally;
	json getTeamJSON();
};
struct Identity {
	UnitType ut = debugger;
	std::vector<ClassType> classes;
	json getIdentityJSON();
	std::string abilityDesc;
};

struct PlanningPhase {
	int time_counter = 0;
};

struct BattlePhase {
	int time_counter = 0;
};

struct VictoryPhase {
	int score = 0;
};

struct DefeatPhase {
	int score = 0;
};

// The 'Transform' component handles transformations passed to the Vertex shader
// (similar to the gl Immediate mode equivalent, e.g., glTranslate()...)
struct Transform {
	mat3 mat = { { 1.f, 0.f, 0.f }, { 0.f, 1.f, 0.f}, { 0.f, 0.f, 1.f} }; // start with the identity
	void scale(vec2 scale);
	void rotate(float radians);
	void translate(vec2 offset);
};

// All data relevant to the shape and motion of entities
struct Motion {
	vec2 initialPosition = {0, 0};
	vec2 position = { 0, 0 };
	float angle = 0;
	vec2 velocity = { 0, 0 };
	vec2 scale = { 10, 10 };
	vec2 destination = { 0, 0 };
	bool is_moving = false;
	float speed = 100.f;
	bool is_in_foreground = false;
	vec4 boundaries = { 0, 0, world_size[0], world_size[1] }; // x_min, y_min, x_max, y_max
};

struct Active {
};

struct Missile {
	ECS::Entity* target;
	float angular_momentum;
	float weight;
};

struct Animated {
};

struct Explode {
	float radius = 200;
};

// All data relevant to ImGui description
struct ImGuiDesc {
	vec2 rel_position_from_center = vec2(0, 0);
	std::vector<std::string> description;
	bool activated = false;
};


// determines magnitude of parallax scrolling
struct Parallax {
	float MAX_DISTANCE = 5.0f;
	// greater distance -> less movement
	float distance = 0.0f; // 0.0f <= distance <= MAX_DISTANCE
};

struct Cost {
	int cost = 0;
};

struct ExtraUnit {
	Team t;
};

struct Squish {
	float x_ratio = 1.3;
	float y_ratio = 0.7;
	float elapsed_ms = 0;
	float max_ms = 75.;
	int phases = 3;
	float dampening = 0.8;
	float old_x_scale;
	float old_y_scale;
};
