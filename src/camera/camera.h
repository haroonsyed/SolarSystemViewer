#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

class Camera {
  private:
    glm::vec3 m_up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 m_cameraPos = glm::vec3(0.0f, 0.0f, 1e3f);
		glm::vec3 m_cameraTarget = glm::vec3(0.0f, 0.0f, -1.0);
		glm::quat m_cameraRot = glm::angleAxis(0.0f, glm::vec3(0.0, 1.0, 0.0));
		float m_fov = 90;
		
  public:
		glm::vec3 getCameraPosition();
		void setCameraPosition(glm::vec3 position);
		glm::vec3 getCameraTarget();
		void setCameraTarget(glm::vec3 target);
		glm::mat4 getViewTransform();
		float getFov();
		void setFov(float fov);
		glm::vec3 getUp();
		void setUp(glm::vec3 up);
};

