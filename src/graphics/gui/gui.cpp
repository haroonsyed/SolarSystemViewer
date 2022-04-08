#include "gui.h"
#include <GL/glew.h>
#include "iostream"
#include <GLFW/glfw3.h>
#define GLFW_INCLUDE_NONE


const char* vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"void main()\n"
"{\n"
"   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
"}\0";
const char* fragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
"}\n\0";


Gui::Gui()
{

    guiVertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(guiVertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(guiVertexShader);
    // fragment shader
    guiFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(guiFragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(guiFragmentShader);
    // link shaders
    guiShaderProgram = glCreateProgram();
    glAttachShader(guiShaderProgram, guiVertexShader);
    glAttachShader(guiShaderProgram, guiFragmentShader);
    glLinkProgram(guiShaderProgram);
    // check for linking errors
    glDeleteShader(guiVertexShader);
    glDeleteShader(guiFragmentShader);


    float vertices[] = {
         0.5f,  0.5f, 0.0f,  // top right
         0.5f, -0.5f, 0.0f,  // bottom right
        -0.5f, -0.5f, 0.0f,  // bottom left
        -0.5f,  0.5f, 0.0f   // top left 
    };

    unsigned int indices[] = {  // note that we start from 0!
        0, 1, 3,  // first Triangle
        1, 2, 3   // second Triangle
    };


    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);




}

void Gui::render(std::unordered_set<unsigned int>* pressedKeys)
{
    if (pressedKeys->count(GLFW_KEY_G))
    {
        if (toggleGUI)
            toggleGUI = false;
        else
            toggleGUI = true;
    }
    if (toggleGUI)
    {
        glUseProgram(guiShaderProgram);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }
}
