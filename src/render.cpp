// internal
#include "render.hpp"
#include "render_components.hpp"
#include "tiny_ecs.hpp"
#include "player.hpp"
#include "pause.hpp"
#include "projectile.hpp"
#include "item.hpp"
#include "space_invader.hpp"

#include <iostream>

void RenderSystem::drawTexturedMesh(ECS::Entity entity, const mat3& projection)
{
	auto& motion = ECS::registry<Motion>.get(entity);
	auto& texmesh = *ECS::registry<ShadedMeshRef>.get(entity).reference_to_cache;
	// Transformation code, see Rendering and Transformation in the template specification for more info
	// Incrementally updates transformation matrix, thus ORDER IS IMPORTANT
	Transform transform;
	transform.translate(motion.position);
	transform.rotate(motion.angle);
	transform.scale(motion.scale);

	// Setting shaders
	glUseProgram(texmesh.effect.program);
	glBindVertexArray(texmesh.mesh.vao);
	gl_has_errors();

	// Enabling alpha channel for textures
	glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);
	gl_has_errors();

	GLint transform_uloc = glGetUniformLocation(texmesh.effect.program, "transform");
	GLint projection_uloc = glGetUniformLocation(texmesh.effect.program, "projection");
	gl_has_errors();

	// Setting vertex and index buffers
	glBindBuffer(GL_ARRAY_BUFFER, texmesh.mesh.vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, texmesh.mesh.ibo);
	gl_has_errors();

	// Input data location as in the vertex buffer
	GLint in_position_loc = glGetAttribLocation(texmesh.effect.program, "in_position");
	GLint in_texcoord_loc = glGetAttribLocation(texmesh.effect.program, "in_texcoord");
	GLint in_color_loc = glGetAttribLocation(texmesh.effect.program, "in_color");
	if (in_texcoord_loc >= 0)
	{
		glEnableVertexAttribArray(in_position_loc);
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), reinterpret_cast<void*>(0));
		glEnableVertexAttribArray(in_texcoord_loc);
		glVertexAttribPointer(in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), reinterpret_cast<void*>(sizeof(vec3))); // note the stride to skip the preceeding vertex position
		// Enabling and binding texture to slot 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texmesh.texture.texture_id);
	}
	else if (in_color_loc >= 0)
	{
		glEnableVertexAttribArray(in_position_loc);
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(ColoredVertex), reinterpret_cast<void*>(0));
		glEnableVertexAttribArray(in_color_loc);
		glVertexAttribPointer(in_color_loc, 3, GL_FLOAT, GL_FALSE, sizeof(ColoredVertex), reinterpret_cast<void*>(sizeof(vec3)));
	}
	else
	{
		throw std::runtime_error("This type of entity is not yet supported");
	}
	gl_has_errors();

	// set animation frames for player
	if (ECS::registry<Player>.has(entity)) {
		// Set x and y frames
		GLuint frame_x = glGetUniformLocation(texmesh.effect.program, "frame_x");
		GLuint frame_y = glGetUniformLocation(texmesh.effect.program, "frame_y");
		glUniform1f(frame_x, player_anim_x_offset);
		glUniform1f(frame_y, player_anim_y_offset);
		gl_has_errors();
	}

	// set animation frames for coin
	if (ECS::registry<Item>.has(entity)) {
		// Set x and y frames
		GLuint frame_x = glGetUniformLocation(texmesh.effect.program, "frame_x");
		GLuint frame_y = glGetUniformLocation(texmesh.effect.program, "frame_y");
		glUniform1f(frame_x, coin_anim_x_offset);
		glUniform1f(frame_y, coin_anim_y_offset);
		gl_has_errors();
	}

	// set animation frames for space invader
	if (ECS::registry<SpaceInvader>.has(entity)) {
		// Set x and y frames
		GLuint frame_x = glGetUniformLocation(texmesh.effect.program, "frame_x");
		GLuint frame_y = glGetUniformLocation(texmesh.effect.program, "frame_y");
		glUniform1f(frame_x, space_invader_anim_x_offset);
		glUniform1f(frame_y, space_invader_anim_y_offset);
		gl_has_errors();
	}

	// Getting uniform locations for glUniform* calls
	GLint color_uloc = glGetUniformLocation(texmesh.effect.program, "fcolor");
	glUniform3fv(color_uloc, 1, (float*)&texmesh.texture.color);
	gl_has_errors();

	// Set clock
	GLfloat time_uloc = glGetUniformLocation(texmesh.effect.program, "time");
	glUniform1f(time_uloc, static_cast<float>(glfwGetTime() * 50.0f));
	gl_has_errors();

	// Get number of indices from index buffer, which has elements uint16_t
	GLint size = 0;
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
	gl_has_errors();
	GLsizei num_indices = size / sizeof(uint16_t);
	//GLsizei num_triangles = num_indices / 3;

	// Setting uniform values to the currently bound program
	glUniformMatrix3fv(transform_uloc, 1, GL_FALSE, (float*)&transform.mat);
	glUniformMatrix3fv(projection_uloc, 1, GL_FALSE, (float*)&projection);
	gl_has_errors();

	// Drawing of num_indices/3 triangles specified in the index buffer
	glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT, nullptr);
	glBindVertexArray(0);
}

