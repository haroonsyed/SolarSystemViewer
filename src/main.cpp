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
#include "shaderManager.h"
#include "meshManager.h"
#include "./input/inputController.h"
#include "config.h"
#include "./physics/system.h"

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
    System system;
    GravBody p1;
    p1.setMass(6e24 / 1e6);
    p1.setPosition(glm::vec3(0, 0, 0));
    p1.setVelocity(glm::vec3(0, 0, 0));
    p1.getMesh()->setMesh("../assets/models/sphere.obj");
    p1.getMesh()->setShaders("../shaders/phong.vs", "../shaders/phong.fs");
    system.addBody(p1);
    GravBody p2;
    p2.setMass(5e3 / 1e6);
    p2.setPosition(glm::vec3(0, 36e6 / 1e6, 0));
    p2.setVelocity(glm::vec3(-3139 / 1e6, 0, 0));
    p2.getMesh()->setMesh("../assets/models/womanhead.obj");
    p2.getMesh()->setShaders("../shaders/phong.vs", "../shaders/phong.fs");
    system.addBody(p2);

    // render loop
    // -----------
    glEnable(GL_DEPTH_TEST);
    InputController inputController(window);
    ShaderManager* shaderManager = ShaderManager::getInstance();
    MeshManager* meshManager = MeshManager::getInstance();
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
        system.update();

        // Light info
        glm::vec3 lightPos = glm::vec3(-1.0f, 0.0f, 0.0f);
        lightPos *= 10000;

        // Loop through objects in system and render them
        for (GravBody& body : system.getBodies()) {

            // Setup transform matrix for this body
            glm::mat4 model = glm::mat4(1.0f);
            //model = glm::translate(model, (float)glfwGetTime() * glm::vec3(0.1));
            model = glm::translate(model, (body.getPosition()/50.0f));
            model = glm::scale(model, glm::vec3(0.05));

            body.getMesh()->bind();

            //Pass to gpu
            unsigned int shaderProgram = shaderManager->getBoundShader();
            unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
            glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
            unsigned int lightLoc = glGetUniformLocation(shaderProgram, "lightPos");
            glUniform3fv(lightLoc, 1, glm::value_ptr(lightPos));

            std::vector<unsigned int> bufferInfo = meshManager->getBufferInfo();
            const unsigned int VAO = bufferInfo[0];
            const unsigned int VBO = bufferInfo[1];
            const unsigned int numVertices = bufferInfo[2];
            glUseProgram(shaderProgram);
            glBindVertexArray(VAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
            glDrawArrays(GL_TRIANGLES, 0, numVertices);

        }

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
