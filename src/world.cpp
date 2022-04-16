// Header
#include "world.hpp"
#include "physics.hpp"
#include "debug.hpp"
#include "render_components.hpp"
#include "game_components.hpp"
#include "pause.hpp"
#include "board.hpp"
#include "unit.hpp"
#include "physics.hpp"
#include "projectile.hpp"
#include "common.hpp"
#include "tiny_ecs.hpp"
#include "render.hpp"
#include "util.hpp"
#include "space_invader.hpp"
#include "particle.hpp"
#include "level_gen.hpp"
#include "Units/unit_ddos.hpp"
#include "Units/unit_debugger.hpp"
#include "Units/unit_hdd.hpp"
#include "Units/unit_trojan.hpp"
#include "Units/units.hpp"

// stlib
#include <algorithm>
#include <string.h>
#include <cassert>
#include <sstream>
#include <limits>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>

#include <../ext/nlohmann/json.hpp>
#include <chrono>

// for convenience
using json = nlohmann::json;

// Create the world
// Note, this has a lot of OpenGL specific things, could be moved to the renderer; but it also defines the callbacks to the mouse and keyboard. That is why it is called here.
WorldSystem::WorldSystem(ivec2 window_size_px)
{
	// Seeding rng with random device
	rng = std::default_random_engine(std::random_device()());

	///////////////////////////////////////
	// Initialize GLFW
	auto glfw_err_callback = [](int error, const char* desc) { std::cerr << "OpenGL:" << error << desc << std::endl; };
	glfwSetErrorCallback(glfw_err_callback);
	if (!glfwInit())
		throw std::runtime_error("Failed to initialize GLFW");

	//-------------------------------------------------------------------------
	// GLFW / OGL Initialization, needs to be set before glfwCreateWindow
	// Core Opengl 3.
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, 1);
#if __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
	glfwWindowHint(GLFW_RESIZABLE, 0);

	// Create the main window (for rendering, keyboard, and mouse input)
	window = glfwCreateWindow(window_size_px.x, window_size_px.y, "BattleNet", nullptr, nullptr);
	if (window == nullptr)
		throw std::runtime_error("Failed to glfwCreateWindow");

	// Setting callbacks to member functions (that's why the redirect is needed)
	// Input is handled using GLFW, for more info see
	// http://www.glfw.org/docs/latest/input_guide.html
	glfwSetWindowUserPointer(window, this);
	auto key_redirect = [](GLFWwindow* wnd, int _0, int _1, int _2, int _3) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_key(_0, _1, _2, _3); };
	auto cursor_pos_redirect = [](GLFWwindow* wnd, double _0, double _1) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_mouse_move({ _0, _1 }); };
	auto cursor_click_redirect = [](GLFWwindow* wnd, int _0, int _1, int _2) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_mouse_click(_0, _1, _2); };

	glfwSetKeyCallback(window, key_redirect);
	glfwSetCursorPosCallback(window, cursor_pos_redirect);
	glfwSetMouseButtonCallback(window, cursor_click_redirect);

	// Playing background music indefinitely
	init_audio();
	Mix_PlayMusic(background_music, -1);
	std::cout << "Loaded music\n";

	// Init buff system
	allyBuffs = BuffManager(ally);
	enemyBuffs = BuffManager(enemy);
	for (uint i = 0; i < getUnits().size() - 2; i++) {
		unitVec.push_back(i);
	}
	shuffle_shop();
}

WorldSystem::~WorldSystem()
{
	// Destroy music components
	if (background_music != nullptr)
		Mix_FreeMusic(background_music);
	Mix_CloseAudio();

	// Destroy all created components
	ECS::ContainerInterface::clear_all_components();

	// Close the window
	glfwDestroyWindow(window);
}

void WorldSystem::init_audio()
{
	//////////////////////////////////////
	// Loading music and sounds with SDL
	if (SDL_Init(SDL_INIT_AUDIO) < 0)
		throw std::runtime_error("Failed to initialize SDL Audio");

	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1)
		throw std::runtime_error("Failed to open audio device");

	int numBgTracks = 2;
	// randomly loads an audio track with format bg-{{ 0 ... numBgTracks - 1 }}.wav
	background_music = Mix_LoadMUS(audio_path("bg-" + std::to_string(rand() % numBgTracks) + ".wav").c_str());
	Mix_Volume(1, 25); // default channel is 1, volume ranges from 0-128

	if (background_music == nullptr)
		throw std::runtime_error("Failed to load sounds make sure the data directory is present: " + audio_path("music.wav"));
}

