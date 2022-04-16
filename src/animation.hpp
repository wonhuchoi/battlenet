#pragma once

#include "common.hpp"
#include "game_components.hpp"
#include "tiny_ecs.hpp"
#include "util.hpp"
#include "render_components.hpp"
#include "../ext/observable/subject.hpp"

// A simple Animation system that renders (sprite-sheet) animations
class AnimationSystem : public Subject<AnimationSystem>
{
public:
	void step(float elapsed_ms, vec2 window_size_in_game_units);
	static void load(ShadedMesh& mesh_container, std::string texture_path, std::string shader_name, std::string key);
	// 0-indexed, texcoord offset from base texcoord
	float player_anim_x_offset;
	float player_anim_y_offset;
	float coin_anim_x_offset;
	float coin_anim_y_offset;
	float space_invader_anim_x_offset;
	float space_invader_anim_y_offset;
private:
	static void load(ShadedMesh& sprite, std::string texture_path, std::string shader_name, float offsetX, float offsetY, std::string key);
	float elapsed_ms = 0;
	int player_curr_animation = 0;
	int player_curr_frame = 1;
	int coin_curr_animation = 0;
	int coin_curr_frame = 1;
	int space_invader_curr_animation = 0;
	int space_invader_curr_frame = 1;
};
