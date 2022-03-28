#include "inputController.h"
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "../config.h"

InputController::InputController(GLFWwindow* window) {
  mousePressed = false;
  this->window = window;
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
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);
  
  // Used to control sensitivity
  double timeSinceLastInput = glfwGetTime() - timeAtLastInput;
  if (timeSinceLastInput > 1.0/INPUT_POLL_RATE) { 

    // Reset time since last input
    timeAtLastInput = glfwGetTime();

    // SCALING VIEW
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) {
      viewTransform = glm::translate(viewTransform, glm::vec3(0,0,-0.05)); // ZOOM IN
    }
    else if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
      viewTransform = glm::translate(viewTransform, glm::vec3(0,0,0.05)); // ZOOM IN
    }

    // ROTATING VIEW
    //Get if mouse is pressed, and grab coordinates (will be relative to zero so easy).
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {

      // On change first set mouse to middle and hide cursor
      if (mousePressed == false) {
          glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
          glfwSetCursorPos(window, SCR_WIDTH/2, SCR_HEIGHT/2);
          mousePressed = true;
      }

      // Determine the amount and direction the mouse moved
      double deltaX, deltaY = 0;
      glfwGetCursorPos(window, &deltaX, &deltaY);
      glfwSetCursorPos(window, SCR_WIDTH / 2, SCR_HEIGHT / 2); // Reset position for next mouse input
      // Adjust for mouse being in middle
      deltaX -= SCR_WIDTH / 2;
      deltaY -= SCR_HEIGHT / 2;
      viewTransform = glm::rotate(viewTransform, (float)(deltaX)*glm::radians(-1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
      viewTransform = glm::rotate(viewTransform, (float)(deltaY)*glm::radians(1.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    }
    //Show mouse cursor and stop locking its position for next press. 
    //Only on change to not pressed anymore.
    else {
      glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
      mousePressed = false;
    }

  }

}

char InputController::getPressedKey()
{
    return pressedKey;
}

glm::mat4 InputController::getViewTransform() {
  return viewTransform;
}

