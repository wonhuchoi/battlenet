#pragma once

#include "common.hpp"

// Data structure for pebble-specific information
namespace DebugSystem {
	extern bool in_debug_mode;

	// Removes all debugging graphics in ECS, called at every iteration of the game loop
	void clearDebugComponents();
	void createLineFromPoints(vec2 p1, vec2 p2, vec3 color);
};
