#include "camera.h"
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include "../config.h"
#include <iostream>

void Camera::update(std::unordered_set<unsigned int>* pressedKeys, float xOffset, float yOffset, glm::vec3 position) {

  // Buttons
  if (pressedKeys->count(GLFW_KEY_W))
  {
      m_cameraPos += cameraSpeed * m_cameraTarget;
  }
  else if (pressedKeys->count(GLFW_KEY_A))
  {
      m_cameraPos -= cameraSpeed * glm::normalize(glm::cross(m_cameraTarget,m_up));
  }
  else if (pressedKeys->count(GLFW_KEY_S))
  {
      m_cameraPos -= cameraSpeed * m_cameraTarget;
  }
  else if (pressedKeys->count(GLFW_KEY_D))
  {
      m_cameraPos += cameraSpeed * glm::normalize(glm::cross(m_cameraTarget, m_up));
  }
  else if (pressedKeys->count(GLFW_MOUSE_BUTTON_MIDDLE))
  {
      if (rotateToggle) 
      {
          rotateToggle = false;
      }
      else
      {
          rotateToggle = true;
      }
      
  }


  // Mouse

  if (rotateToggle)
  {
     yaw += xOffset;
     pitch += yOffset;

     // make sure that when pitch is out of bounds, screen doesn't get flipped
     if (pitch > 89.0f)
         pitch = 89.0f;
     if (pitch < -89.0f)
         pitch = -89.0f;

     glm::vec3 front;
     front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
     front.y = sin(glm::radians(pitch));
     front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
     m_cameraTarget = glm::normalize(front);
  }

  /*m_cameraPos.x = 1000;
  m_cameraPos.y = position.y/1000;
  m_cameraPos.z = position.z;*/
  //m_cameraTarget = position;
  //planet inspector


}

glm::mat4 Camera::getViewTransform() {
  return glm::lookAt(m_cameraPos, m_cameraPos + m_cameraTarget, m_up);
}
