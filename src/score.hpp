#pragma once

#include <vector>

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "battle.hpp"
#include "attack.hpp"
#include <observer.hpp>

class ScoreSystem : public Observer<AttackSystem>
{
public:
  void update (AttackSystem*subject);
  void step(float elapsed_ms, vec2 window_size_in_game_units);
  int enemiesDefeated = 0;
  int waveEnemies = 0;
};
