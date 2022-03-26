#include "scene.h"
#include <iostream>
#include <fstream>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "../graphics/shader/shaderManager.h"
#include "../graphics/mesh/meshManager.h"
#include "nlohmann/json.hpp"

System* Scene::getPhysicsSystem() {
  return &physicsSystem;
}

void Scene::loadScene(std::string sceneFilePath) {

  using namespace nlohmann; // Testing lib namespace

  // Load scene from json file
  std::ifstream file(sceneFilePath);
  std::string scene;
  std::getline(file, scene, '\0');
  json jScene = json::parse(scene);

  // Construct scene. In units of MegaMeter and GigaGram
  for (auto gravBodyJSON : jScene["GravBodies"]) {
    GravBody* body = new GravBody();

    body->setMass(gravBodyJSON["mass"].get<float>() / 1e6);
    body->setPosition(
      gravBodyJSON["position"]["x"].get<float>()/1e6, 
      gravBodyJSON["position"]["y"].get<float>()/1e6,
      gravBodyJSON["position"]["z"].get<float>()/1e6
    );
    body->setVelocity(
      gravBodyJSON["velocity"]["x"].get<float>()/1e6,
      gravBodyJSON["velocity"]["y"].get<float>()/1e6,
      gravBodyJSON["velocity"]["z"].get<float>()/1e6
    );
    body->setMesh(gravBodyJSON["meshFilePath"]);
    body->setShaders(
      gravBodyJSON["vertexShaderPath"],
      gravBodyJSON["fragmentShaderPath"]
    );

    physicsSystem.addBody(body);

  }

  for (auto lightJSON : jScene["Lights"]) {
    Light light;

    light.setPosition(
      lightJSON["position"]["x"].get<float>()/1e6,
      lightJSON["position"]["y"].get<float>()/1e6,
      lightJSON["position"]["z"].get<float>()/1e6
    );

    light.setColor(
      lightJSON["color"]["red"].get<float>(),
      lightJSON["color"]["green"].get<float>(),
      lightJSON["color"]["blue"].get<float>()
    );

    light.setIntensity(lightJSON["intensity"].get<float>());

    lights.push_back(light);

  }

}

void Scene::render(glm::mat4 view) {

  // Get shaderProgram
  ShaderManager* shaderManager = ShaderManager::getInstance();
  MeshManager* meshManager = MeshManager::getInstance();
  unsigned int shaderProgram = shaderManager->getBoundShader();

  // Setup light data
  std::vector<glm::vec3> lightPositions;
  for (Light &light : lights) {
    lightPositions.push_back(light.getPosition());
  }

  //unsigned int lightLocs = glGetUniformLocation(shaderProgram, "lightPositions");
  //glUniform3fv(lightLocs, lightPositions.size(), glm::value_ptr(&lightPositions[0]));
  glm::vec3 lightPos = glm::vec3(-1.0f, 0.0f, 0.0f);
  lightPos *= 10000;

  std::vector<Object*> objects;
  for (Object* bodyPtr : physicsSystem.getBodies()) {
    objects.push_back((Object*)bodyPtr);
  }

  for (Object* obj : objects) {

    // Setup transform matrix for this obj
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, (obj->getPosition() / 50.0f));
    model = glm::scale(model, glm::vec3(obj->getScale()));

    obj->bind();

    //Pass to gpu
    unsigned int shaderProgram = shaderManager->getBoundShader();
    unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    unsigned int lightLoc = glGetUniformLocation(shaderProgram, "lightPos");
    glUniform3fv(lightLoc, 1, glm::value_ptr(lightPos));

    std::vector<unsigned int> bufferInfo = meshManager->getBufferInfo();
    const unsigned int numVertices = bufferInfo[2];
    glDrawArrays(GL_TRIANGLES, 0, numVertices);

  }

}

