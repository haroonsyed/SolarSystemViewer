#pragma once
#include <glm/glm.hpp>
#include "../input/inputController.h"
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

class Camera {
	private:
    glm::vec3 m_up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 m_cameraPos = glm::vec3(0.0f, 0.0f, 1e3f);
    glm::vec3 m_cameraTarget = glm::vec3(0.0f, 0.0f, -1.0);
		glm::quat m_cameraRot = glm::angleAxis(0.0f, glm::vec3(0.0, 1.0, 0.0));;
		
	public:
		glm::vec3 getCameraPosition();
		void setCameraPosition(glm::vec3 position);
		void update(float deltaT, InputController& input);
		glm::mat4 getViewTransform();
};

