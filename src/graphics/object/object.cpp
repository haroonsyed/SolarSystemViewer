#include "object.h"

#include <glm/gtx/quaternion.hpp>
#include "../../../lib/std_image.h"

#include <math.h>  

#include "../mesh/meshManager.h"
#include "../shader/shaderManager.h"
#include "../texture/textureManager.h"


#define PI 3.14159265

//image height and width

Object::Object() {
  m_position = glm::vec3(0.0);
  m_rotation = glm::angleAxis(0.0f, glm::vec3(0.0, 1.0, 0.0));
  m_scale = 70e3/1e3;
  m_meshFilePath = "../assets/models/sphere.obj";
  m_vertexShaderPath = "../shaders/phong.vs";
  m_fragShaderPath = "../shaders/phong.fs";

  Texture m_diffuse;
  Texture m_normal;
  Texture m_specular;
  Texture m_emissive;
  Texture m_cloud;
  Texture m_dynamic;

  value = 0;
}

void Object::setName(std::string name) {
  m_name = name;
}

std::string Object::getName() {
  return m_name;
}

glm::vec3 Object::getPosition() {
  return m_position;
}

void Object::setPosition(float x, float y, float z) {
  m_position = glm::vec3(x, y, z);
}

void Object::setPosition(glm::vec3 position) {
  m_position = position;
}

void Object::setRotation(float angle, glm::vec3 axis) {
  m_rotation = glm::angleAxis(angle, glm::normalize(axis));
}

void Object::rotate(glm::quat rotation) {
  m_rotation = rotation * m_rotation;
}

glm::mat4 Object::getRotationMat() {
  return glm::toMat4(m_rotation);
}

float Object::getScale() {
  return m_scale;
}

void Object::setScale(float scale) {
  m_scale = scale;
}

float Object::getAxis()
{
	return m_axis;
}

void Object::setAxis(float axis)
{
	m_axis = axis;
}

void Object::setMesh(std::string meshFilePath) {
  m_meshFilePath = meshFilePath;
}

void Object::setShaders(std::string vertexShaderPath, std::string fragShaderPath) {
  m_vertexShaderPath = vertexShaderPath;
  m_fragShaderPath = fragShaderPath;
}

void Object::setDiffuseMap(std::string diffuseMapFilePath) {
  m_diffuse = loadData(diffuseMapFilePath, 3);
}

void Object::setNormalMap(std::string normalMapFilePath) {
  m_normal = loadData(normalMapFilePath, 4);
}

void Object::setSpecularMap(std::string specularMapFilePath) {
  m_specular = loadData(specularMapFilePath, 4);
}

void Object::setEmissiveMap(std::string emissiveMapFilePath) {
  m_emissive = loadData(emissiveMapFilePath, 4);
}

void Object::setCloudMap(std::string cloudMapFilePath) {
  m_cloud = loadData(cloudMapFilePath, 3);
}

Texture Object::loadData(std::string path, int channels)
{
	int width, height = -1;
	int nrChannels;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, channels);
	if (height == -1) {
		std::cout << "Could not find load image: " << path << std::endl;
	}
	return Texture(path, height, width, channels, data);
}

//Texture Object::generateCloudTexture() {
//
//	unsigned char* newImage = new unsigned char[8192 * 4096 * 3]();
//
//	float cloudDensity = cos(value);
//
//	for (unsigned char* p = m_diffuse.data, *pg = newImage, *c = m_cloud.data; p != m_diffuse.data + 8192 * 4096 * 3; p += 3, pg += 3, c += 1)
//	{
//		if ((uint8_t)(*c) < 255)
//		{
//			*pg = (uint8_t)(*p);
//			*(pg + 1) = (uint8_t)(*(p + 1));
//			*(pg + 2) = (uint8_t)(*(p + 2));
//		}
//		else
//		{
//			*pg = (uint8_t)(*c);
//			*(pg + 1) = (uint8_t)(*c);
//			*(pg + 2) = (uint8_t)(*c);
//		}
//		//*(pg + 1) = (uint8_t)(*(p + 1));
//		//*(pg + 2) = (uint8_t)(*(p + 2));
//		/*if ((uint8_t)(*c) < 254)
//		{
//			*pg = (uint8_t)(*p);
//			*(pg+1) = (uint8_t)(*(p+1));
//			*(pg+2) = (uint8_t)(*(p+2));
//		}
//		else
//		{
//			*pg = (uint8_t)(*c);
//			*(pg+1) = (uint8_t)(*c);
//			*(pg+2) = (uint8_t)(*c);
//		}*/
//	}
//
//	if (value > 2 * PI)
//	{
//		value = 0;
//	}
//	else
//	{
//		value = value + 1 / 10000;
//	}
//
//	return Texture("test", 4096, 8192, 3, newImage);
//
//
//
//}

std::vector<Texture> Object::getStaticTextures() {
	return
	{
		m_diffuse,
		m_normal,
		m_specular,
		m_emissive,
		m_cloud
	};
}


void Object::bind() {

  // First bind the shader
  ShaderManager* shaderManager = ShaderManager::getInstance();
  shaderManager->bindShader(m_vertexShaderPath, m_fragShaderPath);
  
  // Now bind geometry to buffer
  MeshManager* meshManager = MeshManager::getInstance();
  meshManager->bindMesh(m_meshFilePath);



  // Now bind textures
  TextureManager* textureManager = TextureManager::getInstance();

  std::vector<Texture> staticTextures = getStaticTextures();
  std::vector<Texture> dynamicTextures;

  /*if (m_cloud.data != nullptr)
  {
	  std::cout << "Hello" << std::endl;
	  std::vector<Texture> dynamicTextures = getDynamicTextures();
  }*/

  textureManager->bindTextures(staticTextures, dynamicTextures);

}