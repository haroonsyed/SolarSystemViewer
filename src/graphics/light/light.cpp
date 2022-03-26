#include "light.h"

void Light::setPosition(glm::vec3 position) {
  this->position = position;
}

void Light::setPosition(float x, float y, float z) {
  glm::vec3 position = glm::vec3(x, y, z);
  this->position = position;
}

glm::vec3 Light::getPosition() {
  return position;
}

void Light::setColor(float red, float green, float blue) {
  color.insert(color.end(), { red, green, blue });
}

std::vector<float> Light::getColor() {
  return color;
}

void Light::setIntensity(float intensity) {
  intensity = intensity;
}

float Light::getIntensity() {
  return intensity;
}