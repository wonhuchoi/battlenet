// Functions that control unit class buffs and associated stats.
#include "common.hpp"
#include "observer.hpp"
#include "tiny_ecs.hpp"
#include "game_components.hpp"
#include "unit.hpp"

class BuffManager {
  public:
    BuffManager();
    BuffManager(Team t);
    void calculateBuffs();
    void reset();
    void addActiveUnit(ECS::Entity unit);
    void removeActiveUnit(ECS::Entity unit);
    Team team;
    std::map<ClassType, int> classCount;
  private:
    void addBuff(ClassType ct);
    void removeBuff(ClassType ct);
};