//Updates the ImGUI UI for the unit stats description
void updateUnitStats()
{
	auto& unit_container = ECS::registry<Unit>;

	for (unsigned int i = 0; i < unit_container.components.size(); i++)
	{
		ECS::Entity entity = unit_container.entities[i];
		auto& ig_desc = ECS::registry<ImGuiDesc>.get(entity);
		auto& team = ECS::registry<TeamAffiliation>.get(entity);
		auto& cs = ECS::registry<CombatStats>.get(entity);
		auto& identity = ECS::registry<Identity>.get(entity);

		ig_desc.description = {};
		std::string title = std::string("Unit Stats");
		std::string teamDesc = std::string("Team: ") + teamToString(team.t);
		std::string utDesc = std::string("Unit Type: ") + unitToString(identity.ut);
		std::string hpDesc = std::string("HP: ") + std::to_string(cs.currentHp) + std::string(" / ") + std::to_string(cs.totalHp);
		std::string attackDesc = std::string("Attack: ") + std::to_string(cs.attack);
		std::string attackSPDesc = std::string("Attack Speed: ") + std::to_string(cs.attacks_per_second);
		std::string rangeDesc = std::string("Range: ") + std::to_string(cs.range);
		std::string abilityDesc = std::string("Ability: ") + identity.abilityDesc;;

		ig_desc.description.push_back(title);
		ig_desc.description.push_back(teamDesc);
		ig_desc.description.push_back(utDesc);
		ig_desc.description.push_back(hpDesc);
		ig_desc.description.push_back(attackDesc);
		ig_desc.description.push_back(attackSPDesc);
		ig_desc.description.push_back(rangeDesc);
		ig_desc.description.push_back(abilityDesc);
	}
}

// Update our game world
void WorldSystem::step(float elapsed_ms, vec2 window_size_in_game_units)
{
	// TODO handle updates to game state
	if (worldState != WORLD_START) {
		notifyObservers();
		updateUnitStats();
	}

	if (worldState == WORLD_VICTORY) {
		victory_counter_ms -= elapsed_ms;
		if (victory_counter_ms < 0) {
			next_level();
		}
	}
	else if (worldState == WORLD_DEFEAT) {
		defeat_counter_ms -= elapsed_ms;
		if (defeat_counter_ms < 0) {
			if (in_playground) {
				next_level();
			}
			defeat_counter_ms = 30000;
			restart();
		}
	}
}

void WorldSystem::shuffle_shop() {
	std::default_random_engine rng = std::default_random_engine(std::random_device()());
	rng.seed(std::chrono::system_clock::now().time_since_epoch().count());
	std::random_shuffle(unitVec.begin(), unitVec.end());
	for (int i = 0; i < 4; i++) {
		disabledShopIndexes[i] = false;
	}
}

bool WorldSystem::shopIndexDisabled(int i) {
	if (in_playground) {
		return false;
	}
	// Shop only has 4 units
	if (i > 3) {
		return true;
	}
	return disabledShopIndexes[i];
}

void WorldSystem::disableShopIndex(int i) {
	if (!in_playground) {
		disabledShopIndexes[i] = true;
	}
}

void WorldSystem::next_level() {

	ECS::registry<VictoryPhase>.remove(board);
	ECS::registry<PlanningPhase>.emplace(board);

	worldState = WORLD_CAMPAIGN;
	if (in_playground) {
		WorldSystem::load_level("last.json");
	} else {
		// Shuffle the units for shop
		shuffle_shop();
		if (ECS::registry<BoardInfo>.get(board).currLevel == 7) {
			ECS::registry<BoardInfo>.get(board).currLevel = 1;
		}
		else {
			ECS::registry<BoardInfo>.get(board).currLevel += 1;
		}
		open_level(ECS::registry<BoardInfo>.get(board).currLevel);
	}
	victory_counter_ms = 15000;
}

//camera
vec2 WorldSystem::get_cam_transform()
{
	vec2 cameraTransform = { 0, 0 };

	if (worldState == WORLD_CAMPAIGN)
	{
		auto& player_motion = ECS::registry<Motion>.get(player);

		if (player_motion.position.x < camera_bound[0]) // left
		{
			cameraTransform.x = camera_bound[0] - player_motion.position.x;
		}
		if (player_motion.position.x > camera_bound[1]) // right
		{
			cameraTransform.x = camera_bound[1] - player_motion.position.x;
		}
		if (player_motion.position.y < camera_bound[2]) // top
		{
			cameraTransform.y = camera_bound[2] - player_motion.position.y;
		}
		if (player_motion.position.y > camera_bound[3]) // bottom
		{
			cameraTransform.y = camera_bound[3] - player_motion.position.y;
		}
	}

	return cameraTransform;
}

void::WorldSystem::reset_board() {
	std::cout << "Resetting board" << std::endl;
	// TODO: Remove all entities that we created
	while (ECS::registry<Unit>.entities.size() > 0)
		ECS::ContainerInterface::remove_all_components_of(ECS::registry<Unit>.entities.back());

	std::cout << "board phase" << std::endl;
	// If we are in battle phase, revert to planning phase
	if (ECS::registry<BattlePhase>.has(board))
	{
		ECS::registry<BattlePhase>.remove(board);
		ECS::registry<PlanningPhase>.emplace(board);
		revertBoard();
	}

	std::cout << "cells" << std::endl;
	for (auto& cs : ECS::registry<CellStruct>.components) {
		cs.isOccupied = false;
	}
}

