#include "object.h"
#include "../mesh/meshManager.h"
#include "../shader/shaderManager.h"

void Object::setMesh(std::string meshFilePath) {
	this->meshFilePath = meshFilePath;
}

void Object::setShaders(std::string vertexShaderPath, std::string fragShaderPath) {
	this->vertexShaderPath = vertexShaderPath;
	this->fragShaderPath = fragShaderPath;
}

void Object::bind() {

	// First bind the shader
	// Implement a cache system to speed this up
	ShaderManager* shaderManager = ShaderManager::getInstance();
	shaderManager->bindShader(vertexShaderPath, fragShaderPath);

	// Now bind geometry to buffer
	MeshManager* meshManager = MeshManager::getInstance();
	meshManager->bindMesh(meshFilePath);


}