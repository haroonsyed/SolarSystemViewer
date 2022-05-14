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
  m_screenQuad.setShaders("../assets/shaders/colorCorrection.vs", "../assets/shaders/colorCorrection.fs");

  generateBuffers();

  m_prevExposure = 1.0f;

}

void ScreenManager::generateBuffers() {
  generateFrameBuffers();
  genScreenSSBO();
}

void ScreenManager::genScreenSSBO() {

  // Data to send to vRam for the compute shader
  Config* config = Config::getInstance();
  const unsigned int width = config->getScreenWidth();
  const unsigned int height = config->getScreenHeight();
  const unsigned int percentRangeFromCenterForLuminanceCalc = config->getAutoExposureRange() * 100;
  std::vector<GLuint> data = { 0, width, height, percentRangeFromCenterForLuminanceCalc };

  unsigned int ssboSizeBytes = 4 * sizeof(GLuint); // Luminance, Width, Height, Range, Luminance

  glGenBuffers(1, &m_screenSSBO);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_screenSSBO);
  glBufferData(GL_SHADER_STORAGE_BUFFER, ssboSizeBytes, &data[0], GL_STATIC_DRAW);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_screenSSBO);

}

void ScreenManager::generateFrameBuffers() {

  // Create the hdr frame buffer
  glGenFramebuffers(1, &m_sceneFBO);
  glBindFramebuffer(GL_FRAMEBUFFER, m_sceneFBO);

  // Add color texture to fbo
  Config* config = Config::getInstance();
  unsigned int width = config->getScreenWidth();
  unsigned int height = config->getScreenHeight();

  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

  glGenTextures(1, &m_sceneHDRTexture);
  glBindTexture(GL_TEXTURE_2D, m_sceneHDRTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_sceneHDRTexture, 0);
  glBindTexture(GL_TEXTURE_2D, 0);

  // Create render buffer for depth and stencil data
  unsigned int rbo;
  glGenRenderbuffers(1, &rbo);
  glBindRenderbuffer(GL_RENDERBUFFER, rbo);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);
  
}

unsigned int get1DArrayPositionFrom2D(int x, int y, int rowWidth) {
  return y * rowWidth + x;
}

void ScreenManager::calculateExposure(float deltaT) {

  // Crude, switch to a compute shader with histogram for median brightness
  Config* config = Config::getInstance();
  auto width = config->getScreenWidth();
  auto height = config->getScreenHeight();

  double range = config->getAutoExposureRange();
  int rangeX = width * range;
  int rangeY = height * range;

  if (!config->getAutoExposureEnabled()) {
    m_prevExposure = 1.0f;
    return;
  }

  glBindImageTexture(2, m_sceneHDRTexture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);

  ShaderManager* shaderManager = ShaderManager::getInstance();
  shaderManager->bindComputeShader("../assets/shaders/compute/calculateLuminance.comp");
  unsigned int workerGroupSize = 32;
  glDispatchCompute(rangeX / workerGroupSize, rangeY / workerGroupSize, 1);
  glMemoryBarrier(GL_SHADER_STORAGE_BUFFER);

  float luminance = 0.0f;
  unsigned int luminanceRaw = 0;
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_screenSSBO);
  glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(unsigned int), &luminanceRaw);
  luminance = static_cast<float>(luminanceRaw) / (static_cast<float>(rangeX) * rangeY / (workerGroupSize * workerGroupSize));

  luminance *= std::log(1.0 + luminance);

  float exposureControl = config->getAutoExposureControl();
  float exposure = exposureControl / (luminance);

  // Move toward this luminance from previous luminance
  // Exposure should take 5 seconds to adjust
  float exposureSpeed = deltaT / 3.0f;
  float a = exposureSpeed * (1.0f / exposure) * std::fabsf(exposure - m_prevExposure);
  exposure = m_prevExposure * (1 - a) + exposure * a;
  exposure = glm::isnan(exposure) ? 1.0f : glm::clamp(exposure, 1e-8f, 1e8f);
  m_prevExposure = exposure;

  //std::cout << "Luminance Raw: " << luminanceRaw << std::endl;
  //std::cout << "Luminance: " << luminance << std::endl;
  //std::cout << "Exposure: " << exposure << std::endl;

}

void ScreenManager::bindDefaultBuffer() {
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ScreenManager::bindSceneBuffer() {
  glBindFramebuffer(GL_FRAMEBUFFER, m_sceneFBO);
}

void ScreenManager::clearScreenBuffer() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void ScreenManager::renderToScreen(float deltaT) {

  calculateExposure(deltaT);

  // Bind the quad to render scene texture on
  m_screenQuad.bind();
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, m_sceneHDRTexture);

  glEnable(GL_BLEND); // Blend with background (or skybox)
  glDisable(GL_DEPTH_TEST);

  // Render the frame on the quad with post processing
  ShaderManager* shaderManager = ShaderManager::getInstance();
  unsigned int shaderProgram = shaderManager->getBoundShader();
  unsigned int exposureLoc = glGetUniformLocation(shaderProgram, "exposure");
  glUniform1fv(exposureLoc, 1, &m_prevExposure);

  glDrawArrays(GL_TRIANGLES, 0, 6);

  glDisable(GL_BLEND);
  glEnable(GL_DEPTH_TEST);

}

