#include "scene.h"
#include <iostream>
#include <fstream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>
#include "nlohmann/json.hpp"
#include "../graphics/shader/shaderManager.h"
#include "../graphics/mesh/meshManager.h"
#include "../config.h"

System* Scene::getPhysicsSystem() {
  return &m_physicsSystem;
}

void Scene::loadScene(std::string sceneFilePath) {

  using namespace nlohmann; // Testing lib namespace

  // Load scene from json file
  std::ifstream file(sceneFilePath);
  std::string scene;
  std::getline(file, scene, '\0');
  json jScene = json::parse(scene);

  float physicsDistanceFactor = jScene["PhysicsDistanceFactor"].get<float>();
  float physicsMassFactor = jScene["PhysicsMassFactor"].get<float>();
  m_universeScaleFactor = jScene["UniverseScaleFactor"].get<float>();
  m_physicsSystem.setPhysicsDistanceFactor(physicsDistanceFactor);
  m_physicsSystem.setPhysicsMassFactor(physicsMassFactor);

  // Construct scene. In units of MegaMeter and GigaGram
  for (auto gravBodyJSON : jScene["GravBodies"]) {
    GravBody* body = new GravBody();

    body->setName(gravBodyJSON["name"].get<std::string>());
    body->setScale(gravBodyJSON["radius"].get<float>() / physicsDistanceFactor);
    body->setMass(gravBodyJSON["mass"].get<float>() / physicsMassFactor);
    body->setPosition(
      gravBodyJSON["position"]["x"].get<float>()/physicsDistanceFactor, 
      gravBodyJSON["position"]["y"].get<float>()/physicsDistanceFactor,
      gravBodyJSON["position"]["z"].get<float>()/physicsDistanceFactor
    );
    body->setVelocity(
      gravBodyJSON["velocity"]["x"].get<float>()/physicsDistanceFactor,
      gravBodyJSON["velocity"]["y"].get<float>()/physicsDistanceFactor,
      gravBodyJSON["velocity"]["z"].get<float>()/physicsDistanceFactor
    );
    body->setMesh(gravBodyJSON["meshFilePath"].get<std::string>());
    body->setShaders(
      gravBodyJSON["vertexShaderPath"].get<std::string>(),
      gravBodyJSON["fragmentShaderPath"].get<std::string>()
    );
    body->setImageTexture(gravBodyJSON["textureFilePath"].get<std::string>());

    m_physicsSystem.addBody(body);

  }

  for (auto lightJSON : jScene["Lights"]) {
    Light light;

    light.setPosition(
      lightJSON["position"]["x"].get<float>()/physicsDistanceFactor,
      lightJSON["position"]["y"].get<float>()/physicsDistanceFactor,
      lightJSON["position"]["z"].get<float>()/physicsDistanceFactor
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

void Scene::render(glm::mat4& view) {

  // Get shaderProgram
  ShaderManager* shaderManager = ShaderManager::getInstance();
  MeshManager* meshManager = MeshManager::getInstance();
  unsigned int shaderProgram = shaderManager->getBoundShader();

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
  glm::vec3 lightPos = glm::vec3(-1.0f, 0.0f, 0.0f);
  lightPos *= 10000;

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

    glm::mat4 model = translation * rotation * scale;

    obj->bind();

    //Pass to gpu
    unsigned int shaderProgram = shaderManager->getBoundShader();
    unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    unsigned int projectionLoc = glGetUniformLocation(shaderProgram, "projection");
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    unsigned int lightLoc = glGetUniformLocation(shaderProgram, "lightPos");
    glUniform3fv(lightLoc, 1, glm::value_ptr(lightPos));

    std::vector<unsigned int> bufferInfo = meshManager->getBufferInfo();
    const unsigned int numVertices = bufferInfo[2];
    glDrawArrays(GL_TRIANGLES, 0, numVertices);

  }

}

