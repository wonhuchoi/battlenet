#pragma once

#include "common.hpp"
#include "game_components.hpp"
#include "tiny_ecs.hpp"
#include "render_components.hpp"
#include "animation.hpp"
#include "../ext/observable/observer.hpp"

struct InstancedMesh;
struct ShadedMesh;

// OpenGL utilities
void gl_has_errors();

// System responsible for setting up OpenGL and for rendering all the 
// visual entities in the game
class RenderSystem : public Observer<AnimationSystem>
{
public:
	void update(AnimationSystem *subject) {
		player_anim_x_offset = subject->player_anim_x_offset;
		player_anim_y_offset = subject->player_anim_y_offset;
		coin_anim_x_offset = subject->coin_anim_x_offset;
		coin_anim_y_offset = subject->coin_anim_y_offset;
		space_invader_anim_x_offset = subject->space_invader_anim_x_offset;
		space_invader_anim_y_offset = subject->space_invader_anim_y_offset;
	};
	// Initialize the window
	RenderSystem(GLFWwindow& window);

	// Destroy resources associated to one or all entities created by the system
	~RenderSystem();

	// Draw all entities
	void draw(vec2 cam_transform, vec2 window_size_in_game_units);

	// Expose the creating of visual representations to other systems
	static void createSprite(ShadedMesh& mesh_container, std::string texture_path, std::string shader_name);
	static void createSprite(ShadedMesh& mesh_container, std::string texture_path, std::string shader_name, std::vector<vec2>& hull, bool withHull);
	static void createColoredMesh(ShadedMesh& mesh_container, std::string shader_name);

	static void createHull(std::vector<vec2>& hull_container, std::string texture_path);

private:
	// Initialize the screeen texture used as intermediate render target
	// The draw loop first renders to this texture, then it is used for the water shader
	void initScreenTexture();

	// Internal drawing functions for each entity type
	void drawTexturedMesh(ECS::Entity entity, const mat3& projection);
	void drawToScreen();

	// Window handle
	GLFWwindow& window;

	// Screen texture handles
	GLuint frame_buffer;
	ShadedMesh screen_sprite;
	GLResource<RENDER_BUFFER> depth_render_buffer_id;
	ECS::Entity screen_state_entity;

	float player_anim_x_offset;
	float player_anim_y_offset;

	float coin_anim_x_offset;
	float coin_anim_y_offset;

	float space_invader_anim_x_offset;
	float space_invader_anim_y_offset;
};
