#pragma once

#include "../../../lib/std_image.h"

#include <iostream>
#include <GL/glew.h>
#include <string>

#include "../shader/shaderManager.h"
#include "../object/object.h"

// Singleton pattern (I think it's fine since this class takes over code in the main)
// I could decouple, but I really like the caching feature
// NOT THREAD SAFE
class TextureManager {
private:
  static TextureManager* m_instance;
  static unsigned int m_boundTexture;
  static std::unordered_map<std::string, unsigned int> m_textureMap; 
  TextureManager();

public:
  static TextureManager* getInstance();
  unsigned int getBoundTexture();
  std::string getMapFromUniformLocation(int location);
  void bindTextures(std::vector<Texture>& staticTextures, std::vector<Texture> dynamicTextures);
};