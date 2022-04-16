#include "buff.hpp"
#include "game_components.hpp"
#include "particle.hpp"
#include "tiny_ecs.hpp"
// Calculates and initializes unit buffs for specified team 
// To be called after a level is initalized with units
BuffManager::BuffManager() {
  team = ally;
  calculateBuffs();
}

BuffManager::BuffManager(Team t) {
  team = t;
  calculateBuffs();
}

void BuffManager::calculateBuffs() {
  for (ECS::Entity u : ECS::registry<Unit>.entities) {
    // Skip units on bench
    BoardPosition bp = ECS::registry<BoardPosition>.get(u);
    // Units on bench aren't counted
    if (bp.isBench) {
      continue;
    }
    if (ECS::registry<TeamAffiliation>.get(u).t == team) {
      Identity identity = ECS::registry<Identity>.get(u);
      for (ClassType ct : identity.classes) {
        if (classCount.find(ct) != classCount.end()) {
          classCount[ct]++;
        } else {
          classCount[ct] = 1;
        }
        // If we just reached the minimum amount to buff the class then add the buff
        if (classCount[ct] == getClassBuffMin(ct)) {
          addBuff(ct);
        }
      }
    }
  }
}

void BuffManager::addBuff(ClassType ct) {
  for (ECS::Entity u : ECS::registry<Unit>.entities) {
    Identity identity = ECS::registry<Identity>.get(u);
    if (ECS::registry<TeamAffiliation>.get(u).t == team && 
        std::find(identity.classes.begin(), identity.classes.end(), ct) != identity.classes.end()) {
      CombatStats& cs = ECS::registry<CombatStats>.get(u);
      Motion motion = ECS::registry<Motion>.get(u);
      Buff buff = getClassBuff(ct);
      // Assert the buff is not in the list already! Maybe forgot to call removeActiveUnit when unit was removed from board?
      if (std::find(cs.currentBuffs.begin(), cs.currentBuffs.end(), buff) == cs.currentBuffs.end())
      {
          cs.currentBuffs.push_back(buff);
          cs.currentHp += buff.extraHp;
          cs.totalHp += buff.extraHp;
          cs.attack += buff.extraAttack;
          cs.range += buff.extraRange;
		  if (buff.extra_unit == 1) {
			  bool willAdd = true;
			  for (auto& e : ECS::registry<ExtraUnit>.entities) {
				  ExtraUnit eu = ECS::registry<ExtraUnit>.get(e);
				  if (eu.t == team)
					  willAdd = false;
			  }
			  if(willAdd)
				  ECS::registry<ExtraUnit>.emplace(ECS::Entity());
		  }
          cs.attacks_per_second += buff.extraAttackSpeed;
          //TODO: Add particle effect exploding from units when they are buffed!
          Particle::createParticles(20, motion.position, { 32, 32 }, classToString(ct), 500, 1500, 0.15, -100.f, 100.f, -100.f, 0.f);
      }
    }
  }
}

void BuffManager::removeBuff(ClassType ct) {
  for (ECS::Entity u : ECS::registry<Unit>.entities) {
    Identity identity = ECS::registry<Identity>.get(u);
    // If the unit is on the correct team and has the corresponding class
    if (ECS::registry<TeamAffiliation>.get(u).t == team && 
        std::find(identity.classes.begin(), identity.classes.end(), ct) != identity.classes.end()) {
      CombatStats& cs = ECS::registry<CombatStats>.get(u);
      Buff buff = getClassBuff(ct);
      // Assert that the buff is in the list! Maybe forgot to call addActiveUnit when unit added to board?
      // New change: ignores any units that did not call addActiveUnit, such as new units bought from shop
      if (std::find(cs.currentBuffs.begin(), cs.currentBuffs.end(), buff) != cs.currentBuffs.end())
      {
          cs.currentBuffs.erase(std::find(cs.currentBuffs.begin(), cs.currentBuffs.end(), buff));
          cs.currentHp -= buff.extraHp;
          cs.totalHp -= buff.extraHp;
          cs.attack -= buff.extraAttack;
		  if (buff.extra_unit == 1 && ECS::registry<ExtraUnit>.entities.size() > 0) {
			  for (auto& e : ECS::registry<ExtraUnit>.entities) {
				  ExtraUnit eu = ECS::registry<ExtraUnit>.get(e);
				  if (eu.t == team)
					ECS::registry<ExtraUnit>.remove(e);
			  }
		  }
          cs.range -= buff.extraRange;
          cs.attacks_per_second -= buff.extraAttackSpeed;
      }
    }
  }
}

void BuffManager::addActiveUnit(ECS::Entity unit) {
  Identity identity = ECS::registry<Identity>.get(unit);
  for (ClassType ct : identity.classes) {
    if (classCount.find(ct) != classCount.end()) {
      classCount[ct]++;
    } else {
      classCount[ct] = 1;
    }
    // If we just reached the minimum amount to buff the class then add the buff
    if (classCount[ct] == getClassBuffMin(ct)) {
      addBuff(ct);
    }
  }
}

void BuffManager::removeActiveUnit(ECS::Entity unit) {
  Identity identity = ECS::registry<Identity>.get(unit);
  for (ClassType ct : identity.classes) {
    classCount[ct]--;
    // If we are now below the minimum to get the class buff:
    if (classCount[ct] == getClassBuffMin(ct) - 1) {
      removeBuff(ct);
    }
  }
}

void BuffManager::reset() {
  for (auto it = classCount.begin(); it != classCount.end(); it++) {
    it->second = 0;
  }
}
