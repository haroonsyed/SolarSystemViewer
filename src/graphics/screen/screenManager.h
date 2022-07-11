#pragma once
#include "../object/object.h"

// Singleton pattern, I would avoid but the buffer size needs to chnage with window resize callback
class ScreenManager {
private:

  // Contains all frameBuffers and their textures
  static ScreenManager* m_instance;
  Object m_screenQuad;
  unsigned int m_screenFBO;
  unsigned int m_screenHDRTexture;
  std::vector<unsigned int> m_screenBloomTextures = {0,0,0,0,0}; 
  unsigned int m_screenSSBO;
  float m_prevExposure;
  ScreenManager();
  float calculateLuminance();
  void calculateExposure(float deltaT, float luminance);
  void applyBloom();
  void genScreenSSBO();
  void generateFrameBuffers();

public:
  static ScreenManager* getInstance();
  void generateBuffers();
  void bindDefaultBuffer();
  void bindSceneBuffer();
  void clearScreenBuffer();
  void renderToScreen(float deltaT);
};
