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

  // CLOSE APPLICATION
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);
  
  // Can be used to reduce sensitivity
  double timeSinceLastInput = glfwGetTime() - timeAtLastInput;

  if (timeSinceLastInput > 1.0/INPUT_POLL_RATE) { 

    // Reset time since last input
    timeAtLastInput = glfwGetTime();

    // VIEW DIMENSIONS
    int height, width;
    glfwGetWindowSize(window, &width, &height);

    // SCALING VIEW
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) {
      viewTransform = glm::scale(viewTransform, glm::vec3(1.02,1.02,1.02)); // ZOOM IN
    }
    else if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
      viewTransform = glm::scale(viewTransform, glm::vec3(0.98, 0.98, 0.98)); // ZOOM IN
    }

    // ROTATING VIEW
    //Get if mouse is pressed, and grab coordinates (will be relative to zero so easy).
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {

      // On change first set mouse to middle and hide cursor
      if (mousePressed == false) {
          glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
          glfwSetCursorPos(window, width/2, height/2);
          mousePressed = true;
      }

      // Determine the amount and direction the mouse moved
      double deltaX, deltaY = 0;
      glfwGetCursorPos(window, &deltaX, &deltaY);
      glfwSetCursorPos(window, width / 2, height / 2); // Reset position for next mouse input
      // Adjust for mouse being in middle
      deltaX -= width / 2;
      deltaY -= height / 2;
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

