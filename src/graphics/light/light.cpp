#include "light.h"

void Light::setPosition(glm::vec3 position) {
  m_position = position;
}

void Light::setPosition(float x, float y, float z) {
  glm::vec3 position = glm::vec3(x, y, z);
  m_position = position;
}

glm::vec3 Light::getPosition() {
  return m_position;
}

void Light::setColor(float red, float green, float blue) {
  m_color.insert(m_color.end(), { red, green, blue });
}

std::vector<float> Light::getColor() {
  return m_color;
}

void Light::setIntensity(float intensity) {
  m_intensity = intensity;
}

float Light::getIntensity() {
  return m_intensity;
}