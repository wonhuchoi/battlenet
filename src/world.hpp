#pragma once

// internal
#include "common.hpp"
#include "game_components.hpp"
#include "render_components.hpp"
#include "player.hpp"
#include "unit.hpp"
#include "item.hpp"
#include "buff.hpp"

// stlib
#include <vector>
#include <random>
#include <subject.hpp>


#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>

// Container for all our entities and game logic. Individual rendering / update is 
// deferred to the relative update() methods
// WorldSystem is a Subject
// to subscribe to notifications, extend Observer<WorldSystem> in listener class and implement the `update` fcn
// to notify, call `notifyObservers` in this class
class WorldSystem : public Subject<WorldSystem>
{
public:
	// Creates a window
	WorldSystem(ivec2 window_size_px);

	// Releases all associated resources
	~WorldSystem();

	// restart game
	void restart();

	// create level
	//mode: Campaign, Tutorial, Playground
	void create_level(int mode);

	// create level with option
	void create_level_helper(bool playground);

	void next_level();

	// create level
	void open_level(int level);

	// shop functions
	void shuffle_shop();
	bool shopIndexDisabled(int i);
	void disableShopIndex(int i);

	// Steps the game ahead by ms milliseconds
	void step(float elapsed_ms, vec2 window_size_in_game_units);

	// Load level from file
	void load_level(std::string fileName);

	// Save level to file
	void save_level(std::string fileName);

	// Check for collisions
	void handle_collisions(float elapsed_ms);

	// Renders our scene
	void draw();

	// Should the game be over ?
	bool is_over() const;


	// Bound for from what point the camera follows the player character
	float screen_boundary = 200.f;
	vec4 camera_bound = { screen_boundary, world_size[0] - screen_boundary, screen_boundary, world_size[1] - screen_boundary }; // left, right, top, bottom
	//get camera Transform
	vec2 get_cam_transform();

	bool intersectsCursor(const Motion& motion1, const vec2 cursorPos);

	// OpenGL window handle
	GLFWwindow* window;
	bool in_playground = false;
	int worldState;
	int modeState;
	enum {
		WORLD_START = 1,
		WORLD_PAUSE = 2,
		WORLD_CAMPAIGN = 3,
		WORLD_VICTORY = 4,
		WORLD_DEFEAT = 5,
		WORLD_INTRO = 6,
		WORLD_SELECT = 7,
		WORLD_TUTORIAL = 8
	};
	ECS::Entity board;
	BuffManager allyBuffs;
	BuffManager enemyBuffs;
	ECS::Entity player;
	int prevGold = 0;

	//Each step of tutorial
	int tutorialStep;
	//Whether the player has finished the task for each tutorial step
	bool tutorialStepDone = false;

	//places the unit on the next available bench. Public because I need to call this in ImGuiSystem.cpp
	bool placeUnitOnBench(ECS::Entity u);
	std::vector<int> unitVec;
	std::vector<UnitType> playerTeam;

private:
	// Input callback functions
	void on_key(int key, int, int action, int mod);
	void on_mouse_move(vec2 mouse_pos);
	void on_mouse_click(int button, int action, int mods);
	float get_dir_angle(vec2 source, vec2 destination);
	void snapToClosestCell(ECS::Entity u, vec2 pos);
	int placeUnitAtCell(ECS::Entity u, vec2 startPos, vec2 boardPos);
	void revertBoard();
	// Loads the audio
	void init_audio();

	ECS::Entity* selected;
	ECS::Entity pause;

	float victory_counter_ms = 15000;
	float defeat_counter_ms = 30000;
	
	// music references
	Mix_Music* background_music;

	// C++ random number generator
	std::default_random_engine rng;
	std::uniform_real_distribution<float> uniform_dist;
	// number between 0..1

	// Shop indexes
	bool disabledShopIndexes[4];
	
	void reset_board();

};
