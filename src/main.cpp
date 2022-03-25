// template based on material from learnopengl.com
#include <GL/glew.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdlib.h>
#include <chrono>
#include <thread>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// My Imports
#include "./graphics/mesh/meshManager.h"
#include "./graphics/shader/shaderManager.h"
#include "./input/inputController.h"
#include "config.h"
#include "./scene/scene.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "viewGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // // glew: load all OpenGL function pointers
    glewInit();

    // Load scene
    // Input must be mega meters and giga grams. (SI/1e6)
    Scene scene;
    GravBody p1;
    p1.setMass(6e24 / 1e6);
    p1.setPosition(glm::vec3(0, 0, 0));
    p1.setVelocity(glm::vec3(0, 0, 0));
    p1.getObject()->setMesh("../assets/models/sphere.obj");
    p1.getObject()->setShaders("../shaders/phong.vs", "../shaders/phong.fs");
    scene.getSystem()->addBody(p1);
    GravBody p2;
    p2.setMass(5e3 / 1e6);
    p2.setPosition(glm::vec3(0, 36e6 / 1e6, 0));
    p2.setVelocity(glm::vec3(-3139 / 1e6, 0, 0));
    p2.getObject()->setMesh("../assets/models/womanhead.obj");
    p2.getObject()->setShaders("../shaders/phong.vs", "../shaders/phong.fs");
    scene.getSystem()->addBody(p2);

    // render loop
    // -----------
    glEnable(GL_DEPTH_TEST);
    InputController inputController(window);
    unsigned long frameCounter = 0;
    while (!glfwWindowShouldClose(window))
    {
        // Clear previous frame
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        double startTime = glfwGetTime();

        // Input
        inputController.processInput();
        glm::mat4 view = inputController.getViewTransform();
        if (inputController.mousePressed) {
            int mouseX = inputController.mouseX;
            int mouseY = inputController.mouseY;
            // The negative one invert makes moving the model feel more natural in x direction
            view = glm::rotate(view, (float)(mouseX)*glm::radians(-1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            view = glm::rotate(view, (float)(mouseY)*glm::radians(1.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        }

        // Simulation
        scene.getSystem()->update();
        scene.render(view);
        

        double endTime = glfwGetTime();
        int frameTime = (1000.0 * (endTime - startTime));
        int targetFrameTime = 1000.0 / TARGET_FRAMERATE;
        if (frameTime < targetFrameTime) {
            std::this_thread::sleep_for(std::chrono::milliseconds(targetFrameTime));
        }
        frameCounter++;

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}
