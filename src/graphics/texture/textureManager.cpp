#include "textureManager.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../../../lib/std_image.h"
#include <iostream>
#include <GL/glew.h>

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

void TextureManager::bindTexture(std::string textureFilePath) {

	// Textures are cached so they aren't built every time
	if (m_textureMap.count(textureFilePath) == 0) {

		// Load the file
		int width, height, nrChannels;
		stbi_set_flip_vertically_on_load(true);
		unsigned char* data = stbi_load(textureFilePath.c_str(), &width, &height, &nrChannels, 0);

		unsigned int texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);

		// set the texture wrapping/filtering options (on the currently bound texture object)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// Send the texture to gpu and generate mip maps
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		// Add to map
		m_textureMap[textureFilePath] = texture;

		// Free memory from cpu
		stbi_image_free(data);

	}

	// Bind the texture
	unsigned int texture = m_textureMap.at(textureFilePath);
	glBindTexture(GL_TEXTURE_2D, texture);

}

