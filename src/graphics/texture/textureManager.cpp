#include "textureManager.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../../../lib/std_image.h"
#include <iostream>
#include <GL/glew.h>
#include "../shader/shaderManager.h"

TextureManager* TextureManager::m_instance = nullptr;
unsigned int TextureManager::m_boundTexture = 0;
std::unordered_map<std::string, unsigned int> TextureManager::m_textureMap;

TextureManager::TextureManager() {};

TextureManager* TextureManager::getInstance() {
	if (m_instance == nullptr) {
		m_instance = new TextureManager();
	}
	return m_instance;
}

unsigned int TextureManager::getBoundTexture() {
	return m_boundTexture;
}

std::string TextureManager::getMapFromUniformLocation(int location) {
	switch (location) {
		case 0:
			return "diffuseMap";
		case 1:
			return "normalMap";
		case 2:
			return "specularMap";
		case 3:
			return "emissiveMap";
		default:
			return ("custom"+std::to_string(location));
	}
}

void TextureManager::bindTextures(std::vector<std::string>& textureFilePaths) {

	unsigned int shaderID = ShaderManager::getInstance()->getBoundShader();

	for (int i = 0; i < textureFilePaths.size(); i++) {

		std::string path = textureFilePaths[i];
		unsigned int existsLoc = glGetUniformLocation(shaderID, (getMapFromUniformLocation(i) + "Exists").c_str());
		unsigned int exists = !path.empty();
		glUniform1i(existsLoc, exists);

		if (!exists) {
			// No texture, continue to next one
			continue; 
		}
	
		// Textures are cached so they aren't built every time
		if (m_textureMap.count(path) == 0) {

			// Load the file
			int width, height = -1;
			int nrChannels;
			stbi_set_flip_vertically_on_load(true);
			unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, STBI_rgb_alpha);

			if (height == -1) {
				std::cout << "Could not find load image: " << path << std::endl;
			}

			unsigned int texture;
			glGenTextures(1, &texture);
			glBindTexture(GL_TEXTURE_2D, texture);

			// set the texture wrapping/filtering options (on the currently bound texture object)
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			// Send the texture to gpu and generate mip maps
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);

			// Add to map
			m_textureMap[path] = texture;

			// Free memory from cpu
			stbi_image_free(data);

		}

		// Bind the texture
		unsigned int texture = m_textureMap.at(path);
		glUniform1i(
			glGetUniformLocation(shaderID, getMapFromUniformLocation(i).c_str()),
			(unsigned int)i
		);
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, texture);
	}


}

