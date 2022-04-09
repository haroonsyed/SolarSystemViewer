#pragma once
#include <GLFW/glfw3.h>
#include <unordered_set>

class InputController {
	private:
		std::unordered_set<unsigned int> m_pressedKeys;
		double deltaX, deltaY = 0;
		GLFWwindow* m_window = nullptr;
		double m_timeAtLastInput = glfwGetTime(); //Used to separate input speed from framerate
		bool guiButtonPressed = false;
	public:
		InputController(GLFWwindow* window);
		void processInput();
		std::unordered_set<unsigned int>* getPressedKeys();
		float getMouseDeltaX();
		float getMouseDeltaY();
};

