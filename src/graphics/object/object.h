#pragma once
#include <string>

class Object {
private:
	std::string meshFilePath;
	std::string vertexShaderPath;
	std::string fragShaderPath;

public:
	
	void setMesh(std::string meshFilePath);
	void setShaders(std::string vertexShaderPath, std::string fragShaderPath);
	void bind();

};