#include <GL/glew.h>
#include "scene.h"
#include <iostream>
#include <fstream>
#include <glm/gtc/type_ptr.hpp>
#include "nlohmann/json.hpp"
#include "../graphics/shader/shaderManager.h"
#include "../graphics/mesh/meshManager.h"
#include "../graphics/texture/textureManager.h"
#include "../config.h"

Scene::Scene(GLFWwindow* window) {
  m_universeScaleFactor = 1.0f;
  m_numFloatsPerModelData = 16 * 4; // mat4 (scale, rot, transform, modelMatrix)
}

std::string Scene::getInstanceGroupKey(Object* obj) {
  auto shaders = obj->getShaders();
  auto material = obj->getTextures();
  std::string materialName = "";
  for (auto const& str : material) {
    materialName += str;
  }
  return shaders.first + shaders.second + obj->getMesh() + materialName;
}

System* Scene::getPhysicsSystem() {
  return &m_physicsSystem;
}

Camera* Scene::getCamera() {
  return &m_camera;
}

float Scene::getUniverseScaleFactor() {
  return m_universeScaleFactor;
}

void Scene::loadScene(std::string sceneFilePath) {

  using namespace nlohmann; // json lib namespace

  // Load scene from json file
  std::ifstream file(sceneFilePath);
  std::string scene;
  std::getline(file, scene, '\0');
  json jScene = json::parse(scene);

  // Get units
  m_universeScaleFactor = jScene["UniverseScaleFactor"].get<float>();
  float physicsDistanceFactor = jScene["PhysicsDistanceFactor"].get<float>();
  float physicsMassFactor = jScene["PhysicsMassFactor"].get<float>();

  // Setup camera
  m_camera.setCameraPosition(
    glm::vec3(
      jScene["CameraPosition"]["x"].get<float>() / physicsDistanceFactor / m_universeScaleFactor,
      jScene["CameraPosition"]["y"].get<float>() / physicsDistanceFactor / m_universeScaleFactor,
      jScene["CameraPosition"]["z"].get<float>() / physicsDistanceFactor / m_universeScaleFactor
    )
  );

  // Setup physics
  m_physicsSystem.setPhysicsDistanceFactor(physicsDistanceFactor);
  m_physicsSystem.setPhysicsMassFactor(physicsMassFactor);

  // Construct scene. In units specified in SI units of json
  for (auto gravBodyJSON : jScene["GravBodies"]) {
    GravBody* body = new GravBody(physicsDistanceFactor, physicsMassFactor, gravBodyJSON);
    m_physicsSystem.addBody(body); // Add gravBody to physics system

    // Tell scene to register this object
    registerObjectToScene(body);

  }

  // Construct lights
  for (auto lightJSON : jScene["Lights"]) {
    Light light;

    light.setPosition(
      lightJSON["position"]["x"].get<float>() / physicsDistanceFactor / m_universeScaleFactor,
      lightJSON["position"]["y"].get<float>() / physicsDistanceFactor / m_universeScaleFactor,
      lightJSON["position"]["z"].get<float>() / physicsDistanceFactor / m_universeScaleFactor
    );

    light.setColor(
      lightJSON["color"]["red"].get<float>(),
      lightJSON["color"]["green"].get<float>(),
      lightJSON["color"]["blue"].get<float>()
    );

    light.setIntensity(lightJSON["intensity"].get<float>());

    m_lights.push_back(light);

  }

}

unsigned int Scene::createModelBuffer() {
  unsigned int SSBO;
  glGenBuffers(1, &SSBO);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO);
  glBufferData(GL_SHADER_STORAGE_BUFFER, m_numFloatsPerModelData * 1000000 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
  return SSBO;
}

