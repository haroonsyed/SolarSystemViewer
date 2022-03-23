#pragma once
#include <string>
#include <vector>
#include <glm/glm.hpp>

class Mesh {
private:
    std::string meshFilePath;
    std::string vertexShaderPath;
    std::string fragShaderPath;

public:
    glm::mat4 modelTransform;
    void setMesh(std::string meshFilePath);
    void setShaders(std::string vertexShaderPath, std::string fragShaderPath);
    void bind();
};