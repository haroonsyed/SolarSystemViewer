#pragma once

#include <unordered_set>


class Gui {
private:
	unsigned int VBO, VAO, EBO;
	unsigned int guiVertexShader;
	unsigned int guiFragmentShader;
	unsigned int guiShaderProgram;
	bool toggleGUI = true;

public:
	Gui();
	void render(std::unordered_set<unsigned int> * pressedKeys);

};