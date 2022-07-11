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
  std::vector<GLuint> data = { 0, width, height, percentRangeFromCenterForLuminanceCalc, 0 };

  unsigned int ssboSizeBytes = 5 * sizeof(GLuint); // Luminance, Width, Height, Range, blurPassCount

  glGenBuffers(1, &m_screenSSBO);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_screenSSBO);
  glBufferData(GL_SHADER_STORAGE_BUFFER, ssboSizeBytes, &data[0], GL_STATIC_DRAW);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_screenSSBO);

}

void ScreenManager::generateFrameBuffers() {

  // Create the hdr frame buffer
  glGenFramebuffers(1, &m_screenFBO);
  glBindFramebuffer(GL_FRAMEBUFFER, m_screenFBO);

  // Add color texture to fbo
  Config* config = Config::getInstance();
  unsigned int width = config->getScreenWidth();
  unsigned int height = config->getScreenHeight();

  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

  glGenTextures(1, &m_screenHDRTexture);
  glBindTexture(GL_TEXTURE_2D, m_screenHDRTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_screenHDRTexture, 0);

  // Create render buffer for depth and stencil data
  unsigned int rbo;
  glGenRenderbuffers(1, &rbo);
  glBindRenderbuffer(GL_RENDERBUFFER, rbo);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);

  // Create the mipmap levels for bloom
  // Change to actually use mipmaps instead of different textures...
  glGenTextures(m_screenBloomTextures.size(), &m_screenBloomTextures[0]);
  for (unsigned int i = 0; i < m_screenBloomTextures.size(); i++) {
    glBindTexture(GL_TEXTURE_2D, m_screenBloomTextures[i]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width >> (i + 1), height >> (i + 1), 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  }
  glBindTexture(GL_TEXTURE_2D, 0);
  
}

unsigned int get1DArrayPositionFrom2D(int x, int y, int rowWidth) {
  return y * rowWidth + x;
}

float ScreenManager::calculateLuminance() {

  Config* config = Config::getInstance();
  auto width = config->getScreenWidth();
  auto height = config->getScreenHeight();

  if (!config->getAutoExposureEnabled() && !config->getBloomEnabled()) {
    // No need to calculate luminance
    return 1.0f;
  }

  double range = config->getAutoExposureRange();
  int rangeX = width * range;
  int rangeY = height * range;

  glBindImageTexture(2, m_screenHDRTexture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA16F);

  ShaderManager* shaderManager = ShaderManager::getInstance();
  shaderManager->bindComputeShader("../assets/shaders/compute/calculateLuminance.comp");
  float workGroupSize = 32;

  unsigned int workGroupSize_X = std::ceil(rangeX / workGroupSize);
  unsigned int workGroupSize_Y = std::ceil(rangeY / workGroupSize);
  glDispatchCompute(workGroupSize_X, workGroupSize_Y, 1);
  glMemoryBarrier(GL_SHADER_STORAGE_BUFFER);

  float luminance = 0.0f;
  unsigned int luminanceRaw = 0;

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_screenSSBO);
  glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(unsigned int), &luminanceRaw);

  luminance = static_cast<float>(luminanceRaw) / (static_cast<float>(rangeX) * rangeY / (workGroupSize * workGroupSize));
  luminance *= std::log(1.0 + luminance);

  // Write averaged luminance back into buffer
  glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(GLfloat), &luminance);

  return luminance;

}

void ScreenManager::calculateExposure(float deltaT, float luminance) {

  Config* config = Config::getInstance();

  if (!config->getAutoExposureEnabled()) {
    m_prevExposure = 1.0f;
    return;
  }

  float exposureControl = config->getAutoExposureControl();
  float exposure = exposureControl / (luminance);

  // Move toward this luminance from previous luminance
  float exposureSpeed = deltaT / 3.0f;
  float a = exposureSpeed * (1.0f / exposure) * std::fabs(exposure - m_prevExposure);
  exposure = m_prevExposure * (1 - a) + exposure * a;
  exposure = glm::isnan(exposure) ? 1.0f : glm::clamp(exposure, 1e-8f, 1e8f);
  m_prevExposure = exposure;

  std::cout << "Luminance: " << luminance << std::endl;
  std::cout << "Exposure: " << exposure << std::endl;

}

void ScreenManager::applyBloom() {

  ShaderManager* shaderManager = ShaderManager::getInstance();
  Config* config = Config::getInstance();

  if (!config->getBloomEnabled()) {
    return;
  }

  const unsigned int width = config->getScreenWidth();
  const unsigned int height = config->getScreenHeight();

  float workGroupSize = 32.0f;
  int numOfMipMaps = m_screenBloomTextures.size();
  unsigned int workGroupSize_X = std::ceil(width / workGroupSize);
  unsigned int workGroupSize_Y = std::ceil(height / workGroupSize);

  
  // Downsample hdr buffer into its mipmaps
  shaderManager->bindComputeShader("../assets/shaders/compute/bloom_downsample.comp");
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, m_screenHDRTexture);
  for (int mipMapLevel = 0; mipMapLevel < numOfMipMaps; mipMapLevel++) {
    if (mipMapLevel > 0) {
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, m_screenBloomTextures[mipMapLevel-1]);
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindImageTexture(3, m_screenBloomTextures[mipMapLevel], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);

    const unsigned int adjustedGroupSize_X = std::ceil( (width >> (mipMapLevel + 1)) / 32.0f);
    const unsigned int adjustedGroupSize_Y = std::ceil( (height >> (mipMapLevel + 1)) / 32.0f);

    if (adjustedGroupSize_X > 0 && adjustedGroupSize_Y > 0) {
      glDispatchCompute(adjustedGroupSize_X, adjustedGroupSize_Y, 1);
      glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
    }
  }

  // Now upsample and blur back up
  shaderManager->bindComputeShader("../assets/shaders/compute/bloom_blur_upsample.comp");
  for (int mipMapLevel = numOfMipMaps-1; mipMapLevel >= 0; mipMapLevel--) {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_screenBloomTextures[mipMapLevel]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    if (mipMapLevel > 0) {

    glBindImageTexture(3, m_screenBloomTextures[mipMapLevel-1], 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16F);
    }
    else {
      glBindImageTexture(3, m_screenHDRTexture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16F);
    }

    const unsigned int adjustedGroupSize_X = std::ceil((width >> (mipMapLevel)) / 32.0f);
    const unsigned int adjustedGroupSize_Y = std::ceil((height >> (mipMapLevel)) / 32.0f);

    if (adjustedGroupSize_X > 0 && adjustedGroupSize_Y > 0) {
      glDispatchCompute(adjustedGroupSize_X, adjustedGroupSize_Y, 1);
      glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
    }

  }

}

void ScreenManager::bindDefaultBuffer() {
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ScreenManager::bindSceneBuffer() {
  glBindFramebuffer(GL_FRAMEBUFFER, m_screenFBO);
}

void ScreenManager::clearScreenBuffer() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void ScreenManager::renderToScreen(float deltaT) {
  //float luminance = calculateLuminance();
  //calculateExposure(deltaT, luminance);
  applyBloom();

  // Bind the quad to render screen texture on
  m_screenQuad.bind();
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, m_screenHDRTexture);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, m_screenBloomTextures[0]);
  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, m_screenBloomTextures[1]);
  glActiveTexture(GL_TEXTURE3);
  glBindTexture(GL_TEXTURE_2D, m_screenBloomTextures[2]);
  glActiveTexture(GL_TEXTURE4);
  glBindTexture(GL_TEXTURE_2D, m_screenBloomTextures[3]);

  //glEnable(GL_BLEND); // Blend with background (or skybox)
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

