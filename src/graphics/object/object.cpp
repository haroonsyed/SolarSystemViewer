#include "object.h"
#include "../mesh/meshManager.h"
#include "../shader/shaderManager.h"
#include "../texture/textureManager.h"

Object::Object() {
  m_position = glm::vec3(0.0);
  m_scale = 70e3/1e3;
  m_meshFilePath = "../assets/models/sphere.obj";
  m_vertexShaderPath = "../shaders/phong.vs";
  m_fragShaderPath = "../shaders/phong.fs";
}

void Object::setName(std::string name) {
  m_name = name;
}

std::string Object::getName() {
  return m_name;
}

glm::vec3 Object::getPosition() {
  return m_position;
}

void Object::setPosition(float x, float y, float z) {
  m_position = glm::vec3(x, y, z);
}

void Object::setPosition(glm::vec3 position) {
  m_position = position;
}

float Object::getScale() {
  return m_scale;
}

void Object::setScale(float scale) {
  m_scale = scale;
}

void Object::setMesh(std::string meshFilePath) {
  m_meshFilePath = meshFilePath;
}

void Object::setShaders(std::string vertexShaderPath, std::string fragShaderPath) {
  m_vertexShaderPath = vertexShaderPath;
  m_fragShaderPath = fragShaderPath;
}

void Object::setImageTexture(std::string imgTexFilePath) {
  m_imgTexFilePath = imgTexFilePath;
}

void Object::bind() {

  // First bind the shader
  ShaderManager* shaderManager = ShaderManager::getInstance();
  shaderManager->bindShader(m_vertexShaderPath, m_fragShaderPath);
  
  // Now bind geometry to buffer
  MeshManager* meshManager = MeshManager::getInstance();
  meshManager->bindMesh(m_meshFilePath);

  // Now bind textures
  TextureManager* textureManager = TextureManager::getInstance();
  textureManager->bindTexture(m_imgTexFilePath);

}