// Returns the scale, rotation and translation of the object
std::vector<glm::mat4> Scene::getModelMatrices(Object* obj) {

  glm::mat4 scale = glm::mat4(1.0);
  scale = glm::scale(scale, glm::vec3(obj->getScale()));
  glm::mat4 rotation = obj->getRotationMat();
  glm::mat4 translation = glm::mat4(1.0f);
  translation = glm::translate(translation, obj->getPosition() / m_universeScaleFactor);


  return { scale, rotation, translation };

}

void Scene::registerObjectToScene(Object* obj) {

  std::string instanceGroupKey = getInstanceGroupKey(obj);

  // Make this object's VAO the current context
  obj->bind();

  // Get the modelData (loc, rot, scale)
  std::vector<glm::mat4> modelData = getModelMatrices(obj);

  // Determine if an SSBO has been created for these instances
  auto& sameInstances = m_objects_map.find(instanceGroupKey);
  unsigned int offset = 0;
  unsigned int SSBO;

  if (sameInstances == m_objects_map.end()) {

    // setup an ssbo for this instanceGroup
    SSBO = createModelBuffer();
    std::unordered_map<Object*, unsigned int> modelBufferInfo;
    modelBufferInfo[obj] = 0;

    m_objects_map[instanceGroupKey] = std::make_pair(SSBO, modelBufferInfo);

  }
  else {
    // Determine the ssbo and offset for this object
    SSBO = sameInstances->second.first;
    offset = sameInstances->second.second.size() * m_numFloatsPerModelData;
    sameInstances->second.second[obj] = offset;
  }

  // Send the data to the vram
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO);
  glBufferSubData(GL_SHADER_STORAGE_BUFFER, sizeof(float) * offset, sizeof(glm::mat4) * modelData.size(), &modelData[0]);

  // Set Dynamic attributes for each instance
  glBindBuffer(GL_ARRAY_BUFFER, SSBO);
  glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(float) * m_numFloatsPerModelData, (void*)(sizeof(float) * (m_numFloatsPerModelData - 4 * 4)));
  glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(float) * m_numFloatsPerModelData, (void*)(sizeof(float) * (m_numFloatsPerModelData - 3 * 4)));
  glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(float) * m_numFloatsPerModelData, (void*)(sizeof(float) * (m_numFloatsPerModelData - 2 * 4)));
  glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(float) * m_numFloatsPerModelData, (void*)(sizeof(float) * (m_numFloatsPerModelData - 1 * 4)));
  glVertexAttribDivisor(4, 1);
  glVertexAttribDivisor(5, 1);
  glVertexAttribDivisor(6, 1);
  glVertexAttribDivisor(7, 1);

  glEnableVertexAttribArray(4);
  glEnableVertexAttribArray(5);
  glEnableVertexAttribArray(6);
  glEnableVertexAttribArray(7);

}

// update the scale, rotation and position of an object in the scene.
// Note that particles are calculated on the gpu, and their data will be reset by this operation.
void Scene::updateObjectInScene(Object* obj) {

  std::string instanceGroupKey = getInstanceGroupKey(obj);

  // Make the object's context current
  bindObjectWithModelMatrix(obj);

  std::vector<glm::mat4> modelData = getModelMatrices(obj);

  // Find the offset of the object within the SSBO
  auto const& groupedInstances = m_objects_map[instanceGroupKey];
  unsigned int SSBO = groupedInstances.first;
  unsigned int SSBO_OFFSET = sizeof(float) * groupedInstances.second.at(obj);

  // Upload the new data to vram
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO);
  glBufferSubData(GL_SHADER_STORAGE_BUFFER, SSBO_OFFSET, sizeof(glm::mat4) * modelData.size(), &modelData[0]);

}

void Scene::bindObjectWithModelMatrix(Object* obj) {

  obj->bind();

  // Set vertex attribute for model matrix instances
  unsigned int SSBO = m_objects_map[getInstanceGroupKey(obj)].first;
  glBindBuffer(GL_ARRAY_BUFFER, SSBO);
  glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(float) * m_numFloatsPerModelData, (void*)(sizeof(float) * (m_numFloatsPerModelData - 4 * 4)));
  glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(float) * m_numFloatsPerModelData, (void*)(sizeof(float) * (m_numFloatsPerModelData - 3 * 4)));
  glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(float) * m_numFloatsPerModelData, (void*)(sizeof(float) * (m_numFloatsPerModelData - 2 * 4)));
  glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(float) * m_numFloatsPerModelData, (void*)(sizeof(float) * (m_numFloatsPerModelData - 1 * 4)));


}

