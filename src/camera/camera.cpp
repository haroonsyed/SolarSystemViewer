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

  std::unordered_set<unsigned int>* pressedKeys = input.getPressedKeys();
  glm::vec3 viewDir = m_cameraTarget - m_cameraPos;

  // Buttons
  if (pressedKeys->count(GLFW_KEY_W))
  {
    m_cameraPos += deltaT * viewDir;
  }
  if (pressedKeys->count(GLFW_KEY_A))
  {
    m_cameraPos -= deltaT * glm::cross(viewDir, m_up);
    m_cameraTarget -= deltaT * glm::cross(viewDir, m_up);
  }
  if (pressedKeys->count(GLFW_KEY_S))
  {
    m_cameraPos -= deltaT * viewDir;
  }
  if (pressedKeys->count(GLFW_KEY_D))
  {
    m_cameraPos += deltaT * glm::cross(viewDir, m_up);
    m_cameraTarget += deltaT * glm::cross(viewDir, m_up);
  }
  if (pressedKeys->count(GLFW_MOUSE_BUTTON_MIDDLE))
  {
    float adjustedDeltaX = deltaT * input.getMouseDeltaX();
    float adjustedDeltaY = deltaT * input.getMouseDeltaY();
    float defaultRotationAmountRadians = 1.0;
    m_cameraRot = glm::angleAxis(deltaT, glm::vec3(adjustedDeltaX, adjustedDeltaY, 0.0)) * m_cameraRot;
  }

}

glm::mat4 Camera::getViewTransform() {
  return glm::toMat4(m_cameraRot) * glm::lookAt(m_cameraPos, m_cameraTarget, m_up);
}
