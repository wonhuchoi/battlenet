#define GL3W_IMPLEMENTATION
#include <gl3w.h>

// stlib
#include <chrono>
#include <iostream>
#include <fstream>

// internal
#include "common.hpp"
#include "world.hpp"
#include "tiny_ecs.hpp"
#include "attack.hpp"
#include "render.hpp"
#include "physics.hpp"
#include "score.hpp"
#include "debug.hpp"
#include "ai.hpp"
#include "battle.hpp"
#include "imgui_system.hpp"
#include "animation.hpp"
#include "particle.hpp"

#ifdef IMAGEMAGICK_AVAILABLE
	#include "img_gen.hpp"
#endif
#include <time.h>
#include "phase.hpp"

using Clock = std::chrono::high_resolution_clock;

const ivec2 window_size_in_px = {1200, 800};
const vec2 window_size_in_game_units = { 1200, 800 };
// Note, here the window will show a width x height part of the game world, measured in px. 
// You could also define a window to show 1.5 x 1 part of your game world, where the aspect ratio depends on your window size.

struct Description {
	std::string name;
	Description(const char* str) : name(str) {};
};

// Entry point
int main()
{
	#ifdef IMAGEMAGICK_AVAILABLE
		float rangeConst = QuantumRange/256;
		// Generate unit art of varying colors
		std::vector<std::vector<float>> color;
		color.push_back({QuantumRange, QuantumRange, 0});
		color.push_back({0, QuantumRange, 0});
		color.push_back({0, 0, QuantumRange});
		color.push_back({0, QuantumRange, QuantumRange});
		color.push_back({230*rangeConst, 0, QuantumRange});
		color.push_back({QuantumRange, 94*rangeConst, 0});
		color.push_back({4*rangeConst, 107*rangeConst, 21*rangeConst});
		color.push_back({170*rangeConst, 139*rangeConst, 224*rangeConst});
		color.push_back({QuantumRange, QuantumRange, QuantumRange});
		color.push_back({242*rangeConst, 119*rangeConst, 213*rangeConst});
		color.push_back({52*rangeConst, 48*rangeConst, 110*rangeConst});
		generateUnitImages(11, color, time(NULL));
	#endif
	// Initialize the main systems
	WorldSystem world(window_size_in_px);
	RenderSystem renderer(*world.window);
	PhysicsSystem physics;
	AISystem ai;
	
	ScoreSystem score;
	BattleSystem battle;
	ImguiSystem imgui_system(world.window);
	PhaseSystem phase;
	AnimationSystem anim;
	AttackSystem as;

    // Set up subscriptions
	// battle.subscribe(ai);
	// Set up subscriptions
	anim.subscribe(renderer);
	world.subscribe(phase);
	as.subscribe(score);

	// Set all states to default
	world.restart();
	phase.begin();
	auto t = Clock::now();
	// Variable timestep loop
	while (!world.is_over())
	{
		// Processes system messages, if this wasn't present the window would become unresponsive
		glfwPollEvents();

		// Calculating elapsed times in milliseconds from the previous iteration
		auto now = Clock::now();
		float elapsed_ms = static_cast<float>((std::chrono::duration_cast<std::chrono::microseconds>(now - t)).count()) / 1000.f;
		t = now;

		DebugSystem::clearDebugComponents();
		if (world.worldState != world.WORLD_PAUSE) {
			world.step(elapsed_ms, window_size_in_game_units);
			score.step(elapsed_ms, window_size_in_game_units);
			physics.step(elapsed_ms, window_size_in_game_units, world.worldState);
			phase.step(elapsed_ms);
			world.step(elapsed_ms, window_size_in_game_units);
			world.handle_collisions(elapsed_ms);
			ai.step(elapsed_ms, window_size_in_game_units);
			battle.step(elapsed_ms);
			as.step(elapsed_ms);
		}
		imgui_system.step(world, phase, score, elapsed_ms);
		anim.step(elapsed_ms, window_size_in_game_units);
		renderer.draw(world.get_cam_transform(), window_size_in_game_units);
	}

	//glfwDestroyWindow(window);
	glfwTerminate();
	return EXIT_SUCCESS;
}