// Start Screen
void WorldSystem::restart()
{
	// Debugging for memory/component leaks
	// ECS::ContainerInterface::list_all_components();
	std::cout << "Restarting\n";
	while (ECS::registry<Motion>.entities.size() > 0)
		ECS::ContainerInterface::remove_all_components_of(ECS::registry<Motion>.entities.back());

	WorldSystem::reset_board();

	// Debugging for memory/component leaks
	ECS::ContainerInterface::list_all_components();
	worldState = WORLD_START;
	notifyObservers();
	selected = NULL;
	shuffle_shop();
}

void WorldSystem::revertBoard()
{
	WorldPosition wp = ECS::registry<WorldPosition>.get(board);
	BoardInfo& bi = ECS::registry<BoardInfo>.get(board);
	for (auto e : ECS::registry<Unit>.entities)
	{
		Motion& m = ECS::registry<Motion>.get(e);
		BoardPosition& bp = ECS::registry<BoardPosition>.get(e);
		m.position = boardPosToWorldPos(bp.startPos, bi, wp);
		m.destination = m.position;
		m.velocity = { 0, 0 };
		bp.bPos = bp.startPos;
		bp.futurePos = { -1, -1 };
		CombatStats& cs = ECS::registry<CombatStats>.get(e);
		cs.currentHp = cs.totalHp;
		cs.currMana = 0;
	}
}

// Reset the world state to its initial state

void WorldSystem::create_level(int mode) {
  modeState = mode;
	create_level_helper(false);
}

// Reset the world state to its initial state
void WorldSystem::create_level_helper(bool playground)
{
	pause = Pause::createPause();
	worldState = WORLD_CAMPAIGN;
	in_playground = playground;
	// manually create some space invaders - could look into doing this randomly in a loop
	auto sp = SpaceInvader::createSpaceInvader(vec2(1000, 60), vec2(100, 100), InvaderType::regular);
	auto parallax = ECS::registry<Parallax>.emplace(sp);
	parallax.distance = 1.5;
	sp = SpaceInvader::createSpaceInvader(vec2(1200, 800), vec2(120, 120), InvaderType::floating);
	parallax = ECS::registry<Parallax>.emplace(sp);
	parallax.distance = 0.8;

	board = Board::createBoard({ 100, 100 });
	for (auto e : ECS::registry<Player>.entities) {
		ECS::ContainerInterface::remove_all_components_of(e);
	}
	player = Player::createPlayer({ 100, 100 }, ally, playerCharacter);

	BoardInfo& bi = ECS::registry<BoardInfo>.get(board);
	bi.currLevel = 1;
  
  if (modeState == WORLD_TUTORIAL)
	{
		bi.currLevel = -1;
	}
  
	CombatStats cs = CombatStats();
	// Create demo units
	prevGold = bi.gold;
	LevelSystem::generate(board, unitVec);
	for (auto e : ECS::registry<Allies>.entities)
	{
		if (playerTeam.size() < (bi.boardSize.x - 1)) {
			playerTeam.push_back(ECS::registry<Identity>.get(e).ut);
		}
	}
	allyBuffs = BuffManager(ally);
	enemyBuffs = BuffManager(enemy);
	ECS::registry<PlanningPhase>.emplace(board);
}

void WorldSystem::open_level(int level) {
	shuffle_shop();
	ECS::registry<BoardInfo>.get(board).currLevel = level;
	WorldSystem::reset_board();
	CombatStats cs = CombatStats();
	prevGold = ECS::registry<BoardInfo>.get(board).gold;
	LevelSystem::generate(board, unitVec);

	BoardInfo& bi = ECS::registry<BoardInfo>.get(board);
	WorldPosition wp = ECS::registry<WorldPosition>.get(board);
	ECS::Entity unit;
	for (UnitType u : playerTeam) {
		switch (u) {
		case debugger:
			unit = UnitDebugger::createUnitDebugger(vec2(0, 0), bi, wp, ally);
			placeUnitOnBench(unit);
			break;
		case trojan:
			unit = UnitTrojan::createUnitTrojan(vec2(0, 0), bi, wp, ally);
			placeUnitOnBench(unit);
			break;
		case hdd:
			unit = UnitHDD::createUnitHDD(vec2(0, 0), bi, wp, ally);
			placeUnitOnBench(unit);
			break;
		case ddos:
			unit = UnitDDOS::createUnitDDOS(vec2(0, 0), bi, wp, ally);
			placeUnitOnBench(unit);
			break;
		case udp:
			unit = UnitUDP::createUnitUDP(vec2(0, 0), bi, wp, ally);
			placeUnitOnBench(unit);
			break;
		case dhcp:
			unit = UnitDHCP::createUnitDHCP(vec2(0, 0), bi, wp, ally);
			placeUnitOnBench(unit);
			break;
		case wifi:
			unit = UnitWifi::createUnitWifi(vec2(0, 0), bi, wp, ally);
			placeUnitOnBench(unit);
			break;
		case cpu:
			unit = UnitCPU::createUnitCPU(vec2(0, 0), bi, wp, ally);
			placeUnitOnBench(unit);
			break;
		case phishing:
			unit = UnitPhishing::createUnitPhishing(vec2(0, 0), bi, wp, ally);
			placeUnitOnBench(unit);
			break;
		case ransomware:
			unit = UnitRansomware::createUnitRansomware(vec2(0, 0), bi, wp, ally);
			placeUnitOnBench(unit);
			break;
		case ram:
			unit = UnitRAM::createUnitRAM(vec2(0, 0), bi, wp, ally);
			placeUnitOnBench(unit);
			break;
		default:
			break;
		}
	}
	
	// Create new buffsystems for these newly spawned units
	allyBuffs = BuffManager(ally);
	enemyBuffs = BuffManager(enemy);
}

