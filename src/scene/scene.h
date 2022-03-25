#pragma once
#include <vector>
#include "../physics/system.h"
#include "../graphics/light/light.h"

class Scene {
  private:
    std::vector<Light> lights;
    System physicsSystem;

  public:
    System* getSystem();
    void render(glm::mat4 view);

};
