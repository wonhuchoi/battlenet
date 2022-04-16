#include "common.hpp"
#include "tiny_ecs.hpp"
#include "game_components.hpp"


#include<vector>

namespace util { 
  //get angle from source to destination
  float get_dir_angle(vec2 source, vec2 destination);
  float get_distance(vec2 source, vec2 destination);
  void createLine(vec2 position, vec2 size);
  void createLine(vec2 position, vec2 size, vec3 color);
  void createHPBars(std::vector<ECS::Entity> entities, vec3 color);
  void createManaBars(std::vector<ECS::Entity> entities, vec3 color);
  bool inRange(vec2 p1, vec2 p2, int range);
  vec2 normalize(vec2 vec);
  std::vector<vec2> transformHull(std::vector<vec2>& oldHull, Motion motion);
  float minimum_distance(vec2 v, vec2 w, vec2 p);
  vec2 deCasteljau_bezier(std::vector<vec2> controlPoints, float t);
}
