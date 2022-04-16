// internal
#include "animation.hpp"
#include "render.hpp"
#include "tiny_ecs.hpp"
#include "common.hpp"
#include "player.hpp"
#include "util.hpp"
#include "item.hpp"

/**
 * Sprite sheet format example
 * Animation 1 frame 1 | animation 1 frame 2 ...
 * --------------
 * Animation 2 frame 1 | animation 2 frame 2 ...
 * ...
 **/

float ANIMATION_SPEED = 320.0f;      // in ms
std::string PLAYER_KEY = "player";
int PLAYER_NUM_ANIMATION_FRAMES = 2; // player default is 2 frames
int PLAYER_NUM_ANIMATIONS = 5;       // player has 5 animations - see sprite sheet
float PLAYER_SPRITE_WIDTH = 32.0f;   // width in px
float PLAYER_SPRITE_HEIGHT = 32.0f;  // height in px

std::string COIN_KEY = "item";
int COIN_NUM_ANIMATION_FRAMES = 6; // coin default is 6 frames
int COIN_NUM_ANIMATIONS = 1;       // coin has 1 animation - see sprite sheet
float COIN_SPRITE_WIDTH = 16.0f;   // width in px
float COIN_SPRITE_HEIGHT = 18.0f;  // height in px

std::string SPACE_INVADER_KEY = "space_invader";
std::string SPACE_INVADER_KEY_2 = "space_invader_2";
int SPACE_INVADER_NUM_ANIMATION_FRAMES = 7; // default is 7 frames
int SPACE_INVADER_NUM_ANIMATIONS = 1;       // 1 animation - see sprite sheet
float SPACE_INVADER_SPRITE_WIDTH = 32.0f;   // width in px
float SPACE_INVADER_SPRITE_HEIGHT = 32.0f;  // height in px

float player_texture_x_offset(float animation_frame_offset)
{
	return (animation_frame_offset * PLAYER_SPRITE_WIDTH) / (PLAYER_NUM_ANIMATION_FRAMES * PLAYER_SPRITE_WIDTH);
}

float player_texture_y_offset(float animation_offset)
{
	return (animation_offset * PLAYER_SPRITE_HEIGHT) / (PLAYER_NUM_ANIMATIONS * PLAYER_SPRITE_HEIGHT);
}

float coin_texture_x_offset(float animation_frame_offset)
{
	return (animation_frame_offset * COIN_SPRITE_WIDTH) / (COIN_NUM_ANIMATION_FRAMES * COIN_SPRITE_WIDTH);
}

float coin_texture_y_offset(float animation_offset)
{
	return (animation_offset * COIN_SPRITE_HEIGHT) / (COIN_NUM_ANIMATIONS * COIN_SPRITE_HEIGHT);
}

float space_invader_texture_x_offset(float animation_frame_offset)
{
	return (animation_frame_offset * SPACE_INVADER_SPRITE_WIDTH) / (SPACE_INVADER_NUM_ANIMATION_FRAMES * SPACE_INVADER_SPRITE_WIDTH);
}

float space_invader_texture_y_offset(float animation_offset)
{
	return (animation_offset * SPACE_INVADER_SPRITE_HEIGHT) / (SPACE_INVADER_NUM_ANIMATIONS * SPACE_INVADER_SPRITE_HEIGHT);
}

