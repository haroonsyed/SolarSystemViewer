#pragma once
#include <string>
#include <unordered_map>

// Singleton pattern (I think it's fine since this class takes over code in the main)
// I could decouple, but I really like the caching feature
// NOT THREAD SAFE
class ShaderManager {
private:
    static ShaderManager* instance;
    static unsigned int boundShader;
    static std::unordered_map<std::string, unsigned int> shaderMap;
    std::string readShader(std::string shaderFilePath);
    ShaderManager();

public:
    static ShaderManager* getInstance();
    unsigned int getBoundShader();
    void useShader(std::string vertexShaderPath, std::string fragShaderPath);
};