#include <GL/glew.h>
#include "skybox.h"

#include "../../../lib/std_image.h"

#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>
#include <string>

#include "../shader/shaderManager.h"
#include "../mesh/meshManager.h"


Skybox::Skybox()
{

  std::string skyboxVertShaderPath = "../assets/shaders/skyboxShader.vs";
  std::string skyboxFragShaderPath = "../assets/shaders/skyboxShader.fs";

  std::string meshPath = "../assets/models/skybox.obj";

  m_skybox.setShaders(skyboxVertShaderPath, skyboxFragShaderPath);
  m_skybox.setMesh(meshPath);

  // We handle textures separately since OGL has a CUBE_MAP built in
  std::vector<std::string> faceTextures;

  faceTextures.push_back("../assets/textures/skybox/right.jpg");
  faceTextures.push_back("../assets/textures/skybox/left.jpg");
  faceTextures.push_back("../assets/textures/skybox/top.jpg");
  faceTextures.push_back("../assets/textures/skybox/bottom.jpg");
  faceTextures.push_back("../assets/textures/skybox/front.jpg");
  faceTextures.push_back("../assets/textures/skybox/back.jpg");

  buildCubeMapTextureFromFilePaths(faceTextures);

}

void Skybox::buildCubeMapTextureFromFilePaths(std::vector<std::string> faces)
{
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

}

void Skybox::render(glm::mat4 viewMatrix, glm::mat4 projMatrix)
{
  glDisable(GL_DEPTH_TEST);

  m_skybox.bind();
  ShaderManager* shaderManager = ShaderManager::getInstance();
  MeshManager* meshManager = MeshManager::getInstance();

  glm::mat4 view = glm::mat4(glm::mat3(viewMatrix)); // remove translation from the view matrix
  glUniformMatrix4fv(glGetUniformLocation(shaderManager->getBoundShader(), "view"), 1, GL_FALSE, value_ptr(view));
  glUniformMatrix4fv(glGetUniformLocation(shaderManager->getBoundShader(), "projection"), 1, GL_FALSE, value_ptr(projMatrix));

  std::vector<unsigned int> bufferInfo = meshManager->getBufferInfo();
  const unsigned int numVertices = bufferInfo[2];

  // Bind skybox texture
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, texture);

  // Render skybox
  glDrawArrays(GL_TRIANGLES, 0, numVertices);

  glEnable(GL_DEPTH_TEST);

}
