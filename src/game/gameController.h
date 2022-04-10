#pragma once
#include <GLFW/glfw3.h>
#include <unordered_set>
#include "../scene/scene.h"

// Registers all key presses and updates camera accordingly
class GameController {
private:
	static GameController* m_instance;
	
	static double m_mouseX;
	static double m_mouseY;
	static double m_deltaX;
	static double m_deltaY;
	static std::unordered_set<unsigned int> m_heldKeys;
	static bool m_showGui;

	static GLFWwindow* m_window;
	static Scene* m_boundScene;

	static int m_focusedBody;


	GameController(GLFWwindow* window, Scene* scene);

	static void processKeyInput(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void processMousePress(GLFWwindow* window, int button, int action, int mods);
	static void processMousePos(GLFWwindow* window, double xpos, double ypos);

	static void updateCamera(float deltaT);
	static void updateFocusedPlanet();
	static void toggleGUI();

public:

	static GameController* getInstance(GLFWwindow* window, Scene* scene);
	void update(float deltaT);
	void render();

};

