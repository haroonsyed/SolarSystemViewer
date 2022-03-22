#pragma once
#include <string>
#include <vector>
#include <glm/glm.hpp>

class Mesh {
  private:
    std::string meshFilePath;
    std::vector<float> mesh;

  public:
    glm::mat4 modelTransform;
    void setMesh(std::string meshFilePath);
    std::vector<float> getMesh();
    
};
