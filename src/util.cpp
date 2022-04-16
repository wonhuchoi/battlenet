// Header
#include "util.hpp"
#include "debug.hpp"
#include "tiny_ecs.hpp"
#include "render.hpp"


#include <cmath>
#include <iostream>

#include "render_components.hpp"

namespace util {
	float get_dir_angle(vec2 source, vec2 destination)
	{
		vec2 relative_pos = destination - source;
		float rel_magnitude = sqrt(pow(relative_pos.x, 2) + pow(relative_pos.y, 2));
		vec2 relative_dir = { relative_pos.x / rel_magnitude, relative_pos.y / rel_magnitude };

		vec2 default_dir = { 1,0 };

		float theta = acos(dot(relative_dir, default_dir));

		if (destination.y < source.y)
			return -theta;
		else
			return theta;
	}

	float get_distance(vec2 source, vec2 destination) {
		return sqrt(pow(destination.x - source.x, 2) + pow(destination.y - source.y, 2) * 1.0);
	}

	vec2 normalize(vec2 vec)
	{
		float norm = sqrt(pow(vec.x, 2) + pow(vec.y, 2));
		return {vec.x / norm, vec.y / norm};
	}

	void createHPBars(std::vector<ECS::Entity> entities, vec3 color) {
		for (auto e : entities) {
			if (ECS::registry<Motion>.has(e) && ECS::registry<CombatStats>.has(e)) {
				//TODO make relative to cell size?
				auto& m = ECS::registry<Motion>.get(e);
				auto& cs = ECS::registry<CombatStats>.get(e);
				float barSize = 0.9;
				float barHeight = 8;
				createLine(vec2(m.position.x, m.position.y - m.scale.x/2 - 10), vec2( (float)m.scale.x * barSize+barHeight*0.5, barHeight*1.5 ), { 1.0, 1.0, 1.0 });

				float leftShift = (float)m.scale.x * barSize - std::max(0.f, (float)m.scale.x * barSize * cs.currentHp / cs.totalHp);
				createLine(vec2(m.position.x-(leftShift/2.0), m.position.y - m.scale.x / 2 - 10), vec2( std::max(0.f, (float)m.scale.x * barSize * cs.currentHp / cs.totalHp), barHeight ), color);
			}
		}
	}

	void createManaBars(std::vector<ECS::Entity> entities, vec3 color) {
		for (auto e : entities) {
			if (ECS::registry<Motion>.has(e) && ECS::registry<CombatStats>.has(e)) {
				//TODO make relative to cell size?
				auto& m = ECS::registry<Motion>.get(e);
				auto& cs = ECS::registry<CombatStats>.get(e);
				float barSize = 0.9;
				float barHeight = 4;
				createLine(vec2(m.position.x, m.position.y - m.scale.x / 2 - 2), vec2((float)m.scale.x * barSize + barHeight * 0.5, barHeight * 1.5), { 1.0, 1.0, 1.0 });

				float leftShift = (float)m.scale.x * barSize - std::max(0.f, (float)m.scale.x * barSize * cs.currMana / cs.manaToCast);
				createLine(vec2(m.position.x - (leftShift / 2.0), m.position.y - m.scale.x / 2 - 2), vec2(std::max(0.f, (float)m.scale.x * barSize * cs.currMana / cs.manaToCast), barHeight), color);
			}
		}
	}

	void createLine(vec2 position, vec2 scale) {
		vec3 red = { 0.8,0.1,0.1 };
		createLine(position, scale, red);
	}

	void createLine(vec2 position, vec2 scale, vec3 color) {
		auto entity = ECS::Entity();

		std::string key = "thick_line" + std::to_string(color[0]) + std::to_string(color[1]) + std::to_string(color[2]);
		ShadedMesh& resource = cache_resource(key);
		if (resource.effect.program.resource == 0) {
			// create a procedural circle
			constexpr float z = -0.1f;

			// Corner points
			ColoredVertex v;
			v.position = { -0.5,-0.5,z };
			v.color = color;
			resource.mesh.vertices.push_back(v);
			v.position = { -0.5,0.5,z };
			v.color = color;
			resource.mesh.vertices.push_back(v);
			v.position = { 0.5,0.5,z };
			v.color = color;
			resource.mesh.vertices.push_back(v);
			v.position = { 0.5,-0.5,z };
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

		// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
		ECS::registry<ShadedMeshRef>.emplace(entity, resource);

		// Create motion
		auto& motion = ECS::registry<Motion>.emplace(entity);
		motion.angle = 0.f;
		motion.velocity = { 0, 0 };
		motion.position = position;
		motion.scale = scale;
		motion.is_in_foreground = true;

		//need to move this TODO
		ECS::registry<DebugComponent>.emplace(entity);
	}

	// Returns true if p1 is within range of p2 (euclidean dist)
	bool inRange(vec2 p1, vec2 p2, int range) {
	  float dist = sqrt((p1.x-p2.x)*(p1.x-p2.x) + (p1.y-p2.y)*(p1.y-p2.y));
	  return range >= dist;
	}

	std::vector<vec2> transformHull(std::vector<vec2> & oldHull, Motion motion) {
		std::vector<vec2> hull;
		for (auto p : oldHull) {
			float x1 = p[0] * motion.scale.x;
			float y1 = p[1] * motion.scale.y;

			float xnew = cos(motion.angle) * x1 - sin(motion.angle) * y1 + motion.position.x;
			float ynew = sin(motion.angle) * x1 + cos(motion.angle) * y1 + motion.position.y;

			hull.push_back(vec2(xnew, ynew));
		}
		return hull;
	}
	//from: https://stackoverflow.com/questions/849211/shortest-distance-between-a-point-and-a-line-segment
	float minimum_distance(vec2 v, vec2 w, vec2 p) {
		// Return minimum distance between line segment vw and point p
		const float l2 = pow(distance(w, v), 2.0);  // i.e. |w-v|^2 -  avoid a sqrt
		if (l2 == 0.0) return distance(p, v);   // v == w case
		// Consider the line extending the segment, parameterized as v + t (w - v).
		// We find projection of point p onto the line.
		// It falls where t = [(p-v) . (w-v)] / |w-v|^2
		// We clamp t from [0,1] to handle points outside the segment vw.
		const float t = std::max(0.f, std::min(1.f, dot(p - v, w - v) / l2));
		const vec2 projection = v + t * (w - v);  // Projection falls on the segment
		return distance(p, projection);
	}

	// Evaluates a bezier curve at time = t [0,1]
	// Speed: O(n choose 2) where n is the degree of the bezier curve
	// Based on: https://en.wikipedia.org/wiki/De_Casteljau%27s_algorithm#Definition
	vec2 deCasteljau_bezier(std::vector<vec2> controlPoints, float t) {
		if (controlPoints.size() == 1) {
			return controlPoints[0];
		} else {
			std::vector<vec2> newPts;
			for (uint i = 0; i < controlPoints.size() - 1; i++) {
				vec2 p1 = (1-t) * controlPoints[i];
				vec2 p2 = (t) * controlPoints[i+1];
				newPts.push_back(p1 + p2);
			}
			return deCasteljau_bezier(newPts, t);
		}
	}
}
