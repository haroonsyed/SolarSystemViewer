#pragma once
#include <vector>
#include <unordered_map>
#include <string>
#include "../physics/system.h"
#include "../graphics/light/light.h"
#include "../camera/camera.h"
#include "GLFW/glfw3.h"
#include "../graphics/skybox/skybox.h"

class Scene {
  private:
    Camera m_camera;
    unsigned int m_MAX_NUM_LIGHTS = 20;
    std::vector<Light> m_lights;
    Skybox skybox;

    unsigned int m_uniformBuffer;
    unsigned int m_particleVAO;

    /// Uniform Buffer data for this scene
    float m_ambientStrength;
    float m_specularStrength;
    float m_phongExponent;

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

    void genUniformBuffer();
    unsigned int getUBOSize();
    std::string getInstanceGroupKey(Object* obj);
    unsigned int createModelBuffer();
    std::vector<glm::mat4> getModelMatrices(Object* obj);
    void registerObjectToScene(Object* obj);
    void updateObjectInScene(Object* obj);
    void bindObjectWithModelMatrix(Object* obj);

  public:
    Scene(GLFWwindow* window);
    System* getPhysicsSystem();
    Camera* getCamera();
    float getUniverseScaleFactor();
    void loadScene(std::string sceneFilePath);
    void render();
    void renderSkybox();

};
