#pragma once
#include <GLFW/glfw3.h>
#include <unordered_set>

class InputController {
  private:
	static std::unordered_set<unsigned int> m_pressedKeys;
	static double m_mouseX;
	static double m_mouseY;
	static double m_deltaX;
	static double m_deltaY;
	static bool m_showGui;
	static GLFWwindow* m_window;
	static InputController* m_instance;
		
	InputController(GLFWwindow* window);
    static void processKeyInput(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void processMousePress(GLFWwindow* window, int button, int action, int mods);
	static void processMousePos(GLFWwindow* window, double xpos, double ypos);

  public:
	
	static InputController* getInstance(GLFWwindow* window);
	std::unordered_set<unsigned int>* getPressedKeys();
	double getMouseX();
	double getMouseY();
	double getMouseDeltaX();
	double getMouseDeltaY();
};

