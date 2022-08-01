#pragma once
#include <string>
#include <vector>
#include <utility>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "nlohmann/json.hpp"

class Object {
  private:
    std::string m_meshFilePath;
    std::string m_vertexShaderPath;
    std::string m_fragShaderPath;
    std::string m_diffuseMapFilePath;
    float m_diffuseMapStrength;
    std::string m_normalMapFilePath;
    float m_normalMapStrength;
    std::string m_specularMapFilePath;
    float m_specularMapStrength;
    std::string m_emissiveMapFilePath;
    float m_emissiveMapStrength;


    std::string m_name;
    glm::vec3 m_position;
    glm::vec3 m_velocity;
    glm::quat m_rotation;
    float m_mass;
    float m_scale;
    float m_axis;

    bool m_isParticle;

  public:
    Object();
    void setParamsFromJSON(float SIUnitScaleFactor, nlohmann::json jsonData);
    void setName(std::string name);
    std::string getName();
    glm::vec3 getPosition();
    void setPosition(float x, float y, float z);
    void setPosition(glm::vec3 position);
    void setRotation(float angle, glm::vec3 axis);
    glm::vec3 getVelocity();
    void setVelocity(float x, float y, float z);
    void setVelocity(glm::vec3 velocity);
    void rotate(glm::quat quat);
    glm::mat4 getRotationMat();
    float getScale();
    void setScale(float scale);
    float getAxis();
    void setAxis(float axis);
    float getMass();
    void setMass(float mass);
    void setMesh(std::string meshFilePath);
    std::string getMesh();
    std::pair<std::string, std::string> getShaders();
    void setShaders(std::string vertexShaderPath, std::string fragShaderPath);
    void setDiffuseMap(std::string diffuseMapFilePath);
    void setNormalMap(std::string normalMapFilePath);
    void setSpecularMap(std::string specularMapFilePath);
    void setEmissiveMap(std::string emissiveMapFilePath);
    void setDiffuseMapStrength(int strength);
    void setNormalMapStrength(int strength);
    void setSpecularMapStrength(int strength);
    void setEmissiveMapStrength(int strength);
    std::vector<std::string> getTextures();
    std::vector<float> getTextureStrengths();

    void bind();
    glm::mat4 getModelMatrix();
    bool isParticle();
    void setIsParticle(bool isParticle);

};