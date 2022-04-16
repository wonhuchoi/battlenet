// Header
#include "debug.hpp"
#include "tiny_ecs.hpp"
#include "render.hpp"

#include <cmath>
#include <iostream>
#include <string>

#include "render_components.hpp"

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// DON'T WORRY ABOUT THIS CLASS UNTIL ASSIGNMENT 2
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
namespace DebugSystem 
{

	void clearDebugComponents() {
		// Clear old debugging visualizations
		while (ECS::registry<DebugComponent>.entities.size() > 0) {
			ECS::ContainerInterface::remove_all_components_of(ECS::registry<DebugComponent>.entities.back());
		}
	}

	std::string colorToString(vec3 color) {
		return std::to_string(color[0]) + std::to_string(color[1]) + std::to_string(color[2]);
	}

	// Example usage for red line:
	// creatLineFromPoints({0,0}, {100,100}, {0.8, 0.1, 0.1})
	void createLineFromPoints(vec2 p1, vec2 p2, vec3 color) {
		auto entity = ECS::Entity();

		// Trig to place the line at correct spot/angle
		float minX = min(p1.x, p2.x);
		float maxX = max(p1.x, p2.x);
		float minY = min(p1.y, p2.y);
		float maxY = max(p1.y, p2.y);
		float xDiff = maxX -  minX;
		float yDiff = maxY - minY;
		vec2 centerPos = {minX + .5*xDiff, minY + .5*yDiff};
		float theta = atan2(p2.y - p1.y, p2.x - p1.x);
		float len = sqrt(xDiff*xDiff + yDiff*yDiff);

		std::string key = "thick_line_" + colorToString(color);
		ShadedMesh& resource = cache_resource(key);
		if (resource.effect.program.resource == 0) {
			// create a procedural circle
			constexpr float z = -0.1f;

			// Corner points
			ColoredVertex v;
			v.position = {-0.5, -0.5, z};
			v.color = color;
			resource.mesh.vertices.push_back(v);
			v.position = {-0.5, 0.5, z};
			v.color = color;
			resource.mesh.vertices.push_back(v);
			v.position = {0.5, -0.5, z};
			v.color = color;
			resource.mesh.vertices.push_back(v);
			v.position = {0.5, 0.5, z};
			v.color = color;
			resource.mesh.vertices.push_back(v);

			// Two triangles
			resource.mesh.vertex_indices.push_back(0);
			resource.mesh.vertex_indices.push_back(1);
			resource.mesh.vertex_indices.push_back(3);
			resource.mesh.vertex_indices.push_back(1);
			resource.mesh.vertex_indices.push_back(2);
			resource.mesh.vertex_indices.push_back(3);

			RenderSystem::createColoredMesh(resource, "colored_mesh");
		}

		// Store a reference to the potentially re-used mesh object (the value is
		// stored in the resource cache)
		ECS::registry<ShadedMeshRef>.emplace(entity, resource);

		// Create motion
		auto& motion = ECS::registry<Motion>.emplace(entity);
		// Scale the line horizontally with thickness 2 
		motion.scale = {len, 2};
		// Rotate the line by theta
		motion.angle = theta;
		motion.velocity = {0,0};
		motion.position = centerPos;
		motion.is_in_foreground = true;

		ECS::registry<DebugComponent>.emplace(entity);
	}

	bool in_debug_mode = false;
}
