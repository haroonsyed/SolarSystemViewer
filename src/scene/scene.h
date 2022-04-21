#pragma once
#include <vector>
#include "../physics/system.h"
#include "../graphics/light/light.h"
#include "../camera/camera.h"
#include "GLFW/glfw3.h"

class Scene {
  private:
    Camera m_camera;
    std::vector<Light> m_lights;
    std::vector<Object*> m_objects;
    System m_physicsSystem;
    float m_universeScaleFactor; // Used to scale the distance between objects in scene.
                                 // Compounds ontop of unit system defined in JSON document

  public:
    Scene(GLFWwindow* window);
    System* getPhysicsSystem();
    Camera* getCamera();
    float getUniverseScaleFactor();
    void loadScene(std::string sceneFilePath);
    void render();

};
