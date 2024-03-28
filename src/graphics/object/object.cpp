#include "object.h"
#include <glm/gtc/quaternion.hpp>
#include "../mesh/meshManager.h"
#include "../shader/shaderManager.h"
#include "../texture/textureManager.h"
#include <iostream>
using namespace nlohmann;

Object::Object() {
  m_position = glm::vec3(0.0);
  m_velocity = glm::vec3(1.0);
  m_rotation = glm::angleAxis(0.0f, glm::vec3(0.0, 1.0, 0.0));
  m_scale = 70e3/1e3;
  m_mass = 1.0f;
  m_meshFilePath = "../assets/models/sphere.obj";
  m_vertexShaderPath = "../shaders/phong.vs";
  m_fragShaderPath = "../shaders/phong.fs";
  m_diffuseMapFilePath = "";
  m_normalMapFilePath = "";
  m_specularMapFilePath = "";
  m_emissiveMapFilePath = "";
  m_diffuseMapStrength = 1.0f;
  m_normalMapStrength = 1.0f;
  m_specularMapStrength = 1.0f;
  m_emissiveMapStrength = 1.0f;
  m_isParticle = false;
}

void Object::setParamsFromJSON(float SIUnitScaleFactor, json jsonData) {
  std::string name = jsonData["name"].get<std::string>();
  setName(name);
  setScale(jsonData["radius"].get<float>() / SIUnitScaleFactor);
  setPosition(
    jsonData["position"]["x"].get<float>() / SIUnitScaleFactor,
    jsonData["position"]["y"].get<float>() / SIUnitScaleFactor,
    jsonData["position"]["z"].get<float>() / SIUnitScaleFactor
  );
  setMesh(jsonData["meshFilePath"].get<std::string>());
  setShaders(
    jsonData["vertexShaderPath"].get<std::string>(),
    jsonData["fragmentShaderPath"].get<std::string>()
  );
  if (jsonData.contains("diffuseMap")) {
    setDiffuseMap(jsonData["diffuseMap"].get<std::string>());
  }
  if (jsonData.contains("normalMap")) {
    setNormalMap(jsonData["normalMap"].get<std::string>());
  }
  if (jsonData.contains("specularMap")) {
    setSpecularMap(jsonData["specularMap"].get<std::string>());
  }
  if (jsonData.contains("emissiveMap")) {
    setEmissiveMap(jsonData["emissiveMap"].get<std::string>());
  }
  if (jsonData.contains("diffuseMapStrength")) {
    setDiffuseMapStrength(jsonData["diffuseMapStrength"].get<float>());
  }
  if (jsonData.contains("normalMapStrength")) {
    setNormalMapStrength(jsonData["normalMapStrength"].get<float>());
  }
  if (jsonData.contains("specularMapStrength")) {
    setSpecularMapStrength(jsonData["specularMapStrength"].get<float>());
  }
  if (jsonData.contains("emissiveMapStrength")) {
    setEmissiveMapStrength(jsonData["emissiveMapStrength"].get<float>());
  }
  if (jsonData.contains("isParticle")) {
    setIsParticle(jsonData["isParticle"].get<bool>());
  }

  // Rotate model to begin with north pole facing upward
  const glm::vec3 defaultRotationAxis = glm::vec3(1.0f, 0.0f, 0.0f);
  rotate(glm::angleAxis(3.14159f / 2, defaultRotationAxis));
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

glm::vec3 Object::getVelocity() {
  return m_velocity;
}

void Object::setVelocity(float x, float y, float z) {
  m_velocity = glm::vec3(x, y, z);
}

void Object::setVelocity(glm::vec3 velocity) {
  m_velocity = velocity;
}

void Object::setPosition(float x, float y, float z) {
  m_position = glm::vec3(x, y, z);
}

void Object::setPosition(glm::vec3 position) {
  m_position = position;
}

void Object::setRotation(float angle, glm::vec3 axis) {
  m_rotation = glm::angleAxis(angle, glm::normalize(axis));
}

void Object::rotate(glm::quat rotation) {
  m_rotation = rotation * m_rotation;
}

glm::mat4 Object::getRotationMat() {
  return glm::mat4_cast(m_rotation);
}

float Object::getScale() {
  return m_scale;
}

void Object::setScale(float scale) {
  m_scale = scale;
}

float Object::getAxis()
{
	return m_axis;
}

void Object::setAxis(float axis)
{
	m_axis = axis;
}

float Object::getMass() {
  return m_mass;
}

void Object::setMass(float mass) {
  m_mass = mass;
}

void Object::setMesh(std::string meshFilePath) {
  m_meshFilePath = meshFilePath;
}

std::string Object::getMesh() {
  return m_meshFilePath;
}

std::pair<std::string, std::string> Object::getShaders() {
  return std::make_pair(m_vertexShaderPath, m_fragShaderPath);
}

void Object::setShaders(std::string vertexShaderPath, std::string fragShaderPath) {
  m_vertexShaderPath = vertexShaderPath;
  m_fragShaderPath = fragShaderPath;
}

void Object::setDiffuseMap(std::string diffuseMapFilePath) {
  m_diffuseMapFilePath = diffuseMapFilePath;
}

void Object::setNormalMap(std::string normalMapFilePath) {
  m_normalMapFilePath = normalMapFilePath;
}

void Object::setSpecularMap(std::string specularMapFilePath) {
  m_specularMapFilePath = specularMapFilePath;
}

void Object::setEmissiveMap(std::string emissiveMapFilePath) {
  m_emissiveMapFilePath = emissiveMapFilePath;
}

void Object::setDiffuseMapStrength(int strength) {
  m_diffuseMapStrength = strength;
}

void Object::setNormalMapStrength(int strength) {
  m_normalMapStrength = strength;
}

void Object::setSpecularMapStrength(int strength) {
  m_specularMapStrength = strength;
}

void Object::setEmissiveMapStrength(int strength) {
  m_emissiveMapStrength = strength;
}

std::vector<std::string> Object::getTextures() {
  return
  {
    m_diffuseMapFilePath,
    m_normalMapFilePath,
    m_specularMapFilePath,
    m_emissiveMapFilePath
  };
}

std::vector<float> Object::getTextureStrengths() {
  return {
    m_diffuseMapStrength,
    m_normalMapStrength,
    m_specularMapStrength,
    m_emissiveMapStrength
  };
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
  std::vector<std::string > textures = getTextures();
  std::vector<float> textureStrengths = getTextureStrengths();
  textureManager->bindTextures(textures, textureStrengths);

}

glm::mat4 Object::getModelMatrix() {
  glm::mat4 model(1.0f);
  return glm::translate(glm::mat4_cast(m_rotation) * glm::scale(model, glm::vec3(m_scale)), m_position);
}

bool Object::isParticle() {
  return m_isParticle;
}

void Object::setIsParticle(bool isParticle) {
  m_isParticle = isParticle;
}
