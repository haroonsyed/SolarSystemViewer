#include "object.h"
#include "../mesh/meshManager.h"
#include "../shader/shaderManager.h"

Object::Object() {
  position = glm::vec3(0.0);
  scale = 0.05;
  meshFilePath = "../assets/models/sphere.obj";
  vertexShaderPath = "../shaders/phong.vs";
  fragShaderPath = "../shaders/phong.fs";
}

glm::vec3 Object::getPosition() {
  return position;
}

void Object::setPosition(float x, float y, float z) {
  this->position = glm::vec3(x, y, z);
}

void Object::setPosition(glm::vec3 position) {
  this->position = position;
}

float Object::getScale() {
  return scale;
}

void Object::setScale(float scale) {
  this->scale = scale;
}

void Object::setMesh(std::string meshFilePath) {
  this->meshFilePath = meshFilePath;
}

void Object::setShaders(std::string vertexShaderPath, std::string fragShaderPath) {
  this->vertexShaderPath = vertexShaderPath;
  this->fragShaderPath = fragShaderPath;
}

void Object::bind() {

  // First bind the shader
  // Implement a cache system to speed this up
  ShaderManager* shaderManager = ShaderManager::getInstance();
  shaderManager->bindShader(vertexShaderPath, fragShaderPath);
  
  // Now bind geometry to buffer
  MeshManager* meshManager = MeshManager::getInstance();
  meshManager->bindMesh(meshFilePath);


}