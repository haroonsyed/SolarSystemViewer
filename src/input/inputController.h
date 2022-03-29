#pragma once
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class InputController {
	private:
		char m_pressedKey = 0;
		GLFWwindow* m_window = nullptr;
		glm::mat4 m_viewTransform = glm::mat4(1.0f);
		double m_timeAtLastInput = glfwGetTime(); //Used to separate input speed from framerate
		bool m_mousePressed;
	public:
		InputController(GLFWwindow* window);
		void processInput();
		char getPressedKey();
		glm::mat4 getViewTransform();
};