void WorldSystem::load_level(std::string fileName) {
	std::string path(__FILE__);
	std::ifstream i(path.substr(0, path.size() - 9) + "../levels/" + fileName);
	json j;
	i >> j;
	WorldSystem::reset_board();
	BoardInfo bi = ECS::registry<BoardInfo>.get(board);
	WorldPosition wp = ECS::registry<WorldPosition>.get(board);

	for (auto unit : j["units"]) {
		CombatStats cs = CombatStats();
		cs.setCombatStatsJSON(unit["combatstats"]);
		int ut = unit["identity"]["unit"];
		Team ta = unit["teamaffiliation"]["team"];
		vec2 pos = vec2(unit["position"][0], unit["position"][1]);
		switch (ut) {
		case debugger:
			UnitDebugger::createUnitDebugger(pos, bi, wp, ta);
			break;
		case trojan:
			UnitTrojan::createUnitTrojan(pos, bi, wp, ta);
			break;
		case hdd:
			UnitHDD::createUnitHDD(pos, bi, wp, ta);
			break;
		case ddos:
			UnitDDOS::createUnitDDOS(pos, bi, wp, ta);
			break;
		case udp:
			UnitUDP::createUnitUDP(pos, bi, wp, ta);
			break;
		case dhcp:
			UnitDHCP::createUnitDHCP(pos, bi, wp, ta);
			break;
		case wifi:
			UnitWifi::createUnitWifi(pos, bi, wp, ta);
			break;
		case cpu:
			UnitCPU::createUnitCPU(pos, bi, wp, ta);
			break;
		case phishing:
			UnitPhishing::createUnitPhishing(pos, bi, wp, ta);
			break;
		case ransomware:
			UnitRansomware::createUnitRansomware(pos, bi, wp, ta);
			break;
		case ram:
			UnitRAM::createUnitRAM(pos, bi, wp, ta);
			break;
		default:
			break;
		}
	}
	ECS::registry<BoardInfo>.get(board).gold = j["info"]["gold"];
	ECS::registry<BoardInfo>.get(board).currLevel = j["info"]["stage"];
	ECS::registry<BoardInfo>.get(board).unitLimit = j["info"]["unitLimit"];
	ECS::registry<BoardInfo>.get(board).boardUnits = j["info"]["unitNumber"];
	std::cout << j << std::endl;
	allyBuffs = BuffManager(ally);
	enemyBuffs = BuffManager(enemy);
	i.close();
	return;
}

void WorldSystem::save_level(std::string fileName) {
	std::ofstream myfile;
	std::string path(__FILE__);
	myfile.open(path.substr(0, path.size() - 9) + "../levels/" + fileName);
	json j2;

	int a = 0;
	for (auto& u : ECS::registry<Unit>.entities) {
		ECS::registry<Unit>.list_all_components_of(u);
		j2["units"][a]["position"] = ECS::registry<BoardPosition>.get(u).getPositionJSON();
		j2["units"][a]["teamaffiliation"] = ECS::registry<TeamAffiliation>.get(u).getTeamJSON();
		j2["units"][a]["identity"] = ECS::registry<Identity>.get(u).getIdentityJSON();
		j2["units"][a]["combatstats"] = ECS::registry<CombatStats>.get(u).getCombatStatsJSON();
		a++;
	}
	j2["info"]["gold"] = ECS::registry<BoardInfo>.get(board).gold;
	j2["info"]["stage"] = ECS::registry<BoardInfo>.get(board).currLevel;
	j2["info"]["unitLimit"] = ECS::registry<BoardInfo>.get(board).unitLimit;
	j2["info"]["unitNumber"] = ECS::registry<BoardInfo>.get(board).boardUnits;
	std::cout << j2 << std::endl;
	myfile << std::setw(4) << j2 << std::endl;
	myfile.close();
	return;
}

