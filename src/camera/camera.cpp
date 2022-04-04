#include "camera.h"
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include "../config.h"

void Camera::setCameraPosition(glm::vec3 position) {
  m_cameraPos = position;
}

void Camera::update(std::unordered_set<unsigned int>* pressedKeys) {

  // Zoom
  if(pressedKeys->count(GLFW_KEY_Z)) {
    m_cameraPos = m_cameraPos*glm::vec3(0.0,0.0,0.99);
  }
  else if(pressedKeys->count(GLFW_KEY_X)) {
    m_cameraPos = m_cameraPos*glm::vec3(0.0,0.0,1.01);
  }

  // Mouse

}

glm::mat4 Camera::getViewTransform() {
  return glm::lookAt(m_cameraPos, m_cameraTarget, m_up);
}
