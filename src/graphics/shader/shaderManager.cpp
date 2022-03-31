#include "shaderManager.h"
#include <iostream>
#include <fstream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

ShaderManager* ShaderManager::instance = nullptr;
unsigned int ShaderManager::boundShader = 0;
std::unordered_map<std::string, unsigned int> ShaderManager::shaderMap;

ShaderManager::ShaderManager() {};

std::string ShaderManager::readShader(std::string shaderFilePath) {
	std::ifstream file(shaderFilePath);
	std::string shader;

	std::getline(file, shader, '\0');

	return shader;
}

ShaderManager* ShaderManager::getInstance() {
	if (instance == nullptr) {
		instance = new ShaderManager();
	}
	return instance;
}

unsigned int ShaderManager::getBoundShader() {
	return boundShader;
}

void ShaderManager::bindShader(std::string vertexShaderPath, std::string fragShaderPath) {

	// Shaders are cached so they aren't built every time
	std::string shaderKey = vertexShaderPath + fragShaderPath;

	if (shaderMap.count(shaderKey) == 0) {

		std::string rawVertexShader = readShader(vertexShaderPath);
		std::string rawFragShader = readShader(fragShaderPath);

		const char* vertexShaderSource = rawVertexShader.c_str();
		const char* fragmentShaderSource = rawFragShader.c_str();

		// build and compile our shader program
		// ------------------------------------

		// vertex shader
		unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
		glCompileShader(vertexShader);
		// check for shader compile errors
		int success;
		char infoLog[512];
		glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
		}

		// fragment shader
		unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
		glCompileShader(fragmentShader);
		// check for shader compile errors
		glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
		}

		// link shaders
		unsigned int shaderProgram = glCreateProgram();
		glAttachShader(shaderProgram, vertexShader);
		glAttachShader(shaderProgram, fragmentShader);
		glLinkProgram(shaderProgram);
		// check for linking errors
		glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
		if (!success) {
			glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
		}
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);

		// Add to map
		shaderMap[shaderKey] = shaderProgram;

	}

	// Bind the shaderProgram
	unsigned int shaderProgram = shaderMap.at(shaderKey);
	glUseProgram(shaderProgram);
	boundShader = shaderProgram;

}