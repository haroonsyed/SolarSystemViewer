#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <string>
#include <vector>
#include "../object/object.h"

class Skybox
{
private:

	Object m_skybox;

	unsigned int texture;

public:
	Skybox();
	void buildCubeMapTextureFromFilePaths(std::vector<std::string> faces);
	void render(glm::mat4 viewMatrix, glm::mat4 projMatrix);
};