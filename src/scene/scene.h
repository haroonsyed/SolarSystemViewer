#pragma once
#include <vector>
#include "../physics/system.h"
#include "../graphics/light/light.h"

class Scene {
  private:
    std::vector<Light> m_lights;
    System m_physicsSystem;
    float m_universeScaleFactor; // Used to scale the distance between objects in scene.
                                 // Compounds ontop of unit system defined in JSON document

  public:
    System* getPhysicsSystem();
    void loadScene(std::string sceneFilePath);
    void render(glm::mat4& view);

};
