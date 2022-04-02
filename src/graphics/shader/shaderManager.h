#pragma once
#include <string>
#include <unordered_map>

// Singleton pattern (I think it's fine since this class takes over code in the main)
// I could decouple, but I really like the caching feature
// NOT THREAD SAFE
class ShaderManager {
private:
    static ShaderManager* m_instance;
    static unsigned int m_boundShader;
    static std::unordered_map<std::string, unsigned int> m_shaderMap;
    std::string readShader(std::string shaderFilePath);
    ShaderManager();

public:
    static ShaderManager* getInstance();
    unsigned int getBoundShader();
    void bindShader(std::string vertexShaderPath, std::string fragShaderPath);
};