#pragma once
#include <string>
#include <glm/glm.hpp>

class Object {
  private:
    std::string meshFilePath;
    std::string vertexShaderPath;
    std::string fragShaderPath;
    glm::vec3 position;
    float scale;

  public:
    Object();
    glm::vec3 getPosition();
    void setPosition(float x, float y, float z);
    void setPosition(glm::vec3 position);
    float getScale();
    void setScale(float scale);
    void setMesh(std::string meshFilePath);
    void setShaders(std::string vertexShaderPath, std::string fragShaderPath);
    void bind();

};