// Compute collisions between entities
void WorldSystem::handle_collisions(float elapsed_ms)
{
	// Loop over all collisions detected by the physics system
	auto& registry = ECS::registry<PhysicsSystem::Collision>;
	for (unsigned int i = 0; i < registry.components.size(); i++)
	{
		// The entity and its collider
		auto& entity = registry.entities[i];
		auto entity_other = registry.components[i].other;

		// player - item collisions
		if (ECS::registry<Player>.has(entity))
		{
			if (ECS::registry<Item>.has(entity_other))
			{
				//TODO entity_other.onPickup();
				ECS::ContainerInterface::remove_all_components_of(entity_other);
				int& gold = ECS::registry<BoardInfo>.get(board).gold;
				std::vector<ECS::Entity> allAllies = ECS::registry<Allies>.entities;
				std::vector<ECS::Entity> allies;
				std::copy_if(allAllies.begin(), allAllies.end(), std::back_inserter(allies), [](ECS::Entity e)
					{return ECS::registry<CombatStats>.get(e).currentHp > 0; });
				if (gold > 75) {
					gold += 5;
				}
				else if (allies.size() < 3 || gold <= 10) {
					gold += 20;
				}
				else if ((allies.size() >= 3 && allies.size() < 5) || gold <= 20) {
					gold += 15;
				}
				else {
					gold += 10;
				}
			}
		}
		// projectile - unit collisions
		if (ECS::registry<Projectile>.has(entity) && ECS::registry<Active>.has(entity))
		{
			if (ECS::registry<Unit>.has(entity_other))
			{
				CombatStats& cs = ECS::registry<CombatStats>.get(entity_other);
				Team pTeam = ECS::registry<TeamAffiliation>.get(entity).t;
				if (ECS::registry<TeamAffiliation>.get(entity_other).t == pTeam || cs.currentHp <= 0) {
					continue;
				}
				else {
					std::vector<vec2> oldHull = *ECS::registry<HullMeshRef>.get(entity_other).reference_to_cache;
					Motion motion = ECS::registry<Motion>.get(entity_other);
					Motion& pmotion = ECS::registry<Motion>.get(entity);
					std::vector<vec2> hull;
					hull = util::transformHull(oldHull, motion);
					int n = hull.size();
					for (int i = 0; i < n; i++) {
						vec2 p1 = hull[i];
						vec2 p2 = hull[(i + 1) % n];
						if (util::minimum_distance(p1, p2, pmotion.position) <= (pmotion.scale.x / 2)) {
							CombatStats& cs_projectile = ECS::registry<CombatStats>.get(entity);
							if (ECS::registry<Explode>.has(entity)) {
								Explode explode = ECS::registry<Explode>.get(entity);
								Motion m = ECS::registry<Motion>.get(entity);
								Particle::createParticles(30, m.position, vec2(20., 20.), "green_bits", 600., 600., 0, -150., 150., -150, 150);
								for (auto& e : ECS::registry<Unit>.entities) {
									if ((pTeam != ECS::registry<TeamAffiliation>.get(e).t) && glm::length(ECS::registry<Motion>.get(e).position - motion.position) < explode.radius)
										Unit::damageUnit(e, cs_projectile.attack);
								}
							}
							else {
								Unit::damageUnit(entity_other, cs_projectile.attack);
							}
							vec2 normal = normalize(vec2(1.f * (p1.y - p2.y), -1.f * (p1.x - p2.x)));
							vec2 newVelocity = pmotion.velocity - 2.f * (dot(pmotion.velocity, normal)) * normal;
							double low = 0;
							double high = elapsed_ms;
							double epsilon = 0.001;
							//mid represents how much extra time the projectile moved since collision
							double mid = (high - low) / 2 + low;
							vec2 mid_pos = pmotion.position - vec2(pmotion.velocity.x * mid / 1000.f, pmotion.velocity.y * mid / 1000.f);
							while ((high - low) > epsilon) {
								mid = (high - low) / 2 + low;
								mid_pos = pmotion.position - vec2(pmotion.velocity.x / 1000 * mid, pmotion.velocity.y / 1000 * mid);
								if (util::minimum_distance(p1, p2, mid_pos) <= (pmotion.scale.x / 2)) {
									low = mid;
								}
								else {
									high = mid;
								};
							}

							vec2 newPos = mid_pos + vec2(newVelocity.x / 1000 * mid, newVelocity.y / 1000 * mid);

							if (!ECS::registry<Missile>.has(entity)) {
								auto new_entity = Projectile::createProjectile(newPos, pmotion.scale.x / 2, newVelocity, pTeam, 0.f, vec3(0.0, 0.0, 0.0));

								ECS::registry<Active>.remove(new_entity);
							}

							ECS::ContainerInterface::remove_all_components_of(entity);
							i = n;
						}
					}
				}

			}
		}
	}

	// Remove all collisions from this simulation step
	ECS::registry<PhysicsSystem::Collision>.clear();
}

// Should the game be over ?
bool WorldSystem::is_over() const
{
	return glfwWindowShouldClose(window) > 0;
}

