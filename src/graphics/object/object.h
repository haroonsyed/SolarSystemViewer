#pragma once
#include <string>
#include <vector>
#include <utility>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

class Object {
  private:
    std::string m_meshFilePath;
    std::string m_vertexShaderPath;
    std::string m_fragShaderPath;
    std::string m_diffuseMapFilePath;
    std::string m_normalMapFilePath;
    std::string m_specularMapFilePath;
    std::string m_emissiveMapFilePath;


    std::string m_name;
    glm::vec3 m_position;
    glm::quat m_rotation;
    float m_scale;
    float m_axis;

  public:
    Object();
    void setName(std::string name);
    std::string getName();
    glm::vec3 getPosition();
    void setPosition(float x, float y, float z);
    void setPosition(glm::vec3 position);
    void setRotation(float angle, glm::vec3 axis);
    void rotate(glm::quat quat);
    glm::mat4 getRotationMat();
    float getScale();
    void setScale(float scale);
    float getAxis();
    void setAxis(float axis);
    void setMesh(std::string meshFilePath);
    std::string getMesh();
    std::pair<std::string, std::string> getShaders();
    void setShaders(std::string vertexShaderPath, std::string fragShaderPath);
    void setDiffuseMap(std::string diffuseMapFilePath);
    void setNormalMap(std::string normalMapFilePath);
    void setSpecularMap(std::string specularMapFilePath);
    void setEmissiveMap(std::string emissiveMapFilePath);
    std::vector<std::string> getTextures();
    void bind();
    glm::mat4 getModelMatrix();

};