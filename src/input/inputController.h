#pragma once
#include <GLFW/glfw3.h>
#include <unordered_set>

class InputController {
	private:
		std::unordered_set<unsigned int> m_pressedKeys;
		float lastX = 800 / 2;
		float lastY = 600 / 2;
		float xOffset = 0;
		float yOffset = 0;
		bool firstMouse = true;
		GLFWwindow* m_window = nullptr;
		double m_timeAtLastInput = glfwGetTime(); //Used to separate input speed from framerate
		bool guiButtonPressed = false;
	public:
		InputController(GLFWwindow* window);
		void processInput();
		void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
		std::unordered_set<unsigned int>* getPressedKeys();
		float getXOffset();
		float getYOffset();
};