// On key callback
void WorldSystem::on_key(int key, int, int action, int mod)
{
	// Pause
	if (worldState == WORLD_CAMPAIGN || worldState == WORLD_TUTORIAL|| worldState == WORLD_PAUSE) {
		if (key == GLFW_KEY_P && action == GLFW_RELEASE) {
			if (worldState == WORLD_CAMPAIGN)
			{
				std::cout << "Pausing\n";
				worldState = WORLD_PAUSE;
				ECS::registry<Motion>.get(pause).is_in_foreground = true;
			}
			else if (worldState == WORLD_PAUSE)
			{
				std::cout << "Unpausing\n";
				ECS::registry<Motion>.get(pause).is_in_foreground = false;
				worldState = WORLD_CAMPAIGN;
			}
		}
		if (key == GLFW_KEY_R && action == GLFW_RELEASE) {
			/*int w, h;
				glfwGetWindowSize(window, &w, &h);*/
				//std::cout << worldState;
			restart();
		}
	}

	// Move player if alive
	if (worldState == WORLD_CAMPAIGN)
	{
		// Changing phases
		if (action == GLFW_PRESS && key == GLFW_KEY_SPACE)
		{
			if (ECS::registry<PlanningPhase>.has(board))
			{
				if (in_playground) {
					save_level("last.json");
				}
				ECS::registry<PlanningPhase>.remove(board);
				ECS::registry<BattlePhase>.emplace(board);
			}
		}

		//Handle Player Movement
		auto& player_motion = ECS::registry<Motion>.get(player);

		auto& player_velocity = player_motion.velocity;

		float player_speed = player_motion.speed;

		if (key == GLFW_KEY_UP && action == GLFW_PRESS)
		{
			player_velocity = { player_velocity.x, player_velocity.y - player_speed };
		}
		if (key == GLFW_KEY_DOWN && action == GLFW_PRESS)
		{
			player_velocity = { player_velocity.x, player_velocity.y + player_speed };
		}
		if (key == GLFW_KEY_LEFT && action == GLFW_PRESS)
		{
			player_velocity = { player_velocity.x - player_speed, player_velocity.y };
		}
		if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS)
		{
			player_velocity = { player_velocity.x + player_speed, player_velocity.y };
		}

		if (key == GLFW_KEY_UP && action == GLFW_RELEASE)
		{
			player_motion.velocity = { player_motion.velocity.x, player_motion.velocity.y == 0 ? 0 : player_motion.velocity.y + player_speed };
		}
		if (key == GLFW_KEY_DOWN && action == GLFW_RELEASE)
		{
			player_motion.velocity = { player_motion.velocity.x, player_motion.velocity.y == 0 ? 0 : player_motion.velocity.y - player_speed };
		}
		if (key == GLFW_KEY_LEFT && action == GLFW_RELEASE)
		{
			player_motion.velocity = { player_motion.velocity.x == 0 ? 0 : player_motion.velocity.x + player_speed, player_motion.velocity.y };
		}
		if (key == GLFW_KEY_RIGHT && action == GLFW_RELEASE)
		{
			player_motion.velocity = { player_motion.velocity.x == 0 ? 0 : player_motion.velocity.x - player_speed, player_motion.velocity.y };
		}
	}

	// Close game
	if (action == GLFW_RELEASE && key == GLFW_KEY_ESCAPE)
	{
		glfwSetWindowShouldClose(window, GL_TRUE);
	}

	// Debugging
	if (key == GLFW_KEY_D)
		DebugSystem::in_debug_mode = (action != GLFW_RELEASE);

	// Test Load Level
	if (key == GLFW_KEY_N && action == GLFW_RELEASE)
		WorldSystem::load_level("test.json");

	// Test Save Level
	if (key == GLFW_KEY_M && action == GLFW_RELEASE)
		WorldSystem::save_level("test.json");

	// Test Item
	if (key == GLFW_KEY_I && action == GLFW_RELEASE) {
		Item::createItem(vec2(100, 200));
	}

	// Test Item
	if (key == GLFW_KEY_V && action == GLFW_RELEASE) {
		std::cout << "projectile" << std::endl;
		//currently does 30 damage to non ally units
		Projectile::createProjectile(ECS::registry<Motion>.get(player).position, 10.f, vec2(0, -50.f), ally, 30.f, vec3(1.f, 1.f, 1.f));
	}


	/*if (worldState != WORLD_VICTORY &&
		worldState != WORLD_DEFEAT &&
		key >= GLFW_KEY_0 && key <= GLFW_KEY_9 && action == GLFW_RELEASE)
		open_level(key - GLFW_KEY_1 + 1);*/
}

bool isHoveringOn(vec2 mouse_pos, vec2 position, vec2 scale)
{
	if (mouse_pos.x <= position.x + scale.x / 2 && mouse_pos.x >= position.x - scale.x / 2
		&&
		mouse_pos.y <= position.y + scale.y / 2 && mouse_pos.y >= position.y - scale.y / 2)
	{
		return true;
	}

	return false;
}

