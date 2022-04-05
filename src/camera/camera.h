#pragma once
#include <unordered_set>
#include <glm/glm.hpp>

class Camera {
	private:
    glm::vec3 m_up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 m_cameraPos = glm::vec3(0.0f, 0.0f, 1e3f);
    glm::vec3 m_cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
	public:
		glm::vec3 getCameraPosition();
		void setCameraPosition(glm::vec3 position);
		void update(std::unordered_set<unsigned int>* pressedKeys);
		glm::mat4 getViewTransform();
};