void Scene::render() {

  // Get shaderProgram
  ShaderManager* shaderManager = ShaderManager::getInstance();
  MeshManager* meshManager = MeshManager::getInstance();
  unsigned int shaderProgram = shaderManager->getBoundShader();

  // Get view projection for the entire draw call
  glm::mat4 view = m_camera.getViewTransform();
  glm::mat4 particleView = glm::mat4(1.0f);

  // Setup projection matrix for entire draw call
  Config* config = Config::getInstance();
  unsigned int SCR_WIDTH = config->getScreenWidth();
  unsigned int SCR_HEIGHT = config->getScreenHeight();
  glm::mat4 projection = glm::perspective(glm::radians(m_camera.getFov() / 2.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1e20f);

  // Setup light data
  // x,y,z,type(point/spotlight),r,g,b,strength
  std::vector<float> lightData;
  for (Light& light : m_lights) {
    glm::vec3 lightPos = light.getPosition() / m_universeScaleFactor;
    lightPos = view * glm::vec4(lightPos, 1.0);
    lightData.push_back(lightPos.x);
    lightData.push_back(lightPos.y);
    lightData.push_back(lightPos.z);
    lightData.push_back(0.0f);
    auto lightColor = light.getColor();
    lightData.insert(lightData.end(), lightColor.begin(), lightColor.end());
    lightData.push_back(light.getIntensity());
  }

  // Loop through the groups, then calculate their model matrices and render
  for (auto const& itr : m_objects_map) {

    auto const& instanceGroupKey = itr.first;
    auto const& groupedInstances = itr.second;
    unsigned int SSBO = groupedInstances.first;
    auto const& objs = groupedInstances.second;

    auto const& objsItr = objs.begin();
    Object* instance = objsItr->first;

    // Update vram if the object is not a particles
    if (!instance->isParticle()) {
      for (const auto& itr : objs) {
        updateObjectInScene(itr.first);
      }
    }

    // Bind and calculate the model matrix for all objects in this instanceGroup
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, SSBO);
    shaderManager->bindComputeShader("../assets/shaders/compute/calculateModel.comp");
    glDispatchCompute(objs.size(), 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    // Bind an instance's shader,mesh,mat
    bindObjectWithModelMatrix(instance);

    // Bind the uniform data for this instance
    unsigned int shaderProgram = shaderManager->getBoundShader();

    unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    unsigned int projectionLoc = glGetUniformLocation(shaderProgram, "projection");
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    unsigned int lightCountLoc = glGetUniformLocation(shaderProgram, "lightCount");
    glUniform1i(lightCountLoc, m_lights.size());

    unsigned int lightLoc = glGetUniformLocation(shaderProgram, "lights");
    glUniform1fv(lightLoc, lightData.size(), &(lightData[0]));
    
    // Render
    std::vector<unsigned int> bufferInfo = meshManager->getBufferInfo();
    const unsigned int numVertices = bufferInfo[2];
    glDrawArraysInstanced(GL_TRIANGLES, 0, numVertices, objs.size());

  }

}

void Scene::renderSkybox() {

  // Get view projection for the entire draw call
  glm::mat4 view = m_camera.getViewTransform();

  Config* config = Config::getInstance();
  unsigned int SCR_WIDTH = config->getScreenWidth();
  unsigned int SCR_HEIGHT = config->getScreenHeight();
  glm::mat4 projection = glm::perspective(glm::radians(m_camera.getFov() / 2.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1e20f);

  skybox.render(view, projection);

}

