#include <GL/glew.h>
#include "screenManager.h"
#include "../../config.h"

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
  glDrawArrays(GL_TRIANGLES, 0, 6);

}

