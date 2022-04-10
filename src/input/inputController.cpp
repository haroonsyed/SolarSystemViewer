#include "inputController.h"
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "../config.h"

std::unordered_set<unsigned int> InputController::m_pressedKeys;
double InputController::m_mouseX = 0;
double InputController::m_mouseY = 0;
double InputController::m_deltaX = 0;
double InputController::m_deltaY = 0;
bool InputController::m_showGui = false;
GLFWwindow* InputController::m_window = nullptr;
InputController* InputController::m_instance = nullptr;


InputController* InputController::getInstance(GLFWwindow* window) {
  if (m_instance == nullptr) {
    m_instance = new InputController(window);
  }
  return m_instance;
}

InputController::InputController(GLFWwindow* window) {
  m_window = window;
  glfwSetKeyCallback(m_window, processKeyInput);
  glfwSetMouseButtonCallback(m_window, processMousePress);
  glfwSetCursorPosCallback(m_window, processMousePos);
  glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void InputController::processKeyInput(GLFWwindow* window, int key, int scancode, int action, int mods)
{

  // Get config data
  Config* config = Config::getInstance();
  unsigned int SCR_WIDTH = config->getScreenWidth();
  unsigned int SCR_HEIGHT = config->getScreenHeight();

  // CLOSE APPLICATION
  if (key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE) {
    glfwSetWindowShouldClose(window, true);
  }

  // Get the relevant keys that are pressed
  if (key == GLFW_KEY_W && action == GLFW_PRESS) {
    m_pressedKeys.insert(GLFW_KEY_W);
  }
  if (key == GLFW_KEY_A && action == GLFW_PRESS) {
    m_pressedKeys.insert(GLFW_KEY_A);
  }
  if (key ==  GLFW_KEY_S && action == GLFW_PRESS) {
    m_pressedKeys.insert(GLFW_KEY_S);
  }
  if (key ==  GLFW_KEY_D && action == GLFW_PRESS) {
    m_pressedKeys.insert(GLFW_KEY_D);
  }
  if (key ==  GLFW_KEY_Z && action == GLFW_PRESS) {
    m_pressedKeys.insert(GLFW_KEY_Z);
  }
  if (key == GLFW_KEY_X && action == GLFW_PRESS) {
    m_pressedKeys.insert(GLFW_KEY_X);
  }

  if(key == GLFW_KEY_G && action== GLFW_PRESS) {
    
    m_showGui = !m_showGui;

    if (m_showGui) {
      // Don't hide cursor
      glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    else {
      // Hide and center cursor
      m_deltaX -= SCR_WIDTH / 2;
      m_deltaY -= SCR_HEIGHT / 2;
      glfwSetCursorPos(m_window, SCR_WIDTH / 2, SCR_HEIGHT / 2);
      glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    }

  }

  // Check for the release of keys to remove
  if (key == GLFW_KEY_W && action == GLFW_RELEASE) {
    m_pressedKeys.erase(GLFW_KEY_W);
  }
  if (key == GLFW_KEY_A && action == GLFW_RELEASE) {
    m_pressedKeys.erase(GLFW_KEY_A);
  }
  if (key == GLFW_KEY_S && action == GLFW_RELEASE) {
    m_pressedKeys.erase(GLFW_KEY_S);
  }
  if (key == GLFW_KEY_D && action == GLFW_RELEASE) {
    m_pressedKeys.erase(GLFW_KEY_D);
  }
  if (key == GLFW_KEY_Z && action == GLFW_RELEASE) {
    m_pressedKeys.erase(GLFW_KEY_Z);
  }
  if (key == GLFW_KEY_X && action == GLFW_RELEASE) {
    m_pressedKeys.erase(GLFW_KEY_X);
  }

}

void InputController::processMousePress(GLFWwindow* window, int button, int action, int mods) {

  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {

    m_pressedKeys.insert(GLFW_MOUSE_BUTTON_LEFT);

  }
  if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS) {
    m_pressedKeys.insert(GLFW_MOUSE_BUTTON_MIDDLE);
  }

  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {

    m_pressedKeys.erase(GLFW_MOUSE_BUTTON_LEFT);

  }
  if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_RELEASE) {
    m_pressedKeys.erase(GLFW_MOUSE_BUTTON_MIDDLE);
  }

}

void InputController::processMousePos(GLFWwindow* window, double xpos, double ypos) {

  // Get config data
  Config* config = Config::getInstance();
  unsigned int SCR_WIDTH = config->getScreenWidth();
  unsigned int SCR_HEIGHT = config->getScreenHeight();

  m_mouseX = xpos;
  m_mouseY = ypos;

  if(!m_showGui) {
    // Center cursor
    m_deltaX = m_mouseX - SCR_WIDTH / 2;
    m_deltaY = m_mouseY - SCR_HEIGHT / 2;
    glfwSetCursorPos(m_window, SCR_WIDTH / 2, SCR_HEIGHT / 2);
  }

}

std::unordered_set<unsigned int>* InputController::getPressedKeys()
{
  return &m_pressedKeys;
}

double InputController::getMouseX() {
  return m_mouseX;
}

double InputController::getMouseY() {
  return m_mouseY;
}

double InputController::getMouseDeltaX()
{
  return m_deltaX;
}

double InputController::getMouseDeltaY()
{
  return m_deltaY;
}

