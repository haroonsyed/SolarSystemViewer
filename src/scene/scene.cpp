#include <GL/glew.h>
#include "scene.h"
#include <iostream>
#include <fstream>
#include <glm/gtc/type_ptr.hpp>
#include "nlohmann/json.hpp"
#include "../graphics/shader/shaderManager.h"
#include "../graphics/mesh/meshManager.h"
#include "../config.h"

Scene::Scene(GLFWwindow* window) : m_inputController(window) {
  m_universeScaleFactor = 1.0f;
}

System* Scene::getPhysicsSystem() {
  return &m_physicsSystem;
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
    m_physicsSystem.addBody(body);
  }

  // Construct lights
  for (auto lightJSON : jScene["Lights"]) {
    Light light;

    light.setPosition(
      lightJSON["position"]["x"].get<float>()/physicsDistanceFactor / m_universeScaleFactor,
      lightJSON["position"]["y"].get<float>()/physicsDistanceFactor / m_universeScaleFactor,
      lightJSON["position"]["z"].get<float>()/physicsDistanceFactor / m_universeScaleFactor
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

void Scene::update(float deltaT) {

  // Input
  m_inputController.processInput();

  // Simulation
  m_physicsSystem.update(deltaT);

  // Camera
  m_camera.update(deltaT, m_inputController);

}

void Scene::render() {

  // Get shaderProgram
  ShaderManager* shaderManager = ShaderManager::getInstance();
  MeshManager* meshManager = MeshManager::getInstance();
  unsigned int shaderProgram = shaderManager->getBoundShader();

  // Get view projection for the entire draw call
  glm::mat4 view = m_camera.getViewTransform() * m_universeScaleFactor;

  // Setup projection matrix for entire draw call
  Config* config = Config::getInstance();
  unsigned int SCR_WIDTH = config->getScreenWidth();
  unsigned int SCR_HEIGHT = config->getScreenHeight();
  glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1e20f);

  // Setup light data
  std::vector<glm::vec3> lightPositions;
  for (Light &light : m_lights) {
    lightPositions.push_back(light.getPosition());
  }

  //unsigned int lightLocs = glGetUniformLocation(shaderProgram, "lightPositions");
  //glUniform3fv(lightLocs, lightPositions.size(), glm::value_ptr(&lightPositions[0]));
  glm::vec3 lightPos = glm::normalize(glm::vec3(1.0f, 0.0f, 0.0f));
  lightPos *= 1e6 / m_universeScaleFactor;

  std::vector<Object*> objects;
  for (Object* bodyPtr : m_physicsSystem.getBodies()) {
    objects.push_back((Object*)bodyPtr);
  }

  for (Object* obj : objects) {

    // Setup model matrix for this obj
    glm::mat4 scale = glm::mat4(1.0);
    scale = glm::scale(scale, glm::vec3(obj->getScale()));
    glm::mat4 rotation = obj -> getRotationMat();
    glm::mat4 translation = glm::mat4(1.0f);
    translation = glm::translate(translation, obj->getPosition()/m_universeScaleFactor);

    glm::mat4 modelView = view * translation * rotation * scale;

    obj->bind();

    //Pass to gpu
    unsigned int shaderProgram = shaderManager->getBoundShader();
    unsigned int modelLoc = glGetUniformLocation(shaderProgram, "modelView");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelView));
    unsigned int projectionLoc = glGetUniformLocation(shaderProgram, "projection");
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    unsigned int lightLoc = glGetUniformLocation(shaderProgram, "lightPos");
    glUniform3fv(lightLoc, 1, glm::value_ptr(lightPos));

    std::vector<unsigned int> bufferInfo = meshManager->getBufferInfo();
    const unsigned int numVertices = bufferInfo[2];
    glDrawArrays(GL_TRIANGLES, 0, numVertices);

  }

}

