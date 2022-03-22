#include "shaderImporter.h"
#include <fstream>

std::string ShaderImporter::readShader(std::string meshFilePath) {
	std::ifstream file(meshFilePath);
	std::string shader;
	
	std::getline(file, shader, '\0');

	return shader;
}