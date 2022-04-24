#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>



struct Texture
{
    Texture(std::string p, int h, int w, int c, unsigned char *d)
    {
        this->path = p;
        this->height = h;
        this->width = w;
        this->channels = c;
        this->data = d;
    }
    Texture()
    {
        this->path = "";
        this->height = 0;
        this->width = 0;
        this->channels = 0;
        this->data = nullptr;
    }
    std::string path;
    int height;
    int width;
    int channels;
    unsigned char* data;
};

class Object {
  private:

    std::string m_meshFilePath;
    std::string m_vertexShaderPath;
    std::string m_fragShaderPath;

    Texture m_diffuse;
    Texture m_normal;
    Texture m_specular;
    Texture m_emissive;
    Texture m_cloud;
    Texture m_dynamic;


    std::string m_name;
    glm::vec3 m_position;
    glm::quat m_rotation;
    float m_scale;
    float m_axis;

    double value;

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
    void setShaders(std::string vertexShaderPath, std::string fragShaderPath);
    void setDiffuseMap(std::string diffuseMapFilePath);
    void setNormalMap(std::string normalMapFilePath);
    void setSpecularMap(std::string specularMapFilePath);
    void setEmissiveMap(std::string emissiveMapFilePath);
    void setCloudMap(std::string cloudMapFilePath);
    Texture loadData(std::string path, int channels);
    std::vector<Texture> getStaticTextures();
    std::vector<Texture> getDynamicTextures();
    //Texture generateCloudTexture();
    void bind();

};