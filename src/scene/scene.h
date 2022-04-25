#pragma once
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include "../physics/system.h"
#include "../graphics/light/light.h"
#include "../camera/camera.h"
#include "GLFW/glfw3.h"

class Scene {
  private:
    Camera m_camera;
    std::vector<Light> m_lights;
    std::vector<Object*> m_objects;
    // This format is optimal for batched/instanced rendering
    unsigned int dynamicDataBuffer;
    // [ shaderProgram ] [ meshFilePath ] [ material ] -> set(object*)
    std::unordered_map < std::string, std::unordered_map<std::string, std::unordered_map< std::string, std::unordered_set<Object*>>>> m_objects_map;
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
