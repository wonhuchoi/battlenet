
#include "player.hpp"
#include "render.hpp"
#include "animation.hpp"

// Creates a player entity with motion, team and identity component
// We might make multiple of these if we ever:
//      -Make it multiplayer
//      -Include an AI enemy player that also roams the board.
ECS::Entity Player::createPlayer(vec2 position, Team t, UnitType ut)
{
	auto entity = ECS::Entity();

	std::string key = "player";
	ShadedMesh &resource = cache_resource(key);
	if (resource.effect.program.resource == 0)
		AnimationSystem::load(resource, textures_path("player_sprite_sheet.png"), "spritesheet", key);

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	ECS::registry<ShadedMeshRef>.emplace(entity, resource);

	// Setting initial motion values
	Motion &motion = ECS::registry<Motion>.emplace(entity);
	motion.position = position;
	motion.angle = 0.f;
	motion.velocity = {0.f, 0.f};

	motion.scale = { 100.f, 100.f };
	motion.is_in_foreground = true;

	// Setting identity component
	Identity &identity = ECS::registry<Identity>.emplace(entity);
	identity.ut = ut;

	// Setting team component
	TeamAffiliation &team = ECS::registry<TeamAffiliation>.emplace(entity);
	team.t = t;

	ECS::registry<Player>.emplace(entity);
	ECS::registry<Animated>.emplace(entity);

	return entity;
}
