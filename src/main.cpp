#include <GL/glew.h>
#define GLFW_INCLUDE_NONE

#include <iostream>
#include <chrono>
#include <thread>

// My Imports
#include "config.h"
#include "./game/gameController.h"
#include "./graphics/screen/screenManager.h"

// Use dGPU on windows. (IDK how on linux)
#ifdef _WIN32
#include <windows.h>
extern "C" __declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
extern "C" __declspec(dllexport) DWORD AmdPowerXpressRequestHighPerformance = 0x00000001;
#endif

// Prototypes
GLFWwindow* createWindow();
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

#ifdef RUN_TESTS 
#include "tests/tests.h" //Include tests, uses pre-processor to avoid main conflict when in test mode
#else
int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    #ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif

    // Make window 
    GLFWwindow* window = createWindow();

    // // glew: load all OpenGL function pointers
    glewInit();

    // Load scene
    Scene scene(window);
    scene.loadScene("../assets/scenes/sol.json");

    // Load scene into gameController
    GameController* game = GameController::getInstance(window, &scene);


    // OGL options
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glCullFace(GL_BACK);
    glfwSwapInterval(0);

    double frameTime = 1e-9; // Initialize very small so object don't move on first frame
    double timeAtLastFpsLog = 0.0;
    while (!glfwWindowShouldClose(window))
    {

      double startTime = glfwGetTime();

      game->update((float)frameTime);
      game->render((float)frameTime);

      // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
      glfwSwapBuffers(window);
      glfwPollEvents();

      // Measure performance
      double endTime = glfwGetTime();
      frameTime = endTime - startTime;

    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

#endif

GLFWwindow* createWindow() {

  // Get the config
  Config* config = Config::getInstance();
  unsigned int SCR_WIDTH = config->getScreenWidth();
  unsigned int SCR_HEIGHT = config->getScreenHeight();
  unsigned int TARGET_FRAMERATE = config->getTargetFramerate();

  // glfw window creation
  // --------------------
  GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "viewGL", NULL, NULL);
  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

  if (!window) {
    std::cout << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
  }

  return window;
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
  ScreenManager* screenManager = ScreenManager::getInstance();
  screenManager->generateBuffers();
  glViewport(0, 0, width, height);
}
