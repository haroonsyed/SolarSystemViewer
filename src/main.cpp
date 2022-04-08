// template based on material from learnopengl.com
#include <GL/glew.h>
#define GLFW_INCLUDE_NONE

#include <iostream>
#include <chrono>
#include <thread>
#include <map>

// My Imports   
#include "./camera/camera.h"
#include "./input/inputController.h"
#include "config.h"
#include "./scene/scene.h"
#include "./graphics/gui/gui.h"

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

    // Get the config
    Config* config = Config::getInstance();
    unsigned int SCR_WIDTH = config->getScreenWidth();
    unsigned int SCR_HEIGHT = config->getScreenHeight();
    unsigned int TARGET_FRAMERATE = config->getTargetFramerate();

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
    Scene scene;
    scene.loadScene("../assets/scenes/sol.json");

    // Load Gui
    Gui GUI;

    System* sol = scene.getPhysicsSystem();
    std::vector<GravBody*> objects = sol->getBodies();
    int index = 0;
    for (int i = 0; i < objects.size(); i++)
    {
        if (objects[i]->getName() == "Jupiter")
        {
            index = i;
        }
    }




    


    // render loop
    // -----------
    glEnable(GL_DEPTH_TEST);
    Camera camera;
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
        glm::vec3 position = objects[index]->getPosition();
        camera.update(inputController.getPressedKeys(), inputController.getXOffset(), inputController.getYOffset(), position);
        glm::mat4 view = camera.getViewTransform();

        // Simulation
        GUI.render(inputController.getPressedKeys());
        scene.getPhysicsSystem()->update();
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
  Config* config = Config::getInstance();
  config->setScreenWidth(width);
  config->setScreenHeight(height);
  glViewport(0, 0, width, height);
}
