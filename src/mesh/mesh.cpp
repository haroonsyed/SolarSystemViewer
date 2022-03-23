#include "mesh.h"
#include "../shaderManager.h"
#include "../meshManager.h"

void Mesh::setMesh(std::string meshFilePath) {
	this->meshFilePath = meshFilePath;
}

void Mesh::setShaders(std::string vertexShaderPath, std::string fragShaderPath) {
	this->vertexShaderPath = vertexShaderPath;
	this->fragShaderPath = fragShaderPath;
}

void Mesh::bind() {

	// First bind the shader
	// Implement a cache system to speed this up
	ShaderManager* shaderManager = ShaderManager::getInstance();
	shaderManager->useShader(vertexShaderPath, fragShaderPath);

	// Now bind geometry to buffer
	MeshManager* meshManager = MeshManager::getInstance();
	meshManager->bindMesh(meshFilePath);

}