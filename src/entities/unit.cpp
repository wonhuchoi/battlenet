#include "unit.hpp"
#include "game_components.hpp"
#include "render.hpp"
#include "render_components.hpp"
#include "world.hpp"
#include "tiny_ecs.hpp"
#include "ai.hpp"
#include <unordered_set>
#include <iostream>
#include <vector>


ECS::Entity Unit::createUnit(vec2 boardPosition, BoardInfo bi, WorldPosition wp, Team t, UnitType ut, CombatStats cs) { 
    auto entity = ECS::Entity();

	std::string key = teamToString(t) + "_" + unitToString(ut);
	ShadedMesh& resource = cache_resource(key);
    std::vector<vec2>& hull_resource = cache_hull_resource(key);
	if (resource.effect.program.resource == 0)
		RenderSystem::createSprite(resource, textures_path(key + ".png"), "textured", hull_resource, true);
    if (hull_resource.size() == 0)
        RenderSystem::createHull(hull_resource, textures_path(key + ".png"));
	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	ECS::registry<ShadedMeshRef>.emplace(entity, resource);

    ECS::registry<HullMeshRef>.emplace(entity, hull_resource);

    // Setting BoardPosition component
    BoardPosition& bp = ECS::registry<BoardPosition>.emplace(entity);
	bp.bPos = boardPosition;
	bp.startPos = boardPosition;

    int x = boardPosition.x, y = boardPosition.y;
    CellStruct& cell = ECS::registry<CellStruct>.get(bi.cells[y][x]);
    cell.isOccupied = true;

    // Setting motion component (For moving when unit moves to a new position on the board)
	Motion& motion = ECS::registry<Motion>.emplace(entity);
	motion.position = boardPosToWorldPos(bp.bPos, bi, wp);
    motion.initialPosition = motion.position;
    std::cout << "Initial unit position" << std::endl; 
    std::cout << motion.position.x << ", " << motion.position.y << std::endl;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
    // TODO (#4): Scale based on mesh/sprite size
    /*
	motion.scale = resource.mesh.original_size * 150.f;
	motion.scale.x *= -1; // point front to the right
    */
	motion.scale = vec2({ 1.f, 1.f }) * static_cast<vec2>(resource.texture.size);

    //Attach Bezier curve for motion
    BezierCurve& bc = ECS::registry<BezierCurve>.emplace(entity);
    bc.controlPoints = std::vector<vec2>{vec2(0,.5), vec2(0.5,1.5), vec2(1,2.5), vec2(1.5, 0.1)};
    bc.motionScale = 150;
    
	//Attach UnitAI
	ECS::registry<UnitAI>.emplace(entity, &entity);

    TeamAffiliation& team = ECS::registry<TeamAffiliation>.emplace(entity);
    team.t = t;

    Identity& identity = ECS::registry<Identity>.emplace(entity);
    identity.ut = ut;
    identity.classes = getClassesForUnit(ut);

    CombatStats& combat = ECS::registry<CombatStats>.emplace(entity);
    combat = cs;

    // Useful for battle system when iterating over enemies or allies.
    if (t == ally) {
        ECS::registry<Allies>.emplace(entity);
    } else {
        ECS::registry<Enemies>.emplace(entity);
    }
    ECS::registry<Unit>.emplace(entity);

    auto& IG_desc = ECS::registry<ImGuiDesc>.emplace(entity);
    IG_desc.rel_position_from_center = vec2(motion.scale.x / 2, -motion.scale.y / 2);
    IG_desc.description = {};
    IG_desc.activated = false;

    //add Cost components for shop
    auto& cost = ECS::registry<Cost>.emplace(entity);
    cost.cost = getCostForUnit(ut);

    return entity;
}

int Unit::damageUnit( ECS::Entity e, int amount) {
	CombatStats& cs = ECS::registry<CombatStats>.get(e);
	if (ECS::registry<Bind>.has(e)) {
		Bind& b = ECS::registry<Bind>.get(e);
		CombatStats& cs = ECS::registry<CombatStats>.get(*b.bound_unit);
		cs.currentHp = std::max(cs.currentHp - amount, 0);
	}
	if(!ECS::registry<Immune>.has(e))
		cs.currentHp = std::max(cs.currentHp - amount, 0);
    if (cs.currentHp <= 0.001 && ECS::registry<Enemies>.has(e) && ECS::registry<Unit>.get(e).alive) {
        Motion& m = ECS::registry<Motion>.get(e);
        Item::createItem(m.position);
        ECS::registry<Unit>.get(e).alive = false;
    }
	return amount;
}
