// template based on material from learnopengl.com
#include <GL/glew.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <iostream>

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
    System system;
    GravBody p1;
    p1.setMass(6e24);
    p1.setPosition(glm::vec3(0, 0.5 * 8e9, 0));
    p1.setVelocity(glm::vec3(5e4, 0, 0));
    p1.getMesh()->setMesh("../assets/models/sphere.obj");
    p1.getMesh()->setShaders("../shaders/phong.vs", "../shaders/phong.fs");
    system.addBody(p1);
    GravBody p2;
    p2.setMass(6e24);
    p2.setPosition(glm::vec3(0, -0.5 * 8e9, 0));
    p2.setVelocity(glm::vec3(-5e4, 0, 0));
    p2.getMesh()->setMesh("../assets/models/sphere.obj");
    p2.getMesh()->setShaders("../shaders/phong.vs", "../shaders/phong.fs");
    system.addBody(p2);

    // render loop
    // -----------
    glEnable(GL_DEPTH_TEST);
    InputController inputController(window);
    ShaderManager* shaderManager = ShaderManager::getInstance();
    MeshManager* meshManager = MeshManager::getInstance();
    while (!glfwWindowShouldClose(window))
    {
        // Clear previous frame
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Input
        inputController.processInput();

        // Simulation
        //system.update();

        // Find max distance object in order to make a windowing function
        float maxDistance = 0.0f; // Used for windowing function
        for (GravBody& body : system.getBodies()) {
            maxDistance = std::fmaxf(maxDistance, glm::length(body.getPosition()));
        }

        glm::mat4 view = inputController.getViewTransform();

        // Light info
        glm::vec3 lightPos = glm::vec3(-1.0f, 0.2f, -1.0f);
        lightPos *= 1000;

        // Loop through objects in system and render them
        int count = 0;
        for (GravBody& body : system.getBodies()) {

            // Setup transform matrix for this body
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::scale(model, glm::vec3(0.2));
            model = glm::translate(model, glm::vec3(2 * count, 0, 0));
            //model = glm::translate(model, body.getPosition());
            //model = glm::scale(model, glm::vec3(1 / maxDistance));
            count++;

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
