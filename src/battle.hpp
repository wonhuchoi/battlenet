#pragma once

#include <random>
#include <vector>
#include <unordered_set>
#include "common.hpp"
#include "tiny_ecs.hpp"
#include <subject.hpp>

struct vec2Hash {
  size_t operator()(const vec2& v)const {
    return std::hash<float>()(v.x) ^ std::hash<float>()(v.y) << 1;
  }
};

struct vec2Compare {
  bool operator()(const vec2& v1, const vec2& v2)const {
    return v1.x == v2.x && v1.y == v2.y;
  }
};

typedef std::unordered_set<vec2, vec2Hash, vec2Compare> USetVec2;

// BattleSystem is a Subject
// to subscribe to notifications, extend Observer<BattleSystem> in listener class and implement the `update` fcn
// to notify, call `notifyObservers` in this class
class BattleSystem : public Subject<BattleSystem>
{
  public:
    void step(float elapsed_ms);
  private:
    // Countdown to next battle system step
    int nextStep = 1000;
    // Returns the position of the next adjacent square to move to, or the current position if no adjacent squares are available.
    vec2 moveEntity(ECS::Entity e, USetVec2 targets, USetVec2& occupied, vec2 boardSize);
    void handleMovement(std::vector<ECS::Entity> allies, std::vector<ECS::Entity> enemies);
    std::unordered_set<int> handleAttacks(std::vector<ECS::Entity> allies, std::vector<ECS::Entity> enemies);
    USetVec2 occupiedSquares(std::vector<ECS::Entity> units);
    void resolveFuturePos(std::vector<ECS::Entity> entities);
    std::default_random_engine rng;
    std::uniform_real_distribution<float> uniform_dist;
};
