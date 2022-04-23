#include <GL/glew.h>
#include <iostream>
#include "screenManager.h"
#include "../shader/shaderManager.h"
#include "../../config.h"
#include <math.h>

ScreenManager* ScreenManager::m_instance = nullptr;

ScreenManager* ScreenManager::getInstance() {
  if (m_instance == nullptr) {
    m_instance = new ScreenManager();
  }
  return m_instance;
}

ScreenManager::ScreenManager() {
  // Setup screen quad to display on
  m_screenQuad.setMesh("../assets/models/plane.obj");
  m_screenQuad.setShaders("../assets/shaders/postProcessing.vs", "../assets/shaders/postProcessing.fs");

  generateFrameBuffers();

  m_prevExposure = 1.0f;

}

void ScreenManager::generateFrameBuffers() {
  // Create the hdr frame buffer
  glGenFramebuffers(1, &m_sceneFBO);
  glBindFramebuffer(GL_FRAMEBUFFER, m_sceneFBO);

  // Add color texture to fbo
  Config* config = Config::getInstance();
  unsigned int width = config->getScreenWidth();
  unsigned int height = config->getScreenHeight();

  glGenTextures(1, &m_sceneFBOTexture);
  glBindTexture(GL_TEXTURE_2D, m_sceneFBOTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_sceneFBOTexture, 0);
  glBindTexture(GL_TEXTURE_2D, 0);

  // Create render buffer for depth and stencil data
  unsigned int rbo;
  glGenRenderbuffers(1, &rbo);
  glBindRenderbuffer(GL_RENDERBUFFER, rbo);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

void ScreenManager::bindDefaultBuffer() {
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ScreenManager::bindSceneBuffer() {
  glBindFramebuffer(GL_FRAMEBUFFER, m_sceneFBO);
}

void ScreenManager::render() {

  glClear(GL_COLOR_BUFFER_BIT);
  glDisable(GL_DEPTH_TEST);

  m_screenQuad.bind();
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, m_sceneFBOTexture);

  // Used to find avg luminance
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glGenerateMipmap(GL_TEXTURE_2D); 

  // Crude, switch to a compute shader with histogram for median brightness
  GLfloat* pixels = new GLfloat[4];
  Config* config = Config::getInstance();
  auto width = config->getScreenWidth();
  auto height = config->getScreenHeight();

  unsigned int baseMMLevel = ceil(log2(std::max(width, height))) - 1;
  glGetTexImage(GL_TEXTURE_2D, baseMMLevel, GL_RGBA, GL_FLOAT, pixels);

  //Calculate Luminance
  float luminance = (pixels[0] + pixels[1] + pixels[2]);
  float exposure = 0.5f / luminance;
  exposure = std::max(exposure, 0.3f); // Limit so bright object look bright
  exposure = std::min(exposure, 1e5f); // Don't let exposure go to infinity
  
  // Interpolate between this luminance and previous luminance
  // Should probably be based on deltaT
  exposure = glm::mix(m_prevExposure, exposure, 0.01f);
  m_prevExposure = exposure;

  //std::cout << "Luminance: " << luminance << " Exposure: " << exposure << std::endl;
  ShaderManager* shaderManager = ShaderManager::getInstance();
  unsigned int shaderProgram = shaderManager->getBoundShader();
  unsigned int lightLoc = glGetUniformLocation(shaderProgram, "exposure");
  glUniform1fv(lightLoc, 1, &exposure);

  delete[] pixels;
  glDrawArrays(GL_TRIANGLES, 0, 6);

}

