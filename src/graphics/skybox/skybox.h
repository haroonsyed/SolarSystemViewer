#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string>
#include <vector>

class Skybox
{
private:
	unsigned int texture;

	std::string skyboxVertShaderPath = "../assets/shaders/skyboxShader.vs";
	std::string skyboxFragShaderPath = "../assets/shaders/skyboxShader.fs";

	std::string meshPath = "../assets/models/skybox.obj";

	std::vector<std::string> faces;

public:
	Skybox();
	unsigned int loadCubemap(std::vector<std::string> faces);
	void render(glm::mat4 viewMatrix, glm::mat4 projMatrix);
};