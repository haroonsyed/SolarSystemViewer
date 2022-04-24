#include "skybox.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../../../lib/std_image.h"

#include <iostream>
#include <GL/glew.h>
#include <vector>
#include <string>

#include "../shader/shaderManager.h"
#include "../mesh/meshManager.h"


Skybox::Skybox()
{
    ShaderManager* shaderManager = ShaderManager::getInstance();
    shaderManager->bindShader(skyboxVertShaderPath, skyboxFragShaderPath);

    MeshManager * meshManager = MeshManager::getInstance();
    meshManager->bindMesh(meshPath);

    faces.push_back("C:\\Users\\erics\\source\\repos\\SolarSystemSimulator\\assets\\textures\\right.png");
    faces.push_back("C:\\Users\\erics\\source\\repos\\SolarSystemSimulator\\assets\\textures\\left.png");
    faces.push_back("C:\\Users\\erics\\source\\repos\\SolarSystemSimulator\\assets\\textures\\top.png");
    faces.push_back("C:\\Users\\erics\\source\\repos\\SolarSystemSimulator\\assets\\textures\\bottom.png");
    faces.push_back("C:\\Users\\erics\\source\\repos\\SolarSystemSimulator\\assets\\textures\\front.png");
    faces.push_back("C:\\Users\\erics\\source\\repos\\SolarSystemSimulator\\assets\\textures\\back.png");

    texture = loadCubemap(faces);
}

unsigned int Skybox::loadCubemap(std::vector<std::string> faces)
{
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
            );
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return texture;
}

void Skybox::render(glm::mat4 viewMatrix, glm::mat4 projMatrix)
{
    glDepthFunc(GL_LEQUAL);

    ShaderManager* shaderManager = ShaderManager::getInstance();
    shaderManager->bindShader(skyboxVertShaderPath, skyboxFragShaderPath);

    glm::mat4 view = glm::mat4(glm::mat3(viewMatrix)); // remove translation from the view matrix
    glUniformMatrix4fv(glGetUniformLocation(shaderManager->getBoundShader(), "view"), 1, GL_FALSE, value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shaderManager->getBoundShader(), "projection"), 1, GL_FALSE, value_ptr(projMatrix));

    MeshManager* meshManager = MeshManager::getInstance();
    std::vector<unsigned int> bufferInfo = meshManager->getBufferInfo();
    const unsigned int numVertices = bufferInfo[2];

    // skybox cube
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
    glDrawArrays(GL_TRIANGLES, 0, numVertices);
    glBindVertexArray(0);

    glDepthFunc(GL_LESS);
}
