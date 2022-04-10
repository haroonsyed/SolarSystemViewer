#pragma once
#include <vector>
#include "../physics/system.h"
#include "../graphics/light/light.h"
#include "../camera/camera.h"
#include "../input/inputController.h"

class Scene {
  private:
    Camera m_camera;
    InputController* m_inputController;
    std::vector<Light> m_lights;
    System m_physicsSystem;
    float m_universeScaleFactor; // Used to scale the distance between objects in scene.
                                 // Compounds ontop of unit system defined in JSON document

  public:
    Scene(GLFWwindow* window);
    System* getPhysicsSystem();
    void loadScene(std::string sceneFilePath);
    void update(float deltaT);
    void render();

};
