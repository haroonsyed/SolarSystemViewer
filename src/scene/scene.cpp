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
  genUniformBuffer();
  glGenVertexArrays(1, &m_particleVAO);
}

unsigned int Scene::getUBOSize() {
  // view, projection, 20 lights, ambient, specular, phong, kc, kl, kq ( 6 constants for lighting);
  // Each light takes 2 vec4s
  const unsigned int numOfLightingConstants = 6;
  return sizeof(glm::mat4) * 2 + sizeof(glm::vec4) * (m_MAX_NUM_LIGHTS * 2 + numOfLightingConstants);
}

void Scene::genUniformBuffer() {
  unsigned int sizeOfUBO = getUBOSize();
  glGenBuffers(1, &m_uniformBuffer);
  glBindBuffer(GL_UNIFORM_BUFFER, m_uniformBuffer);
  glBufferData(GL_UNIFORM_BUFFER, sizeOfUBO, nullptr, GL_DYNAMIC_DRAW);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);
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
  m_universeScaleFactor = jScene["UniverseScaleFactor"].get<double>();
  float SIUnitScaleFactor = jScene["SIUnitScaleFactor"].get<double>();

  // Setup camera
  m_camera.setCameraPosition(
    glm::vec3(
      jScene["CameraPosition"]["x"].get<double>() / SIUnitScaleFactor,
      jScene["CameraPosition"]["y"].get<double>() / SIUnitScaleFactor,
      jScene["CameraPosition"]["z"].get<double>() / SIUnitScaleFactor
    )
  );

  // Setup lighting data in uniform
  m_ambientStrength = jScene["ambientStrength"].get<float>();
  m_specularStrength = jScene["specularStrength"].get<float>();
  m_phongExponent = jScene["phongExponent"].get<float>();

  // Setup physics
  m_physicsSystem.setSIUnitScaleFactor(SIUnitScaleFactor);
  std::vector<Body> gpuBodies;
  
  // Construct scene. In units specified in SI units of json
  for (auto gravBodyJSON : jScene["GravBodies"]) {
           
    // Register body as cpu-synced object
    if (gravBodyJSON["isParticle"].get<bool>() == false) {
      GravBody* body = new GravBody(SIUnitScaleFactor, gravBodyJSON);
      m_physicsSystem.addBody(body); // Add gravBody to physics system

      // Tell scene to register this object
      registerObjectToScene(body);
    }

    // Register body as gpu-accelerated-particle
    else {
      glm::vec4 position = glm::vec4(0.0);
      glm::vec4 velocity = glm::vec4(0.0);
      GLfloat mass = 0.0;
      position.x = gravBodyJSON["position"]["x"].get<double>() / SIUnitScaleFactor;
      position.y = gravBodyJSON["position"]["y"].get<double>() / SIUnitScaleFactor;
      position.z = 0.0;
      position.w = 0.0;

      velocity.x = gravBodyJSON["velocity"]["x"].get<double>() / SIUnitScaleFactor;
      velocity.y = gravBodyJSON["velocity"]["y"].get<double>() / SIUnitScaleFactor;
      velocity.z = 0.0;
      velocity.w = 0.0;

      mass = gravBodyJSON["mass"].get<double>() / SIUnitScaleFactor;
      Body body{position, velocity, mass};
      gpuBodies.push_back(body);
    }

  }

  // Add the particles to the physics system
  m_physicsSystem.setBodiesGPU(gpuBodies, 12);

  // Construct lights
  for (auto lightJSON : jScene["Lights"]) {
    Light light;

    light.setPosition(
      lightJSON["position"]["x"].get<float>() / SIUnitScaleFactor,
      lightJSON["position"]["y"].get<float>() / SIUnitScaleFactor,
      lightJSON["position"]["z"].get<float>() / SIUnitScaleFactor
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
  auto const& sameInstances = m_objects_map.find(instanceGroupKey);
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

  // Get view projection for the entire draw call
  glm::mat4 view = m_camera.getViewTransform();

  // Setup projection matrix for entire draw call
  Config* config = Config::getInstance();
  unsigned int SCR_WIDTH = config->getScreenWidth();
  unsigned int SCR_HEIGHT = config->getScreenHeight();
  glm::mat4 projection = glm::perspective(glm::radians(m_camera.getFov()), (float)SCR_WIDTH / (float)SCR_HEIGHT, 1e-1f, 1e10f);

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

  // Set the uniform data in the UBO
  glBindBuffer(GL_UNIFORM_BUFFER, m_uniformBuffer);
  glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_uniformBuffer);
  glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), &view);
  glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), &projection);
  glBufferSubData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4) + sizeof(float) * 0, sizeof(float), &m_ambientStrength);
  glBufferSubData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4) + sizeof(float) * 1, sizeof(float), &m_specularStrength);
  glBufferSubData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4) + sizeof(float) * 2, sizeof(float), &m_phongExponent);
  unsigned int numOfLights = m_lights.size();
  glBufferSubData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4) + sizeof(float) * 3, sizeof(unsigned int), &numOfLights);
  glBufferSubData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4) + sizeof(float) * 4, sizeof(float) * lightData.size(), &(lightData[0]));

  // Loop through the groups, then calculate their model matrices and render
  for (auto const& itr : m_objects_map) {

    auto const& instanceGroupKey = itr.first;
    auto const& groupedInstances = itr.second;
    unsigned int SSBO = groupedInstances.first;
    auto const& objs = groupedInstances.second;

    auto const& objsItr = objs.begin();
    Object* instance = objsItr->first;

    // Update vram if the object is not a particles
    for (const auto& itr : objs) {
      updateObjectInScene(itr.first);
    }

    // Bind and calculate the model matrix for all objects in this instanceGroup
    unsigned int workerGroupSize = 1;
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, SSBO);
    shaderManager->bindComputeShader("../assets/shaders/compute/model/calculateModel.comp");
    glDispatchCompute(objs.size(), 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    // Bind an instance's shader,mesh,mat
    bindObjectWithModelMatrix(instance);
    
    // Render
    std::vector<unsigned int> bufferInfo = meshManager->getBufferInfo();
    const unsigned int numVertices = bufferInfo[2];
    glDrawArraysInstanced(GL_TRIANGLES, 0, numVertices, objs.size());

  }


  // Render particles (physics has already been calculated in system.cpp)
  // Assumed SSBO_BODIES is bound on 4
  double startTime = glfwGetTime();
  const unsigned int numberOfParticles = m_physicsSystem.getNumberOfGPUBodies();
  const unsigned int bodiesSSBO = m_physicsSystem.getBodiesSSBO();
  shaderManager->bindShader("../assets/shaders/particle.vs", "../assets/shaders/particle.fs");
  glBindVertexArray(m_particleVAO);
  glBindBuffer(GL_ARRAY_BUFFER, bodiesSSBO);
  glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Body), (void*)0); // Position Data
  glVertexAttribDivisor(0, 1);
  glEnableVertexAttribArray(0);
  glDisable(GL_DEPTH_TEST);
  glDrawArraysInstanced(GL_POINTS, 0, 2, numberOfParticles);
  glEnable(GL_DEPTH_TEST);
  glBindVertexArray(0);
  glFinish();
  std::cout << "Time to render: " << glfwGetTime() - startTime << std::endl;

}

void Scene::renderSkybox() {

  Config* config = Config::getInstance();

  if (!config->getSkyBoxEnabled()) {
      return;
  }

  // Get view projection for the entire draw call
  glm::mat4 view = m_camera.getViewTransform();

  unsigned int SCR_WIDTH = config->getScreenWidth();
  unsigned int SCR_HEIGHT = config->getScreenHeight();
  glm::mat4 projection = glm::perspective(glm::radians(m_camera.getFov() / 2.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1e20f);

  skybox.render(view, projection);

}

