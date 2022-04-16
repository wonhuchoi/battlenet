#pragma once

#include <random>
#include <vector>
#include <unordered_set>
#include "common.hpp"
#include "tiny_ecs.hpp"
#include <subject.hpp>

// BattleSystem is a Subject
// to subscribe to notifications, extend Observer<BattleSystem> in listener class and implement the `update` fcn
// to notify, call `notifyObservers` in this class
class AttackSystem : public Subject<AttackSystem>
{
  public:
    void step(float elapsed_ms);
  private:
    void handleAttacks(std::vector<ECS::Entity> allies, std::vector<ECS::Entity> enemies, float elapsed_ms);
};
