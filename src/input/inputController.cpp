#include "inputController.h"
#include <iostream>

InputController::InputController(GLFWwindow* window) {
    mouseX = 0;
    mouseY = 0;
    mousePressed = false;
    this->window = window;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void InputController::processInput()
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    
    double timeSinceLastInput = glfwGetTime() - timeAtLastInput;

    if (timeSinceLastInput > 1.0/60) { // 60 Hz polling rate

        // Reset time since last input
        timeAtLastInput = glfwGetTime();


        if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) {
            viewTransform = glm::scale(viewTransform, glm::vec3(1.02,1.02,1.02)); // ZOOM IN
        }
        else if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
            viewTransform = glm::scale(viewTransform, glm::vec3(0.98, 0.98, 0.98)); // ZOOM IN
        }


        //Get if mouse is pressed, and grab coordinates (will be relative to zero so easy).
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
            // On change first set mouse back to 0 so no model jump in view
            if (mousePressed == false) { 
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN); // Hide cursor
                glfwSetCursorPos(window, 0, 0);
                mousePressed = true;
            }

            // Implement a infinite mouse wrap (you can keep dragging and it keeps receiving input)
            double deltaX, deltaY = 0;
            glfwGetCursorPos(window, &deltaX, &deltaY);
            mouseX += deltaX;
            mouseY += deltaY;
            glfwSetCursorPos(window, 0, 0); // Reset position for next mouse input
        }
        //Show mouse cursor and stop locking its position for next press. 
        //Only on change to not pressed anymore.
        else if(mousePressed == true) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            mousePressed = false;
            mouseX = mouseY = 0;
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

