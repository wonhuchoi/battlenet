// Header
#include "space_invader.hpp"
#include "render.hpp"

#include <iostream>
#include <stdio.h>
#include <string>
#include <stdlib.h>

using namespace std;

std::map<InvaderType, std::string> paths = {
	{regular, "space_invader_sprite_sheet.png"},
	{floating, "space_invader_sprite_sheet_2.png"},
};

std::string getInvaderPath(InvaderType it) {
	return paths[it];
}

// Needs to match key in animation.cpp
std::map<InvaderType, std::string> keys = {
	{regular, "space_invader"},
	{floating, "space_invader_2"},
};

ECS::Entity SpaceInvader::createSpaceInvader(vec2 position, vec2 scale, InvaderType type)
{
	auto entity = ECS::Entity();

	// Create rendering primitives
	std::string key = keys[type];
	ShadedMesh& resource = cache_resource(key);
	if (resource.effect.program.resource == 0)
		AnimationSystem::load(resource, textures_path(getInvaderPath(type)), "spritesheet", key);

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	ECS::registry<ShadedMeshRef>.emplace(entity, resource);

	// Initialize the motion
	auto& motion = ECS::registry<Motion>.emplace(entity);

	motion.position = position;
	motion.angle = 0.f;
	motion.velocity = {0.f, 0.f};
	motion.scale = scale;
	motion.is_in_foreground = true;

	ECS::registry<SpaceInvader>.emplace(entity);
	ECS::registry<Animated>.emplace(entity);

	return entity;
}
