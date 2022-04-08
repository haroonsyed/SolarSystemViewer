#include "inputController.h"
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "../config.h"

InputController::InputController(GLFWwindow* window) {
  // glfwSetCursorPos(m_window, SCR_WIDTH/2, SCR_HEIGHT/2);
  m_window = window;
}
// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void InputController::processInput()
{

  // Get config data
  Config* config = Config::getInstance();
  unsigned int SCR_WIDTH = config->getScreenWidth();
  unsigned int SCR_HEIGHT = config->getScreenHeight();
  unsigned int INPUT_POLL_RATE = config->getInputPollRate();

  // CLOSE APPLICATION
  if (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(m_window, true);
  
  // Used to control sensitivity
  double timeSinceLastInput = glfwGetTime() - m_timeAtLastInput;
  if (timeSinceLastInput > 1.0/INPUT_POLL_RATE) { 

    // Reset time since last input
    m_timeAtLastInput = glfwGetTime();

    m_pressedKeys.clear();

    // Get the relevant keys that are pressed
    if (glfwGetKey(m_window, GLFW_KEY_W) == GLFW_PRESS) {
        m_pressedKeys.insert(GLFW_KEY_W);
    }
    if (glfwGetKey(m_window, GLFW_KEY_A) == GLFW_PRESS) {
        m_pressedKeys.insert(GLFW_KEY_A);
    }
    if (glfwGetKey(m_window, GLFW_KEY_S) == GLFW_PRESS) {
        m_pressedKeys.insert(GLFW_KEY_S);
    }
    if (glfwGetKey(m_window, GLFW_KEY_D) == GLFW_PRESS) {
        m_pressedKeys.insert(GLFW_KEY_D);
    }
    if (glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS) {
        m_pressedKeys.insert(GLFW_MOUSE_BUTTON_MIDDLE);
    }

    if(glfwGetKey(m_window, GLFW_KEY_G) == GLFW_PRESS) {
        guiButtonPressed = true;
    }

    if (glfwGetKey(m_window, GLFW_KEY_G) == GLFW_RELEASE && guiButtonPressed)
    {
        m_pressedKeys.insert(GLFW_KEY_G);
        guiButtonPressed = false;
    }

    // Get the amount the mouse has moved
    glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    double xPosIn = 0;
    double yPosIn = 0;

    glfwGetCursorPos(m_window, &xPosIn, &yPosIn);

    float xpos = static_cast<float>(xPosIn);
    float ypos = static_cast<float>(yPosIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    if (xpos == 1919)
    {
        xOffset = 1;
    }
    else if (xpos == 0)
    {
        xOffset = -1;
    }
    else if (ypos == -29)
    {
        yOffset = 1;
    }
    else if (ypos == 1050)
    {
        yOffset = -1;
    }
    else
    {
        xOffset = xpos - lastX;
        yOffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
        lastX = xpos;
        lastY = ypos;

        float sensitivity = 0.08f; // change this value to your liking
        xOffset *= sensitivity;
        yOffset *= sensitivity;
    }

    // glfwSetCursorPos(m_window, SCR_WIDTH/2, SCR_HEIGHT/2);

  }

}

void InputController::scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{


}

std::unordered_set<unsigned int>* InputController::getPressedKeys()
{
    return &m_pressedKeys;
}

float InputController::getXOffset()
{
    return xOffset;
}

float InputController::getYOffset()
{
    return yOffset;
}

