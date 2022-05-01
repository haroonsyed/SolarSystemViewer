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

    // Contains all model transforms in SSBO for on-vram updates with compute shaders
    unsigned int m_modelBuffer;
    unsigned int m_modelBufferSize;

    // The m_objects_map contains all objects registered to render in the scene, as well as their offset in the dynamicDataBuffer
    // [ shaderProgram ] [ meshFilePath ] [ material ] [ Object* ] -> unsigned int
    std::unordered_map < std::string, std::unordered_map<std::string, std::unordered_map< std::string, std::unordered_map< Object*, unsigned int > >>> m_objects_map;
    
    // Contains objects that need to be updated in a given frame
    std::vector < Object* > m_newAndUpdatedObjects;

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
