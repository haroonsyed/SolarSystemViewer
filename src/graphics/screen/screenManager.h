#pragma once
#include "../object/object.h"

// Singleton pattern, I would avoid but the buffer size needs to chnage with window resize callback
class ScreenManager {
private:

  // Contains all frameBuffers and their textures
  static ScreenManager* m_instance;
  Object m_screenQuad;
  unsigned int m_sceneFBO;
  unsigned int m_sceneHDRTexture;
  unsigned int m_sceneBloomTexture;
  unsigned int m_screenSSBO;
  float m_prevExposure;
  ScreenManager();
  void calculateExposure(float deltaT);
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