void WorldSystem::on_mouse_move(vec2 mouse_pos)
{
	vec2 cam_transform = get_cam_transform();
	mouse_pos -= cam_transform;

	//hover detection
	auto& IG_container = ECS::registry<ImGuiDesc>;
	// for (auto [i, motion_i] : enumerate(motion_container.components)) // in c++ 17 we will be able to do this instead of the next three lines
	for (unsigned int i = 0; i < IG_container.components.size(); i++)
	{
		ImGuiDesc& ig_desc = IG_container.components[i];
		ECS::Entity entity = IG_container.entities[i];
		auto& motion = ECS::registry<Motion>.get(entity);

		if (isHoveringOn(mouse_pos, motion.position, motion.scale))
		{
			ig_desc.activated = true;
		}
		else
		{
			ig_desc.activated = false;
		}
	}

	if (ECS::registry<PlanningPhase>.has(board))
	{
		if (selected != NULL)
		{
			Motion& m = ECS::registry<Motion>.get(*selected);
			m.position = mouse_pos;
		}
	}
}

void WorldSystem::on_mouse_click(int button, int action, int mods)
{
	if (ECS::registry<PlanningPhase>.has(board) || ECS::registry<BattlePhase>.has(board))
	{
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		// Compensate for camera transformation
		vec2 cam_transform = get_cam_transform();
		vec2 fixed_mousePos = { xpos, ypos };
		fixed_mousePos = fixed_mousePos - cam_transform;

		if (worldState == WORLD_CAMPAIGN && button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
		{
			// Compensate for camera transformation
			vec2 cam_transform = get_cam_transform();
			xpos -= cam_transform.x;
			ypos -= cam_transform.y;

			auto& player_motion = ECS::registry<Motion>.get(player);
			vec2 source = player_motion.position;
			vec2 destination = vec2(xpos, ypos);
			if (source != destination)
			{
				player_motion.destination = destination;
				player_motion.is_moving = true;
			}

		} 
		if (ECS::registry<PlanningPhase>.has(board)) {
			if (selected == NULL)
			{
				if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
				{
					for (auto& e : ECS::registry<Unit>.entities)
					{
						Motion& m = ECS::registry<Motion>.get(e);
						if (intersectsCursor(m, fixed_mousePos))
						{
							selected = &e;
							break;
						}
					}
				}
			}
			else if (selected != NULL)
			{
				if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
				{
					snapToClosestCell(*selected, fixed_mousePos);
					selected = NULL;
				}
			}
		}
		return;
	}
}

void WorldSystem::snapToClosestCell(ECS::Entity u, vec2 cursorPos)
{
	BoardInfo& bi = ECS::registry<BoardInfo>.get(board);
	BoardPosition& bp = ECS::registry<BoardPosition>.get(u);
	TeamAffiliation& ta = ECS::registry<TeamAffiliation>.get(u);
	// Can not move enemies
	if (ta.t == enemy && !in_playground) {
		placeUnitAtCell(u, bp.bPos, bp.bPos);
		return;
	}
	// check unit limit, cannot move benched units if full
	int extra_unit = 0;
	if (ECS::registry<TeamAffiliation>.get(u).t == ally) {
		for (auto& e : ECS::registry<ExtraUnit>.entities) {
			ExtraUnit eu = ECS::registry<ExtraUnit>.get(e);
			if (eu.t == ally)
				extra_unit = 1;
		}
	}
	if (bi.unitLimit + extra_unit == bi.boardUnits && bp.bPos.y == bi.boardSize.y && !in_playground) {
		placeUnitAtCell(u, bp.bPos, bp.bPos);
		return;
	}
	vec2 startPos = bp.bPos;
	vec2 newBoardPos = bp.bPos;
	WorldPosition wp = ECS::registry<WorldPosition>.get(board);
	float minDist = length(boardPosToWorldPos(bp.bPos, bi, wp) - cursorPos);
	Team newTeam = ta.t;
	for (auto e : ECS::registry<CellStruct>.entities)
	{
		CellStruct& cs = ECS::registry<CellStruct>.get(e);
		if (((cs.ct == allyCell || in_playground) || cs.ct == benchCell || cs.ct == shopCell) && cs.isOccupied == false)
		{
			Motion cellMotion = ECS::registry<Motion>.get(e);
			float dist = length(cellMotion.position - cursorPos);
			if (dist < minDist)
			{
				BoardPosition& cellbp = ECS::registry<BoardPosition>.get(e);
				minDist = dist;
				newBoardPos = cellbp.startPos;
				newTeam = (cs.ct == enemyCell ? enemy : ally);
			}
		}
	}
	if (in_playground && ta.t != newTeam) {
		Team newT = newTeam;
		ECS::Entity newU;
		vec2 pos = vec2(0, 0);
		switch (ECS::registry<Identity>.get(u).ut) {
		case debugger:
			newU = UnitDebugger::createUnitDebugger(pos, bi, wp, newT);
			break;
		case trojan:
			newU = UnitTrojan::createUnitTrojan(pos, bi, wp, newT);
			break;
		case hdd:
			newU = UnitHDD::createUnitHDD(pos, bi, wp, newT);
			break;
		case ddos:
			newU = UnitDDOS::createUnitDDOS(pos, bi, wp, newT);
			break;
		case udp:
			newU = UnitUDP::createUnitUDP(pos, bi, wp, newT);
			break;
		case dhcp:
			newU = UnitDHCP::createUnitDHCP(pos, bi, wp, newT);
			break;
		case wifi:
			newU = UnitWifi::createUnitWifi(pos, bi, wp, newT);
			break;
		case cpu:
			newU = UnitCPU::createUnitCPU(pos, bi, wp, newT);
			break;
		case phishing:
			newU = UnitPhishing::createUnitPhishing(pos, bi, wp, newT);
			break;
		case ransomware:
			newU = UnitRansomware::createUnitRansomware(pos, bi, wp, newT);
			break;
		case ram:
			newU = UnitRAM::createUnitRAM(pos, bi, wp, newT);
			break;
		default:
			break;
		}
		ECS::ContainerInterface::remove_all_components_of(u);
		placeUnitAtCell(newU, startPos, newBoardPos);
	}
	else {
		placeUnitAtCell(u, startPos, newBoardPos);
	}
	
}

int WorldSystem::placeUnitAtCell(ECS::Entity u, vec2 startPos, vec2 boardPos)
{
	ECS::Entity** grid = Board::getCells(board);
	int x = boardPos.x, y = boardPos.y;
	CellStruct& new_cs = ECS::registry<CellStruct>.get(grid[y][x]);
	BoardInfo& bi = ECS::registry<BoardInfo>.get(board);
	if (!new_cs.isOccupied || startPos == boardPos) {
		int old_x = startPos.x, old_y = startPos.y;
		CellStruct& old_cs = ECS::registry<CellStruct>.get(grid[old_y][old_x]);
		old_cs.isOccupied = false;

		Motion& unitMotion = ECS::registry<Motion>.get(u);
		BoardPosition& bp = ECS::registry<BoardPosition>.get(u);
		WorldPosition wp = ECS::registry<WorldPosition>.get(board);

		bool wasBench = bp.isBench;
		unitMotion.position = boardPosToWorldPos(boardPos, bi, wp);
		unitMotion.initialPosition = unitMotion.position;
		bp.startPos = boardPos;
		bp.bPos = boardPos;

		if (new_cs.ct == shopCell) {
			ECS::registry<BoardInfo>.get(board).gold += 5;
			if (!wasBench) {
				bi.boardUnits--;
			}
			ECS::ContainerInterface::remove_all_components_of(u);
			new_cs.isOccupied = false;
		} else if (bp.bPos.y == bi.boardSize.y) {
			bp.isBench = true;
			// Newly benched unit, remove the unit from active buff system
			if (!wasBench) {
				bi.boardUnits--;
				allyBuffs.removeActiveUnit(u);
			}
			new_cs.isOccupied = true;
		} else {
			bp.isBench = false;
			if (wasBench) {
				bi.boardUnits++;
				allyBuffs.addActiveUnit(u);

				//Tutorial
				if (tutorialStep == 2)
				{
					Identity& unitI = ECS::registry<Identity>.get(u);
					if (unitI.ut == trojan)
					{
						tutorialStepDone = true;
					}
				}
			}
			new_cs.isOccupied = true;
		}

		return 1;
	}
	return -1;
}

bool WorldSystem::placeUnitOnBench(ECS::Entity u)
{
	BoardPosition& bp = ECS::registry<BoardPosition>.get(u);
	bp.isBench = true;
	vec2 startPos = bp.bPos;
	vec2 newBoardPos = bp.bPos;

	for (auto e : ECS::registry<CellStruct>.entities)
	{
		CellStruct& cs = ECS::registry<CellStruct>.get(e);
		if (cs.ct == benchCell && cs.isOccupied == false)
		{
			BoardPosition& cellbp = ECS::registry<BoardPosition>.get(e);
			newBoardPos = cellbp.startPos;
			if (placeUnitAtCell(u, startPos, newBoardPos) == 1)
			{
				return true;
			}
		}
	}

	return false;
}

float WorldSystem::get_dir_angle(vec2 source, vec2 destination)
{
	vec2 relative_pos = destination - source;
	float rel_magnitude = sqrt(pow(relative_pos.x, 2) + pow(relative_pos.y, 2));
	vec2 relative_dir = { relative_pos.x / rel_magnitude, relative_pos.y / rel_magnitude };

	vec2 default_dir = { 1, 0 };

	float theta = acos(dot(relative_dir, default_dir));

	if (destination.y < source.y)
		return -theta;
	else
		return theta;
}

bool WorldSystem::intersectsCursor(const Motion& motion, const vec2 cursorPos)
{
	auto dp = motion.position - cursorPos;
	float dist_squared = length(dp);
	float other_r = std::sqrt(std::pow(abs(motion.scale.x) / 2.f, 2) + std::pow(abs(motion.scale.y) / 2.f, 2.f));
	if (dist_squared < other_r)
		return true;
	return false;
}
