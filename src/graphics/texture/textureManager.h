#pragma once
#include <string>
#include <unordered_map>

// Singleton pattern (I think it's fine since this class takes over code in the main)
// I could decouple, but I really like the caching feature
// NOT THREAD SAFE
class TextureManager {
private:
  static TextureManager* instance;
  static unsigned int boundTexture;
  static std::unordered_map<std::string, unsigned int> textureMap; 
  TextureManager();

public:
  static TextureManager* getInstance();
  unsigned int getBoundTexture();
  void bindTexture(std::string textureFilePath);
};