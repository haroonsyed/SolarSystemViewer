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

    unsigned int m_numFloatsPerModelData;
    // The m_objects_map contains all objects registered to render in the scene, as well as their offset in the modelBuffer
    // [ groupInstanceKey (shader+mesh+material) ] -> std::pair< SSBO, uMap< Object*, SSBO_ID > >
    std::unordered_map< std::string, 
      std::pair< 
        unsigned int, 
        std::unordered_map<Object*, unsigned int> 
      > 
    > m_objects_map;

    System m_physicsSystem;
    float m_universeScaleFactor; // Used to scale the distance between objects in scene.
                                 // Compounds ontop of unit system defined in JSON document

    std::string getInstanceGroupKey(Object* obj);
    unsigned int createModelBuffer();
    void registerObjectToScene(Object* obj);
    void updateObjectInScene(Object* obj);

  public:
    Scene(GLFWwindow* window);
    System* getPhysicsSystem();
    Camera* getCamera();
    float getUniverseScaleFactor();
    void loadScene(std::string sceneFilePath);
    void render();

};
