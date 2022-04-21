// template based on material from learnopengl.com
#include <GL/glew.h>
#define GLFW_INCLUDE_NONE

#include <iostream>
#include <chrono>
#include <thread>

// My Imports
#include "config.h"
#include "./game/gameController.h"
#include "./graphics/shader/shaderManager.h"
#include "./graphics/mesh/meshManager.h"

// Prototypes
GLFWwindow* createWindow();
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
    unsigned int TARGET_FRAMERATE = config->getTargetFramerate();


    // Make window 
    GLFWwindow* window = createWindow();
    if (!window) {
      std::cout << "Failed to create GLFW window" << std::endl;
      glfwTerminate();
      return -1;
    }

    // // glew: load all OpenGL function pointers
    glewInit();

    // Create quad vao (used as surface rendered image will be put on)
    float quadVertices[] = {
      // positions        // texCoords 
      -1.0f,  1.0f, 0.0f,  0.0f, 1.0f, 
      -1.0f, -1.0f, 0.0f,  0.0f, 0.0f, 
       1.0f, -1.0f, 0.0f,  1.0f, 0.0f, 
                                      
      -1.0f,  1.0f, 0.0f,  0.0f, 1.0f, 
       1.0f, -1.0f, 0.0f,  1.0f, 0.0f, 
       1.0f,  1.0f, 0.0f,  1.0f, 1.0f 
    };
    unsigned int numDataPoints = 5; // Each vertex has pos(3), tex(2)
    unsigned int numVertices = 6;

    // Setup buffers for data
    unsigned int VBO, quadVAO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &VBO);
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(quadVAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * numDataPoints * numVertices, quadVertices, GL_STATIC_DRAW);

    // Add attribute data
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, numDataPoints * sizeof(float), (void*)0); // Position Data
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, numDataPoints * sizeof(float), (void*)(3 * sizeof(float))); // Texture data

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);


    // Create the frame buffer
    unsigned int hdrFBO;
    glGenFramebuffers(1, &hdrFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);

    // Add color texture to fbo
    unsigned int width = config->getScreenWidth();
    unsigned int height = config->getScreenHeight();
    unsigned int fboTexture;
    glGenTextures(1, &fboTexture);
    glBindTexture(GL_TEXTURE_2D, fboTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboTexture, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Create render buffer for depth and stencil data
    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    // Load scene
    Scene scene(window);
    scene.loadScene("../assets/scenes/testing.json");

    // Load scene into gameController
    GameController* game = GameController::getInstance(window, &scene);

    // render loop
    // -----------
    unsigned long frameCounter = 0;
    double frameTime = 1e-9; // Initialize very small so object don't move on first frame
    double timeAtLastDebug = 0.0;
    while (!glfwWindowShouldClose(window))
    {
      // Clear previous frame
      glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
      glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      glEnable(GL_DEPTH_TEST);

      double startTime = glfwGetTime();

      game->update((float) frameTime);
      game->render();

      // Bind back the default fbo and perform image processing
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
      glClear(GL_COLOR_BUFFER_BIT);

      // Switch to post-processing shader
      
      ShaderManager* shaderManager = ShaderManager::getInstance();
      shaderManager->bindShader("../assets/shaders/postProcessing.vs", "../assets/shaders/postProcessing.fs");

      // Render to quad
      glBindVertexArray(quadVAO);
      glDisable(GL_DEPTH_TEST);
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, fboTexture);
      glDrawArrays(GL_TRIANGLES, 0, 6);


      // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
      // -------------------------------------------------------------------------------
      glfwSwapBuffers(window);
      glfwPollEvents();

      // Measure performance
      double endTime = glfwGetTime();
      frameTime = endTime - startTime;
      double targetFrameTime = 1.0 / TARGET_FRAMERATE;

      frameCounter++;

      // 5 percent error acceptance
      if (frameTime > targetFrameTime*1.05) {
        std::cout << "Framerate struggling to keep up!" << std::endl;
      }

      // Print FPS every n seconds
      if (endTime - timeAtLastDebug > 5) {
        timeAtLastDebug = endTime;
        std::cout << "\nOverall Average framerate: " << frameCounter / endTime << " fps.\n" << std::endl;
      }
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

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
  glViewport(0, 0, width, height);
}
