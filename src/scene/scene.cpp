#include "scene.h"
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "../graphics/shader/shaderManager.h"
#include "../graphics/mesh/meshManager.h"

System* Scene::getSystem() {
  return &physicsSystem;
}

void Scene::render(glm::mat4 view) {

  // Get shaderProgram
  ShaderManager* shaderManager = ShaderManager::getInstance();
  MeshManager* meshManager = MeshManager::getInstance();
  unsigned int shaderProgram = shaderManager->getBoundShader();

  // Setup light data
  std::vector<glm::vec3> lightPositions;
  for (Light light : lights) {
    lightPositions.push_back(light.getPosition());
  }

  //unsigned int lightLocs = glGetUniformLocation(shaderProgram, "lightPositions");
  //glUniform3fv(lightLocs, lightPositions.size(), glm::value_ptr(&lightPositions[0]));
  glm::vec3 lightPos = glm::vec3(-1.0f, 0.0f, 0.0f);
  lightPos *= 10000;

  for (GravBody body : physicsSystem.getBodies()) {

    // Setup transform matrix for this body
    glm::mat4 model = glm::mat4(1.0f);
    //model = glm::translate(model, (float)glfwGetTime() * glm::vec3(0.1));
    model = glm::translate(model, (body.getPosition() / 50.0f));
    model = glm::scale(model, glm::vec3(0.05));

    body.getObject()->bind();

    //Pass to gpu
    unsigned int shaderProgram = shaderManager->getBoundShader();
    unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    unsigned int lightLoc = glGetUniformLocation(shaderProgram, "lightPos");
    glUniform3fv(lightLoc, 1, glm::value_ptr(lightPos));

    std::vector<unsigned int> bufferInfo = meshManager->getBufferInfo();
    const unsigned int VAO = bufferInfo[0];
    const unsigned int VBO = bufferInfo[1];
    const unsigned int numVertices = bufferInfo[2];
    glUseProgram(shaderProgram);
    glBindVertexArray(VAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
    glDrawArrays(GL_TRIANGLES, 0, numVertices);


  }

}