// Draw the intermediate texture to the screen, with some distortion to simulate water
void RenderSystem::drawToScreen() 
{
	// Setting shaders
	glUseProgram(screen_sprite.effect.program);
	glBindVertexArray(screen_sprite.mesh.vao);
	gl_has_errors();

	// Clearing backbuffer
	int w, h;
	glfwGetFramebufferSize(&window, &w, &h);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, w, h);
	glDepthRange(0, 10);
	glClearColor(1.f, 0, 0, 1.0);
	glClearDepth(1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	gl_has_errors();
	
	// Disable alpha channel for mapping the screen texture onto the real screen
	glDisable(GL_BLEND); // we have a single texture without transparency. Areas with alpha <1 cab arise around the texture transparency boundary, enabling blending would make them visible.
	glDisable(GL_DEPTH_TEST);

	glBindBuffer(GL_ARRAY_BUFFER, screen_sprite.mesh.vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, screen_sprite.mesh.ibo); // Note, GL_ELEMENT_ARRAY_BUFFER associates indices to the bound GL_ARRAY_BUFFER
	gl_has_errors();

	// Draw the screen texture on the quad geometry
	gl_has_errors();

	// Set clock
	GLuint time_uloc       = glGetUniformLocation(screen_sprite.effect.program, "time");
	glUniform1f(time_uloc, static_cast<float>(glfwGetTime() * 10.0f));
	gl_has_errors();

	// Set the vertex position and vertex texture coordinates (both stored in the same VBO)
	GLint in_position_loc = glGetAttribLocation(screen_sprite.effect.program, "in_position");
	glEnableVertexAttribArray(in_position_loc);
	glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), (void*)0);
	GLint in_texcoord_loc = glGetAttribLocation(screen_sprite.effect.program, "in_texcoord");
	glEnableVertexAttribArray(in_texcoord_loc);
	glVertexAttribPointer(in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), (void*)sizeof(vec3)); // note the stride to skip the preceeding vertex position
	gl_has_errors();

	// Bind our texture in Texture Unit 0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, screen_sprite.texture.texture_id);

	// Draw
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr); // two triangles = 6 vertices; nullptr indicates that there is no offset from the bound index buffer
	glBindVertexArray(0);
	gl_has_errors();
}

// Render our game world
// http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-14-render-to-texture/
void RenderSystem::draw(vec2 cam_transform, vec2 window_size_in_game_units)
{
	// Getting size of window
	ivec2 frame_buffer_size; // in pixels
	glfwGetFramebufferSize(&window, &frame_buffer_size.x, &frame_buffer_size.y);

	// First render to the custom framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
	gl_has_errors();

	// Clearing backbuffer
	glViewport(0, 0, frame_buffer_size.x, frame_buffer_size.y);
	glDepthRange(0.00001, 10);
	glClearColor(0, 0, 0, 1.0);
	glClearDepth(1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	gl_has_errors();

	// Fake projection matrix, scales with respect to window coordinates
	float left = 0.f;
	float top = 0.f;
	float right = window_size_in_game_units.x;
	float bottom = window_size_in_game_units.y;

	float sx = 2.f / (right - left);
	float sy = 2.f / (top - bottom);
	float tx = -(right + left) / (right - left);
	float ty = -(top + bottom) / (top - bottom);
	mat3 projection_2D{ { sx, 0.f, 0.f },{ 0.f, sy, 0.f },{ tx, ty, 1.f } };

	//Handle Camera Transform updated in world::step()
	Transform zoom;
	zoom.scale({1.2, 1.2});
	//projection_2D = projection_2D * zoom.mat;

	Transform cameraTransform;
	cameraTransform.translate(cam_transform);
	projection_2D = projection_2D * cameraTransform.mat;

	std::vector<ECS::Entity> entities_in_foreground;
	// Draw all textured meshes that have a position and size component
	for (ECS::Entity entity : ECS::registry<ShadedMeshRef>.entities)
	{
		if (!ECS::registry<Motion>.has(entity))
			continue;
		bool is_ranged = false;
		// Dont render dead units
		if (ECS::registry<CombatStats>.has(entity) && (ECS::registry<CombatStats>.get(entity).currentHp<=0) )
			continue;
		if (ECS::registry<CombatStats>.has(entity) && (ECS::registry<CombatStats>.get(entity).range > 1))
			is_ranged = true;
		
		// Note, its not very efficient to access elements indirectly via the entity albeit iterating through all Sprites in sequence
		if (ECS::registry<Motion>.get(entity).is_in_foreground) {
			entities_in_foreground.push_back(entity);
		}
		else {
			if (is_ranged) {
				auto& m = ECS::registry<Motion>.get(entity);
					m.scale.y = 30; 
				drawTexturedMesh(entity, projection_2D);
			}
			else {
				drawTexturedMesh(entity, projection_2D);
			}
			
			gl_has_errors();
		}
	}
	for (ECS::Entity entity : entities_in_foreground)
	{
		drawTexturedMesh(entity, projection_2D);
		gl_has_errors();
	}
	for (ECS::Entity e : ECS::registry<Pause>.entities) {
		if (ECS::registry<Motion>.get(e).is_in_foreground) {
			drawTexturedMesh(e, projection_2D);
			gl_has_errors();
		}
	}

	// Truely render to the screen
	drawToScreen();

	// flicker-free display with a double buffer
	// Removed to allow imgui
	//glfwSwapBuffers(&window);
}

void gl_has_errors()
{
	GLenum error = glGetError();

	if (error == GL_NO_ERROR)
		return;
	
	const char* error_str = "";
	while (error != GL_NO_ERROR)
	{
		switch (error)
		{
		case GL_INVALID_OPERATION:
			error_str = "INVALID_OPERATION";
			break;
		case GL_INVALID_ENUM:
			error_str = "INVALID_ENUM";
			break;
		case GL_INVALID_VALUE:
			error_str = "INVALID_VALUE";
			break;
		case GL_OUT_OF_MEMORY:
			error_str = "OUT_OF_MEMORY";
			break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			error_str = "INVALID_FRAMEBUFFER_OPERATION";
			break;
		}

		std::cerr << "OpenGL:" << error_str << std::endl;
		error = glGetError();
	}
	throw std::runtime_error("last OpenGL error:" + std::string(error_str));
}
