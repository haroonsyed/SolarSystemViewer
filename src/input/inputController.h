#pragma once
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class InputController {
	private:
		char pressedKey = 0;
		GLFWwindow* window = nullptr;
		glm::mat4 viewTransform = glm::mat4(1.0f);
		double timeAtLastInput = glfwGetTime(); //Used to separate input speed from framerate
	public:
		bool mousePressed;
		InputController(GLFWwindow* window);
		void processInput();
		char getPressedKey();
		glm::mat4 getViewTransform();
};

