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

glm::vec3 Camera::getCameraTarget() {
  return m_cameraTarget;
}

void Camera::setCameraTarget(glm::vec3 target) {
  m_cameraTarget = target;
}

glm::mat4 Camera::getViewTransform() {
  return glm::lookAt(m_cameraPos, m_cameraTarget, m_up);
}

float Camera::getFov() {
  return m_fov;
}

void Camera::setFov(float fov) {
  m_fov = fov;
}

glm::vec3 Camera::getUp() {
  return m_up;
}