// TODO - handle future animations with ECS::registry<Animated>
void AnimationSystem::step(float ms, vec2 window_size_in_game_units)
{
	elapsed_ms += ms;
	if (elapsed_ms > ANIMATION_SPEED)
	{
		elapsed_ms = 0;
		// continuously cycle through animation frames
		player_curr_frame = (player_curr_frame + 1) % PLAYER_NUM_ANIMATION_FRAMES;
		
		// play current player animation based on player velocity
		auto& entities = ECS::registry<Player>.entities;
		if (entities.size() > 0) {
			// TODO - handle more players if needed
			auto& player_entity = entities[0];
			auto& player_motion = ECS::registry<Motion>.get(player_entity);
			if (player_motion.velocity.x == 0 && player_motion.velocity.y == 0) {
				player_curr_animation = 0;
			} else {
				float angle = util::get_dir_angle(player_motion.position, player_motion.position + player_motion.velocity);
				float offset = 0.6;
        // up
        if (angle < 0) {
					player_curr_animation = 2;
        }
        // right
				else if (angle < PI / 2 - offset)
				{
					player_curr_animation = 4;
				}
        // down
				else if (angle < PI - offset)
				{
					player_curr_animation = 1;
				}
        // left
				else if (angle < PI + PI / 2 + offset)
				{
					player_curr_animation = 3;
				}
				else
				{
					player_curr_animation = 0;
				}
			}
		}

		player_anim_x_offset = player_texture_x_offset(player_curr_frame);
		player_anim_y_offset = player_texture_y_offset(player_curr_animation);

		// continuously cycle through animation frames
		coin_curr_frame = (coin_curr_frame + 1) % COIN_NUM_ANIMATION_FRAMES;
		coin_anim_x_offset = coin_texture_x_offset(coin_curr_frame);
		coin_anim_y_offset = coin_texture_y_offset(coin_curr_animation);

		space_invader_curr_frame = (space_invader_curr_frame + 1) % SPACE_INVADER_NUM_ANIMATION_FRAMES;
		space_invader_anim_x_offset = space_invader_texture_x_offset(space_invader_curr_frame);
		space_invader_anim_y_offset = space_invader_texture_y_offset(space_invader_curr_animation);

		notifyObservers();
	}

	// translate parallax entities in the x-direction
	for (auto& entity : ECS::registry<Parallax>.entities) {
		// TODO - handle more players if needed
		auto& playerMotion = ECS::registry<Motion>.get(ECS::registry<Player>.entities[0]);
		auto& parallaxMotion = ECS::registry<Motion>.get(entity);
		auto& parallax = ECS::registry<Parallax>.get(entity);
		if (playerMotion.velocity.x != 0) {
			float distanceDiff = parallax.MAX_DISTANCE - parallax.distance;
			float offset = (std::pow(distanceDiff, 3) + distanceDiff - 12) * std::abs(window_size_in_game_units.x / 2 - playerMotion.position.x) * 0.000006;
			parallaxMotion.position.x = playerMotion.velocity.x > 0 ? parallaxMotion.position.x - offset : parallaxMotion.position.x + offset;
		}
	}
}

// Load animation 1 frame 1
void AnimationSystem::load(ShadedMesh &sprite, std::string texture_path, std::string shader_name, std::string key)
{
	load(sprite, texture_path, shader_name, 1, 1, key);
}

// overload with any animation any frame, 1-indexed
void AnimationSystem::load(ShadedMesh &sprite, std::string texture_path, std::string shader_name, float offsetX, float offsetY, std::string key)
{
	if (texture_path.length() > 0)
		sprite.texture.load_from_file(texture_path.c_str());

	float y_offset = 0;
	if (key == PLAYER_KEY)  {
		y_offset = player_texture_y_offset(offsetY);
	}
	else if (key == COIN_KEY) {
		y_offset = coin_texture_y_offset(offsetY);
	}
	else if (key == SPACE_INVADER_KEY || key == SPACE_INVADER_KEY_2) {
		y_offset = space_invader_texture_y_offset(offsetY);
	}
	float y_prev = y_offset - 1 > 0 ? y_offset - 1 : 0.f;
	float x_offset = 0;
	if (key == PLAYER_KEY)  {
		x_offset = player_texture_x_offset(offsetX);
	}
	else if (key == COIN_KEY) {
		x_offset = coin_texture_x_offset(offsetX);
	}
	else if (key == SPACE_INVADER_KEY || key == SPACE_INVADER_KEY_2) {
		x_offset = space_invader_texture_x_offset(offsetX);
	}
	float x_prev = x_offset - 1 > 0 ? x_offset - 1 : 0.f;
	// The position corresponds to the center of the texture.
	TexturedVertex vertices[4];
	vertices[0].position = {-1.f / 2, +1.f / 2, 0.f};
	vertices[1].position = {+1.f / 2, +1.f / 2, 0.f};
	vertices[2].position = {+1.f / 2, -1.f / 2, 0.f};
	vertices[3].position = {-1.f / 2, -1.f / 2, 0.f};
	vertices[0].texcoord = {y_prev, y_offset};
	vertices[1].texcoord = {x_offset, y_offset};
	vertices[2].texcoord = {x_offset, x_prev};
	vertices[3].texcoord = {x_prev, y_prev};

	// Counterclockwise as it's the default opengl front winding direction.
	uint16_t indices[] = {0, 3, 1, 1, 3, 2};

	glGenVertexArrays(1, sprite.mesh.vao.data());
	glGenBuffers(1, sprite.mesh.vbo.data());
	glGenBuffers(1, sprite.mesh.ibo.data());
	gl_has_errors();

	// Vertex Buffer creation
	glBindBuffer(GL_ARRAY_BUFFER, sprite.mesh.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); // sizeof(TexturedVertex) * 4
	gl_has_errors();

	// Index Buffer creation
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sprite.mesh.ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW); // sizeof(uint16_t) * 6
	gl_has_errors();

	glBindVertexArray(0); // Unbind VAO (it's always a good thing to unbind any buffer/array to prevent strange bugs), remember: do NOT unbind the EBO, keep it bound to this VAO

	// Loading shaders
	sprite.effect.load_from_file(shader_path(shader_name) + ".vs.glsl", shader_path(shader_name) + ".fs.glsl");
}
