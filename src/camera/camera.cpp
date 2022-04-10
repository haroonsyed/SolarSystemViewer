#include "camera.h"
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include "../config.h"
#include <iostream>
#include <math.h>

glm::vec3 Camera::getCameraPosition() {
  return m_cameraPos;
}

void Camera::setCameraPosition(glm::vec3 position) {
  m_cameraPos = position;
}

void Camera::update(float deltaT, InputController& input) {

  Config* config = Config::getInstance();

  std::unordered_set<unsigned int>* pressedKeys = input.getPressedKeys();
  float deltaDistance = deltaT * glm::distance(m_cameraPos, m_cameraTarget);
  glm::vec3 viewDir = glm::normalize(m_cameraTarget - m_cameraPos);

  // Use mouse + LF to move camera. Use Scroll / X,Z to zoom
  if (pressedKeys->count(GLFW_MOUSE_BUTTON_LEFT)) {

    double sensitivity = config->getMouseSensitivity();
    float adjustedDeltaX = sensitivity * deltaDistance * input.getMouseDeltaX();
    float adjustedDeltaY = sensitivity * deltaDistance * input.getMouseDeltaY();
    glm::vec3 left = glm::vec3(-1.0, 0.0, 0.0);

    m_cameraPos += adjustedDeltaY * m_up;
    m_cameraTarget += adjustedDeltaY * m_up;
    m_cameraPos += adjustedDeltaX * left;
    m_cameraTarget += adjustedDeltaX * left;

  }

  if (pressedKeys->count(GLFW_KEY_Z)) {
    m_cameraPos += deltaDistance * viewDir;
  }
  else if (pressedKeys->count(GLFW_KEY_X)) {
    m_cameraPos -= deltaDistance * viewDir;
  }

}

glm::mat4 Camera::getViewTransform() {
  return glm::lookAt(m_cameraPos, m_cameraTarget, m_up);
}
