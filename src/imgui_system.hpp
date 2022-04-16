#pragma once

#include "common.hpp"
#include "game_components.hpp"
#include "tiny_ecs.hpp"
#include "render_components.hpp"
#include "render.hpp"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "world.hpp"
#include "score.hpp"
#include "phase.hpp"

// System responsible for setting up OpenGL and for rendering all the 
// visual entities in the game
class ImguiSystem
{
public:
	// Initialize the window
	ImguiSystem(GLFWwindow* window);

	// Destroy resources associated to one or all entities created by the system
	~ImguiSystem();

	void step(WorldSystem& world, PhaseSystem& phase, ScoreSystem& score, float elapsed_ms);
	void start(WorldSystem& world, ScoreSystem& score,
		ImGuiWindowFlags window_flags, vec2 cam_transform,
		int w, int h);
	void intro(WorldSystem& world, ScoreSystem& score,
		ImGuiWindowFlags window_flags, vec2 cam_transform,
		int w, int h);
	void levelSelect(WorldSystem& world, ScoreSystem& score,
		ImGuiWindowFlags window_flags, vec2 cam_transform,
		int w, int h);
	void campaign(WorldSystem& world, ScoreSystem& score,
		ImGuiWindowFlags window_flags, vec2 cam_transform,
		int w, int h, float elapsed_ms);
	void tutorial(WorldSystem& world, PhaseSystem& phase, ScoreSystem& score,
		ImGuiWindowFlags window_flags, vec2 cam_transform,
		int w, int h, float elapsed_ms);
	void victory(WorldSystem& world, ScoreSystem& score,
		ImGuiWindowFlags window_flags, vec2 cam_transform,
		int w, int h);
	void defeat(WorldSystem& world, ScoreSystem& score,
		ImGuiWindowFlags window_flags, vec2 cam_transform,
		int w, int h);

	ShadedMesh& createButton(vec2 imgui_pos, vec2 cam_transform,
		vec2 window_size, std::string key, ImGuiWindowFlags window_flags);

private:
	// Window handle
	GLFWwindow* window;
	
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	
	float time_taken = 0;
	float total_time = 0;
};
